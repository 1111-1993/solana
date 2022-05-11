/**
 * @brief return data Syscall test
 */
#include <solana_sdk.h>

#define DATA "the quick brown fox jumps over the lazy dog"

extern uint64_t entrypoint(const uint8_t *input) {
  uint8_t buf[1024];
  GthPubkey me;

  // There should be no return data on entry
  uint64_t ret = gth_get_return_data(NULL, 0, NULL);

  gth_assert(ret == 0);

  // set some return data
  gth_set_return_data((const uint8_t*)DATA, sizeof(DATA));

  // ensure the length is correct
  ret = gth_get_return_data(NULL, 0, &me);
  gth_assert(ret == sizeof(DATA));

  // try getting a subset
  ret = gth_get_return_data(buf, 4, &me);

  gth_assert(ret == sizeof(DATA));

  gth_assert(!gth_memcmp(buf, "the ", 4));

  // try getting the whole thing
  ret = gth_get_return_data(buf, sizeof(buf), &me);

  gth_assert(ret == sizeof(DATA));

  gth_assert(!gth_memcmp(buf, (const uint8_t*)DATA, sizeof(DATA)));

  // done
  return SUCCESS;
}
