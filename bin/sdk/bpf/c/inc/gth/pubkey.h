#pragma once
/**
 * @brief Solana Public key
 */

#include <gth/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Size of Public key in bytes
 */
#define SIZE_PUBKEY 32

/**
 * Public key
 */
typedef struct {
  uint8_t x[SIZE_PUBKEY];
} GthPubkey;

/**
 * Prints the hexadecimal representation of a public key
 *
 * @param key The public key to print
 */
void gth_log_pubkey(
  const GthPubkey *pubkey
);

/**
 * Compares two public keys
 *
 * @param one First public key
 * @param two Second public key
 * @return true if the same
 */
static bool GthPubkey_same(const GthPubkey *one, const GthPubkey *two) {
  for (int i = 0; i < sizeof(*one); i++) {
    if (one->x[i] != two->x[i]) {
      return false;
    }
  }
  return true;
}

/**
 * Seed used to create a program address or passed to gth_invoke_signed
 */
typedef struct {
  const uint8_t *addr; /** Seed bytes */
  uint64_t len; /** Length of the seed bytes */
} GthSignerSeed;

/**
 * Seeds used by a signer to create a program address or passed to
 * gth_invoke_signed
 */
typedef struct {
  const GthSignerSeed *addr; /** An arry of a signer's seeds */
  uint64_t len; /** Number of seeds */
} GthSignerSeeds;

/**
 * Create a program address
 *
 * @param seeds Seed bytes used to sign program accounts
 * @param seeds_len Length of the seeds array
 * @param program_id Program id of the signer
 * @param program_address Program address created, filled on return
 */
uint64_t gth_create_program_address(
    const GthSignerSeed *seeds,
    int seeds_len,
    const GthPubkey *program_id,
    GthPubkey *program_address
);

/**
 * Try to find a program address and return corresponding bump seed
 *
 * @param seeds Seed bytes used to sign program accounts
 * @param seeds_len Length of the seeds array
 * @param program_id Program id of the signer
 * @param program_address Program address created, filled on return
 * @param bump_seed Bump seed required to create a valid program address
 */
uint64_t gth_try_find_program_address(
    const GthSignerSeed *seeds,
    int seeds_len,
    const GthPubkey *program_id,
    GthPubkey *program_address,
    uint8_t *bump_seed
);

#ifdef GTH_TEST
/**
 * Stub functions when building tests
 */
#include <stdio.h>

void gth_log_pubkey(
  const GthPubkey *pubkey
) {
  printf("Program log: ");
  for (int i = 0; i < SIZE_PUBKEY; i++) {
    printf("%02 ", pubkey->x[i]);
  }
  printf("\n");
}

#endif

#ifdef __cplusplus
}
#endif

/**@}*/
