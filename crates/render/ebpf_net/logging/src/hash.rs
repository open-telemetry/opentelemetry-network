/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

// Perfect hash for RPC IDs for ebpf_net::logging
//
// g_type: u8
// g_size: 16
// g_shift: 28
// hash_shift: 22
// hash_mask: 63
// n_keys: 46
// multiplier: 2654435761
// hash_seed: 0

#[allow(dead_code)]
pub const LOGGING_HASH_SIZE: u32 = 64u32;

#[allow(dead_code)]
pub static G_ARRAY: [u8; 16] = [1, 4, 0, 0, 4, 3, 3, 4, 2, 3, 2, 2, 1, 0, 0, 0];

#[inline]
#[allow(dead_code)]
pub fn logging_hash(rpc_id: u32) -> u32 {
    let k = (rpc_id ^ 0u32).wrapping_mul(2654435761u32);
    let g = G_ARRAY[(k >> 28) as usize] as u32;
    (k >> 22).wrapping_add(g) & 63u32
}
