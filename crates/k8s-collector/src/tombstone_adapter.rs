use std::collections::VecDeque;
use std::time::{Duration, Instant};

use kube::runtime::watcher::Event;

/// Stream-level adapter that retains Delete events temporarily.
///
/// - For Apply/InitApply/Init events: emit expired deletes first, then forward the event unchanged.
/// - For Delete events: retain internally; if capacity exceeded or entries have expired, emit those deletes.
/// - For InitDone: clear the retained deletes (drop them) and forward InitDone.
pub struct TombstoneAdapter<T> {
    ttl: Duration,
    cap: usize,
    q: VecDeque<(Instant, T)>,
}

impl<T: Clone> TombstoneAdapter<T> {
    /// Create a new adapter with the given time-to-live and capacity.
    pub fn new(ttl: Duration, cap: usize) -> Self {
        Self {
            ttl,
            cap,
            q: VecDeque::new(),
        }
    }

    /// Handle a new event and return an iterator over forwarded events:
    /// - Delete: retained; may emit evicted/expired deletes
    /// - Apply/Init/InitApply: emit expired deletes first, then the event
    /// - InitDone: clear retention and forward InitDone
    pub fn handle(&mut self, ev: Event<T>) -> impl Iterator<Item = Event<T>> {
        let mut out: Vec<Event<T>> = Vec::new();
        let now = Instant::now();

        let mut drain_expired = |out: &mut Vec<Event<T>>| {
            // Only inspect the timestamp by reference; clone `T` only when expired
            loop {
                match self.q.front() {
                    Some((ts, _)) if now.duration_since(*ts) >= self.ttl => {
                        if let Some((_, t)) = self.q.pop_front() {
                            out.push(Event::Delete(t));
                        }
                    }
                    _ => break,
                }
            }
        };

        match ev {
            Event::Init => {
                // Forward init after flushing expired deletes
                drain_expired(&mut out);
                out.push(Event::Init);
            }
            Event::InitApply(t) => {
                drain_expired(&mut out);
                out.push(Event::InitApply(t));
            }
            Event::Apply(t) => {
                drain_expired(&mut out);
                out.push(Event::Apply(t));
            }
            Event::Delete(t) => {
                // retain; drop expired first
                drain_expired(&mut out);
                self.q.push_back((now, t));
                while self.q.len() > self.cap {
                    if let Some((_, t)) = self.q.pop_front() {
                        out.push(Event::Delete(t));
                    }
                }
            }
            Event::InitDone => {
                // Drop retained deletes on init-done; forward the signal
                self.q.clear();
                out.push(Event::InitDone);
            }
        }

        out.into_iter()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    // Capacity eviction: when capacity is 1, inserting a second Delete
    // should evict (and thus emit) the oldest retained Delete event.
    fn capacity_evicts_oldest_delete() {
        let mut t = TombstoneAdapter::new(Duration::from_secs(60), 1);
        // First delete retained, nothing emitted
        assert!(t.handle(Event::Delete(1)).next().is_none());
        // Second delete should evict the first
        let mut evs = t.handle(Event::Delete(2));
        match evs.next() {
            Some(Event::Delete(x)) => assert_eq!(x, 1),
            _ => panic!("expected Delete(1)"),
        }
        assert!(evs.next().is_none());
    }

    #[test]
    // InitDone clears the retention queue, so no previously retained
    // deletes should be surfaced after it. The InitDone marker itself
    // is forwarded.
    fn initdone_clears_queue() {
        let mut t = TombstoneAdapter::new(Duration::from_secs(60), 2);
        // Retain two deletes
        assert!(t.handle(Event::Delete(10)).next().is_none());
        assert!(t.handle(Event::Delete(20)).next().is_none());
        // InitDone should drop queue and forward InitDone
        let mut evs = t.handle(Event::InitDone);
        matches!(evs.next(), Some(Event::InitDone));
        assert!(evs.next().is_none());
        // Another delete should not surface the old deletes anymore, they are dropped
        let mut evs2 = t.handle(Event::Delete(30));
        if evs2.next().is_some() {
            panic!("expected event - queue should be empty");
        }
        assert!(evs2.next().is_none());
    }

    #[test]
    // TTL expiration: with ttl=0, a retained Delete is expired immediately
    // on the next handle() call and emitted before forwarding the new event.
    fn ttl_expires_before_apply() {
        // ttl=0 forces immediate expiration on next handle
        let mut t = TombstoneAdapter::new(Duration::from_secs(0), 10);
        assert!(t.handle(Event::Delete(42)).next().is_none());
        // Next non-delete should surface the expired delete first
        let mut it = t.handle(Event::Apply(7));
        match it.next() {
            Some(Event::Delete(x)) => assert_eq!(x, 42),
            _ => panic!("expected Delete(42)"),
        }
        match it.next() {
            Some(Event::Apply(x)) => assert_eq!(x, 7),
            _ => panic!("expected Apply(7)"),
        }
        assert!(it.next().is_none());
    }
}
