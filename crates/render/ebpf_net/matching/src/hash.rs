/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

// Perfect hash for RPC IDs for ebpf_net::matching
//
// g_type: u8
// g_size: 8
// g_shift: 29
// hash_shift: 24
// hash_mask: 31
// n_keys: 22
// multiplier: 2654435761
// hash_seed: 0

#[allow(dead_code)]
pub const MATCHING_HASH_SIZE: u32 = 32u32;

#[allow(dead_code)]
pub static G_ARRAY: [u8; 8] = [1, 2, 0, 0, 2, 0, 0, 1];

#[inline]
#[allow(dead_code)]
pub fn matching_hash(rpc_id: u32) -> u32 {
    let k = (rpc_id ^ 0u32).wrapping_mul(2654435761u32);
    let g = G_ARRAY[(k >> 29) as usize] as u32;
    (k >> 24).wrapping_add(g) & 31u32
}
