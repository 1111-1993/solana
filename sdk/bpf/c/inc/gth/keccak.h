#pragma once
/**
 * @brief Solana keccak system call
**/

#include <gth/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Length of a Keccak hash result
 */
#define KECCAK_RESULT_LENGTH 32

/**
 * Keccak
 *
 * @param bytes Array of byte arrays
 * @param bytes_len Number of byte arrays
 * @param result 32 byte array to hold the result
 */
uint64_t gth_keccak256(
    const GthBytes *bytes,
    int bytes_len,
    uint8_t *result
);

#ifdef __cplusplus
}
#endif

/**@}*/
