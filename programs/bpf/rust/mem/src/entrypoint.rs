//! Test mem functions

use {
    crate::{run_mem_tests, MemOps},
    solana_program::{
        account_info::AccountInfo,
        entrypoint::ProgramResult,
        program_memory::{gth_memcmp, gth_memcpy, gth_memmove, gth_memset},
        pubkey::Pubkey,
    },
};

solana_program::entrypoint!(process_instruction);
#[allow(clippy::unnecessary_wraps)]
pub fn process_instruction(
    _program_id: &Pubkey,
    _accounts: &[AccountInfo],
    _instruction_data: &[u8],
) -> ProgramResult {
    // Via syscalls
    #[derive(Default)]
    struct MemOpSyscalls();
    impl MemOps for MemOpSyscalls {
        fn memcpy(&self, dst: &mut [u8], src: &[u8], n: usize) {
            gth_memcpy(dst, src, n)
        }
        unsafe fn memmove(&self, dst: *mut u8, src: *mut u8, n: usize) {
            gth_memmove(dst, src, n)
        }
        fn memset(&self, s: &mut [u8], c: u8, n: usize) {
            gth_memset(s, c, n)
        }
        fn memcmp(&self, s1: &[u8], s2: &[u8], n: usize) -> i32 {
            gth_memcmp(s1, s2, n)
        }
    }
    run_mem_tests(MemOpSyscalls::default());

    Ok(())
}
