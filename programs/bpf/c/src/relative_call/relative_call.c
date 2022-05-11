/**
 * @brief test program that generates BPF PC relative call instructions
 */
#include <solana_sdk.h>

void __attribute__ ((noinline)) helper() {
  gth_log(__func__);
}

extern uint64_t entrypoint(const uint8_t *input) {
  gth_log(__func__);
  helper();
  return SUCCESS;
}
