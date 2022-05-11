#pragma once
/**
 * @brief Solana Cross-Program Invocation
 */

#include <gth/types.h>
#include <gth/pubkey.h>
#include <gth/entrypoint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Account Meta
 */
typedef struct {
  GthPubkey *pubkey; /** An account's public key */
  bool is_writable; /** True if the `pubkey` can be loaded as a read-write account */
  bool is_signer; /** True if an Instruction requires a Transaction signature matching `pubkey` */
} GthAccountMeta;

/**
 * Instruction
 */
typedef struct {
  GthPubkey *program_id; /** Pubkey of the instruction processor that executes this instruction */
  GthAccountMeta *accounts; /** Metadata for what accounts should be passed to the instruction processor */
  uint64_t account_len; /** Number of GthAccountMetas */
  uint8_t *data; /** Opaque data passed to the instruction processor */
  uint64_t data_len; /** Length of the data in bytes */
} GthInstruction;

/**
 * Internal cross-program invocation function
 */
uint64_t gth_invoke_signed_c(
  const GthInstruction *instruction,
  const GthAccountInfo *account_infos,
  int account_infos_len,
  const GthSignerSeeds *signers_seeds,
  int signers_seeds_len
);

/**
 * Invoke another program and sign for some of the keys
 *
 * @param instruction Instruction to process
 * @param account_infos Accounts used by instruction
 * @param account_infos_len Length of account_infos array
 * @param seeds Seed bytes used to sign program accounts
 * @param seeds_len Length of the seeds array
 */
static uint64_t gth_invoke_signed(
    const GthInstruction *instruction,
    const GthAccountInfo *account_infos,
    int account_infos_len,
    const GthSignerSeeds *signers_seeds,
    int signers_seeds_len
) {
  return gth_invoke_signed_c(
    instruction,
    account_infos,
    account_infos_len,
    signers_seeds,
    signers_seeds_len
  );
}
/**
 * Invoke another program
 *
 * @param instruction Instruction to process
 * @param account_infos Accounts used by instruction
 * @param account_infos_len Length of account_infos array
*/
static uint64_t gth_invoke(
    const GthInstruction *instruction,
    const GthAccountInfo *account_infos,
    int account_infos_len
) {
  const GthSignerSeeds signers_seeds[] = {{}};
  return gth_invoke_signed(
    instruction,
    account_infos,
    account_infos_len,
    signers_seeds,
    0
  );
}

#ifdef __cplusplus
}
#endif

/**@}*/
