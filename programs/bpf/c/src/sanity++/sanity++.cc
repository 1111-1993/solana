/**
 * @brief Example C++-based BPF program that prints out the parameters
 * passed to it
 */
#include <solana_sdk.h>

/**
 * Custom error for when input serialization fails
 */
#define INVALID_INPUT 1

extern uint64_t entrypoint(const uint8_t *input) {
  GthAccountInfo ka[1];
  GthParameters params = (GthParameters) { .ka = ka };

  gth_log(__FILE__);

  if (!gth_deserialize(input, &params, GTH_ARRAY_SIZE(ka))) {
    return ERROR_INVALID_ARGUMENT;
  }

  // Log the provided input parameters.  In the case of  the no-op
  // program, no account keys or input data are expected but real
  // programs will have specific requirements so they can do their work.
  gth_log_params(&params);
  return SUCCESS;
}
