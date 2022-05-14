//! Example Rust-based BPF program that moves a wei from one account to another

extern crate solana_program;
use solana_program::{account_info::AccountInfo, entrypoint::ProgramResult, pubkey::Pubkey};

solana_program::entrypoint!(process_instruction);
#[allow(clippy::unnecessary_wraps)]
fn process_instruction(
    _program_id: &Pubkey,
    accounts: &[AccountInfo],
    _instruction_data: &[u8],
) -> ProgramResult {
    // account 0 is the mint and not owned by this program, any debit of its weis
    // should result in a failed program execution.  Test to ensure that this debit
    // is seen by the runtime and fails as expected
    **accounts[0].weis.borrow_mut() -= 1;

    Ok(())
}
