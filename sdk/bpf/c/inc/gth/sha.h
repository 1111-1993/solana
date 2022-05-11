#pragma once
/**
 * @brief Solana sha system call
 */

#include <gth/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Length of a sha256 hash result
 */
#define SHA256_RESULT_LENGTH 32

/**
 * Sha256
 *
 * @param bytes Array of byte arrays
 * @param bytes_len Number of byte arrays
 * @param result 32 byte array to hold the result
 */
uint64_t gth_sha256(
    const GthBytes *bytes,
    int bytes_len,
    uint8_t *result
);

#ifdef __cplusplus
}
#endif

/**@}*/
