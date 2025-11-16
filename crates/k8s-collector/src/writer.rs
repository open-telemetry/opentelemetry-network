//! Async TCP writer to the reducer with LZ4 streaming compression.
//!
//! Semantics:
//! - On connect, the first message is sent uncompressed to negotiate
//!   decompression on the reducer side.
//! - Subsequent messages are appended to a single persistent LZ4 frame.
//! - `flush()` calls the encoder's `flush()` and drains any newly produced bytes
//!   to the socket, then flushes the socket.
//!
//! The writer only writes the newly produced compressed tail per call; the
//! underlying frame buffer is retained between calls and only reset on
//! reconnect.

use std::io;
use tokio::io::{AsyncWrite, AsyncWriteExt};
use tokio_util::io::SyncIoBridge;

/// Internal sink state: uncompressed passthrough initially, then a persistent
/// LZ4 frame once the first message has been sent.
enum Sink<W: AsyncWrite + Unpin + Send + 'static> {
    Uncompressed(W),
    Compressed {
        encoder: Box<lz4_flex::frame::FrameEncoder<SyncIoBridge<W>>>,
    },
}

pub struct Writer<W: AsyncWrite + Unpin + Send + 'static> {
    // Invariant: always Some; None is a temporary state during transitions
    sink: Option<Sink<W>>,
}

impl<W: AsyncWrite + Unpin + Send + 'static> Writer<W> {
    /// Create a new writer. The first message is sent uncompressed; subsequent
    /// messages are appended to a persistent LZ4 frame.
    pub fn new(sink: W) -> Self {
        Self {
            sink: Some(Sink::Uncompressed(sink)),
        }
    }

    /// Send a single message, applying the protocol described above.
    pub async fn send(&mut self, buf: &[u8]) -> io::Result<()> {
        // Take ownership of the sink enum to allow moving `W` between variants
        let sink = self.sink.take().expect("unreachable: sink must be set");
        match sink {
            Sink::Uncompressed(mut w) => {
                // First message: send uncompressed, then switch to compressed mode
                w.write_all(buf).await?;
                // Switch to compressed mode where the encoder writes directly to the inner sink
                let bridge = SyncIoBridge::new(w);
                let encoder = lz4_flex::frame::FrameEncoder::new(bridge);
                self.sink = Some(Sink::Compressed {
                    encoder: Box::new(encoder),
                });
                Ok(())
            }
            Sink::Compressed { mut encoder } => {
                use std::io::Write;
                // Perform synchronous compression in a blocking task; the bridge
                // will block on the async writer without panicking.
                let data = buf.to_vec();
                let res = tokio::task::spawn_blocking(move || {
                    encoder.write_all(&data)?;
                    Ok::<_, io::Error>(encoder)
                })
                .await
                .map_err(|e| io::Error::other(format!("join error: {}", e)))??;
                self.sink = Some(Sink::Compressed { encoder: res });
                Ok(())
            }
        }
    }

    /// Flush any pending compressed bytes and the underlying socket.
    pub async fn flush(&mut self) -> io::Result<()> {
        // We need ownership of the encoder to run flush inside spawn_blocking,
        // so we temporarily take the enum and put it back after flushing.
        let sink = self.sink.take().expect("unreachable: sink must be set");
        match sink {
            Sink::Uncompressed(mut w) => {
                w.flush().await?;
                self.sink = Some(Sink::Uncompressed(w));
                Ok(())
            }
            Sink::Compressed { encoder } => {
                // Flush the encoder which flushes the underlying sink via the bridge
                use std::io::Write;
                let res = tokio::task::spawn_blocking(move || {
                    let mut enc = encoder;
                    enc.flush()?;
                    Ok::<_, io::Error>(enc)
                })
                .await
                .map_err(|e| io::Error::other(format!("join error: {}", e)))??;
                self.sink = Some(Sink::Compressed { encoder: res });
                Ok(())
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use lz4_flex::frame::FrameDecoder;
    use std::io::Read;
    use std::pin::Pin;
    use std::sync::{Arc, Mutex};
    use tokio::io::AsyncWrite;

    /// Simple in-memory AsyncWrite sink used for tests.
    /// Bytes written are appended to the shared buffer.
    struct TestSink(pub Arc<Mutex<Vec<u8>>>);

    impl AsyncWrite for TestSink {
        fn poll_write(
            self: Pin<&mut Self>,
            _cx: &mut std::task::Context<'_>,
            buf: &[u8],
        ) -> std::task::Poll<std::io::Result<usize>> {
            let mut g = self.0.lock().unwrap();
            g.extend_from_slice(buf);
            std::task::Poll::Ready(Ok(buf.len()))
        }

        fn poll_flush(
            self: Pin<&mut Self>,
            _cx: &mut std::task::Context<'_>,
        ) -> std::task::Poll<std::io::Result<()>> {
            std::task::Poll::Ready(Ok(()))
        }

        fn poll_shutdown(
            self: Pin<&mut Self>,
            _cx: &mut std::task::Context<'_>,
        ) -> std::task::Poll<std::io::Result<()>> {
            std::task::Poll::Ready(Ok(()))
        }
    }

    #[test]
    // Property-based test: with compression enabled, the first flush exposes the
    // uncompressed handshake bytes; subsequent flushes append to the persistent
    // LZ4 frame. Decoding the combined compressed bytes after both flushes must
    // equal the concatenation of the payloads.
    fn flush_visibility_compressed_prop() {
        // Property-based: first flush exposes the uncompressed handshake (a),
        // subsequent flushes append to the persistent LZ4 frame (b then b+c).
        proptest::proptest!(|(a in "[ -~]{0,64}", b in "[ -~]{0,64}", c in "[ -~]{0,64}")| {
            let buf = Arc::new(Mutex::new(Vec::new()));
            let sink = TestSink(buf.clone());
            let rt = tokio::runtime::Runtime::new().unwrap();
            let a2 = a.clone(); let b2 = b.clone(); let c2 = c.clone();
            rt.block_on(async move {
                let mut w = super::Writer::new(sink);
                // First write is uncompressed handshake
                w.send(a2.as_bytes()).await.unwrap();
                w.flush().await.unwrap();
                // Verify handshake visible
                assert_eq!(&buf.lock().unwrap()[..], a2.as_bytes());

                // Second write goes into persistent frame
                w.send(b2.as_bytes()).await.unwrap();
                w.flush().await.unwrap();
                let snap = buf.lock().unwrap().clone();
                let (first, rest) = snap.split_at(a2.len());
                assert_eq!(first, a2.as_bytes());
                let mut dec = FrameDecoder::new(rest);
                let mut out = Vec::new();
                dec.read_to_end(&mut out).unwrap();
                assert_eq!(out, b2.as_bytes());

                // Third write appends to same frame
                w.send(c2.as_bytes()).await.unwrap();
                w.flush().await.unwrap();
                let snap2 = buf.lock().unwrap().clone();
                let (_first2, rest2) = snap2.split_at(a2.len());
                let mut dec2 = FrameDecoder::new(rest2);
                let mut out2 = Vec::new();
                dec2.read_to_end(&mut out2).unwrap();
                let mut expected = Vec::new();
                expected.extend_from_slice(b2.as_bytes());
                expected.extend_from_slice(c2.as_bytes());
                assert_eq!(out2, expected);
            });
        });
    }
}
