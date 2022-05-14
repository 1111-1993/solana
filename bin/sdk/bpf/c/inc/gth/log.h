#pragma once
/**
 * @brief Solana logging utilities
 */

#include <gth/types.h>
#include <gth/string.h>
#include <gth/entrypoint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Prints a string to stdout
 */
void gth_log_(const char *, uint64_t);
#define gth_log(message) gth_log_(message, gth_strlen(message))

/**
 * Prints a 64 bit values represented in hexadecimal to stdout
 */
void gth_log_64_(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
#define gth_log_64 gth_log_64_

/**
 * Prints the current compute unit consumption to stdout
 */
void gth_log_compute_units_();
#define gth_log_compute_units() gth_log_compute_units_()

/**
 * Prints the hexadecimal representation of an array
 *
 * @param array The array to print
 */
static void gth_log_array(const uint8_t *array, int len) {
  for (int j = 0; j < len; j++) {
    gth_log_64(0, 0, 0, j, array[j]);
  }
}

/**
 * Print the base64 representation of some arrays.
 */
void gth_log_data(GthBytes *fields, uint64_t fields_len);

/**
 * Prints the program's input parameters
 *
 * @param params Pointer to a GthParameters structure
 */
static void gth_log_params(const GthParameters *params) {
  gth_log("- Program identifier:");
  gth_log_pubkey(params->program_id);

  gth_log("- Number of KeyedAccounts");
  gth_log_64(0, 0, 0, 0, params->ka_num);
  for (int i = 0; i < params->ka_num; i++) {
    gth_log("  - Is signer");
    gth_log_64(0, 0, 0, 0, params->ka[i].is_signer);
    gth_log("  - Is writable");
    gth_log_64(0, 0, 0, 0, params->ka[i].is_writable);
    gth_log("  - Key");
    gth_log_pubkey(params->ka[i].key);
    gth_log("  - Weis");
    gth_log_64(0, 0, 0, 0, *params->ka[i].weis);
    gth_log("  - data");
    gth_log_array(params->ka[i].data, params->ka[i].data_len);
    gth_log("  - Owner");
    gth_log_pubkey(params->ka[i].owner);
    gth_log("  - Executable");
    gth_log_64(0, 0, 0, 0, params->ka[i].executable);
    gth_log("  - Rent Epoch");
    gth_log_64(0, 0, 0, 0, params->ka[i].rent_epoch);
  }
  gth_log("- Instruction data\0");
  gth_log_array(params->data, params->data_len);
}

#ifdef GTH_TEST
/**
 * Stub functions when building tests
 */
#include <stdio.h>

void gth_log_(const char *s, uint64_t len) {
  printf("Program log: %s\n", s);
}
void gth_log_64(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
  printf("Program log: %llu, %llu, %llu, %llu, %llu\n", arg1, arg2, arg3, arg4, arg5);
}

void gth_log_compute_units_() {
  printf("Program consumption: __ units remaining\n");
}
#endif

#ifdef __cplusplus
}
#endif

/**@}*/
