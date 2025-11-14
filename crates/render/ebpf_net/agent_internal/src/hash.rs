/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

// Perfect hash for RPC IDs for ebpf_net::agent_internal
//
// g_type: u8
// g_size: 8
// g_shift: 29
// hash_shift: 23
// hash_mask: 63
// n_keys: 29
// multiplier: 2654435761
// hash_seed: 0

#[allow(dead_code)]
pub const AGENT_INTERNAL_HASH_SIZE: u32 = 64u32;

#[allow(dead_code)]
pub static G_ARRAY: [u8; 8] = [1, 0, 0, 1, 0, 0, 1, 1];

#[inline]
#[allow(dead_code)]
pub fn agent_internal_hash(rpc_id: u32) -> u32 {
    let k = (rpc_id ^ 0u32).wrapping_mul(2654435761u32);
    let g = G_ARRAY[(k >> 29) as usize] as u32;
    (k >> 23).wrapping_add(g) & 63u32
}
