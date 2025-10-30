/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

// Perfect hash for RPC IDs for ebpf_net::aggregation
//
// g_type: u8
// g_size: 4
// g_shift: 30
// hash_shift: 26
// hash_mask: 15
// n_keys: 8
// multiplier: 2654435761
// hash_seed: 0

#[allow(dead_code)]
pub const AGGREGATION_HASH_SIZE: u32 = 16u32;

#[allow(dead_code)]
pub static G_ARRAY: [u8; 4] = [0, 9, 0, 0];

#[inline]
#[allow(dead_code)]
pub fn aggregation_hash(rpc_id: u32) -> u32 {
    let k = (rpc_id ^ 0u32).wrapping_mul(2654435761u32);
    let g = G_ARRAY[(k >> 30) as usize] as u32;
    (k >> 26).wrapping_add(g) & 15u32
}
