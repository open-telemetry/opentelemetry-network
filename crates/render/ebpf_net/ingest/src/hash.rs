/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

// Perfect hash for RPC IDs for ebpf_net::ingest
//
// g_type: u8
// g_size: 32
// g_shift: 27
// hash_shift: 20
// hash_mask: 127
// n_keys: 91
// multiplier: 2654435761
// hash_seed: 0

#[allow(dead_code)]
pub const INGEST_HASH_SIZE: u32 = 128u32;

#[allow(dead_code)]
pub static G_ARRAY: [u8; 32] = [
    0, 10, 9, 0, 63, 1, 11, 0, 0, 1, 4, 31, 4, 21, 14, 0, 5, 4, 4, 1, 0, 14, 3, 1, 4, 118, 0, 0,
    17, 1, 1, 0,
];

#[inline]
#[allow(dead_code)]
pub fn ingest_hash(rpc_id: u32) -> u32 {
    let k = (rpc_id ^ 0u32).wrapping_mul(2654435761u32);
    let g = G_ARRAY[(k >> 27) as usize] as u32;
    (k >> 20).wrapping_add(g) & 127u32
}
