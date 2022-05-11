/**
 * @brief Example C based BPF program that performs operations
 * on floating point values.  The test fails if floating point
 * emulation functions were not linked to the module.
 */
#include <solana_sdk.h>

extern double log2(double arg);

extern uint64_t entrypoint(const uint8_t *input) {
  GthAccountInfo ka[1];
  GthParameters params = (GthParameters) { .ka = ka };

  if (!gth_deserialize(input, &params, GTH_ARRAY_SIZE(ka))) {
    return ERROR_INVALID_ARGUMENT;
  }
  /* test float conversion to int compiles and works */
  uint32_t *data = (uint32_t *)(params.ka[0].data);
  uint32_t new_data = *data + 1;
  *data += 1.5;
  gth_assert(*data == new_data);

  /* test signed division works for FP values */
  double value = (double)new_data + 1.0;
  value /= -2.0;
  gth_assert(value < 0.0);

  /* test that standard math functions are available */
  value = *data + 1.0;
  gth_assert(log2(value) == 1.0);

  return SUCCESS;
}
