/**
 * @brief Example C-based BPF program that exercises duplicate keyed accounts
 * passed to it
 */
#include <solana_sdk.h>

/**
 * Custom error for when input serialization fails
 */

extern uint64_t entrypoint(const uint8_t *input) {
  GthAccountInfo accounts[5];
  GthParameters params = (GthParameters){.ka = accounts};

  if (!gth_deserialize(input, &params, GTH_ARRAY_SIZE(accounts))) {
    return ERROR_INVALID_ARGUMENT;
  }

  switch (params.data[0]) {
  case (1):
    gth_log("modify first account data");
    accounts[2].data[0] = 1;
    break;
  case (2):
    gth_log("modify first account data");
    accounts[3].data[0] = 2;
    break;
  case (3):
    gth_log("modify both account data");
    accounts[2].data[0] += 1;
    accounts[3].data[0] += 2;
    break;
  case (4):
    gth_log("modify first account weis");
    *accounts[1].weis -= 1;
    *accounts[2].weis += 1;
    break;
  case (5):
    gth_log("modify first account weis");
    *accounts[1].weis -= 2;
    *accounts[3].weis += 2;
    break;
  case (6):
    gth_log("modify both account weis");
    *accounts[1].weis -= 3;
    *accounts[2].weis += 1;
    *accounts[3].weis += 2;
    break;
  case (7):
    gth_log("check account (0,1,2,3) privs");
    gth_assert(accounts[0].is_signer);
    gth_assert(!accounts[1].is_signer);
    gth_assert(accounts[2].is_signer);
    gth_assert(accounts[3].is_signer);

    gth_assert(accounts[0].is_writable);
    gth_assert(accounts[1].is_writable);
    gth_assert(accounts[2].is_writable);
    gth_assert(accounts[3].is_writable);

    if (params.ka_num > 4) {
      {
        GthAccountMeta arguments[] = {{accounts[0].key, true, true},
                                      {accounts[1].key, true, false},
                                      {accounts[2].key, true, false},
                                      {accounts[3].key, false, true}};
        uint8_t data[] = {7};
        const GthInstruction instruction = {
            (GthPubkey *)params.program_id, arguments,
            GTH_ARRAY_SIZE(arguments), data, GTH_ARRAY_SIZE(data)};
        gth_assert(SUCCESS ==
                   gth_invoke(&instruction, accounts, params.ka_num));
      }
      {
        GthAccountMeta arguments[] = {{accounts[0].key, true, true},
                                      {accounts[1].key, true, false},
                                      {accounts[2].key, true, false},
                                      {accounts[3].key, true, false}};
        uint8_t data[] = {3};
        const GthInstruction instruction = {
            (GthPubkey *)params.program_id, arguments,
            GTH_ARRAY_SIZE(arguments), data, GTH_ARRAY_SIZE(data)};
        gth_assert(SUCCESS ==
                   gth_invoke(&instruction, accounts, params.ka_num));
      }
      gth_assert(accounts[2].data[0] == 3);
      gth_assert(accounts[3].data[0] == 3);
    }
    break;
  default:
    gth_log("Unrecognized command");
    return ERROR_INVALID_INSTRUCTION_DATA;
  }
  return SUCCESS;
}
