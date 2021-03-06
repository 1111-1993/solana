/**
 * @brief Example C-based BPF program that moves funds from one account to
 * another
 */
#include <solana_sdk.h>

/**
 * Number of SolKeyedAccount expected. The program should bail if an
 * unexpected number of accounts are passed to the program's entrypoint
 */
#define NUM_KA 3

extern uint64_t entrypoint(const uint8_t *input) {
  GthAccountInfo ka[NUM_KA];
  GthParameters params = (GthParameters) { .ka = ka };

  if (!gth_deserialize(input, &params, GTH_ARRAY_SIZE(ka))) {
    return ERROR_INVALID_ARGUMENT;
  }

  if (!params.ka[0].is_signer) {
    gth_log("Transaction not signed by key 0");
    return ERROR_MISSING_REQUIRED_SIGNATURES;
  }

  int64_t weis = *(int64_t *)params.data;
  if (*params.ka[0].weis >= weis) {
    *params.ka[0].weis -= weis;
    *params.ka[2].weis += weis;
    // gth_log_64(0, 0, *ka[0].weis, *ka[2].weis, weis);
  } else {
    // gth_log_64(0, 0, 0xFF, *ka[0].weis, weis);
  }
  return SUCCESS;
}
