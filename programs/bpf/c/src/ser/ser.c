/**
 * @brief Example C-based BPF sanity rogram that prints out the parameters
 * passed to it
 */
#include <solana_sdk.h>

extern uint64_t entrypoint(const uint8_t *input) {
  GthAccountInfo ka[2];
  GthParameters params = (GthParameters){.ka = ka};

  gth_log(__FILE__);

  if (!gth_deserialize(input, &params, GTH_ARRAY_SIZE(ka))) {
    return ERROR_INVALID_ARGUMENT;
  }

  char ka_data[] = {1, 2, 3};
  GthPubkey ka_owner;
  gth_memset(ka_owner.x, 0, SIZE_PUBKEY); // set to system program

  gth_assert(params.ka_num == 2);
  for (int i = 0; i < 2; i++) {
    gth_assert(*params.ka[i].weis == 42);
    gth_assert(!gth_memcmp(params.ka[i].data, ka_data, 4));
    gth_assert(GthPubkey_same(params.ka[i].owner, &ka_owner));
    gth_assert(params.ka[i].is_signer == false);
    gth_assert(params.ka[i].is_writable == false);
    gth_assert(params.ka[i].executable == false);
  }

  char data[] = {4, 5, 6, 7};
  gth_assert(params.data_len = 4);
  gth_assert(!gth_memcmp(params.data, data, 4));

  return SUCCESS;
}
