/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

// Perfect hash for RPC IDs for ebpf_net::cloud_collector
//
// g_type: u8
// g_size: 2
// g_shift: 31
// hash_shift: 29
// hash_mask: 3
// n_keys: 1
// multiplier: 2654435761
// hash_seed: 0

#[allow(dead_code)]
pub const CLOUD_COLLECTOR_HASH_SIZE: u32 = 4u32;

#[allow(dead_code)]
pub static G_ARRAY: [u8; 2] = [0, 2];

#[inline]
#[allow(dead_code)]
pub fn cloud_collector_hash(rpc_id: u32) -> u32 {
    let k = (rpc_id ^ 0u32).wrapping_mul(2654435761u32);
    let g = G_ARRAY[(k >> 31) as usize] as u32;
    (k >> 29).wrapping_add(g) & 3u32
}
