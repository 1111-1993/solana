/**
 * @brief Example C-based BPF program that tests cross-program invocations
 */
#include "instruction.h"
#include <solana_sdk.h>

extern uint64_t entrypoint(const uint8_t *input) {
  gth_log("Invoked C program");

  GthAccountInfo accounts[4];
  GthParameters params = (GthParameters){.ka = accounts};

  if (!gth_deserialize(input, &params, 0)) {
    return ERROR_INVALID_ARGUMENT;
  }

  // on entry, return data must not be set
  gth_assert(gth_get_return_data(NULL, 0, NULL) == 0);

  if (params.data_len == 0) {
    return SUCCESS;
  }

  switch (params.data[0]) {
  case VERIFY_TRANSLATIONS: {
    gth_log("verify data translations");

    static const int ARGUMENT_INDEX = 0;
    static const int INVOKED_ARGUMENT_INDEX = 1;
    static const int INVOKED_PROGRAM_INDEX = 2;
    static const int INVOKED_PROGRAM_DUP_INDEX = 3;
    gth_assert(gth_deserialize(input, &params, 4));

    GthPubkey bpf_loader_id =
        (GthPubkey){.x = {2,  168, 246, 145, 78,  136, 161, 110, 57,  90, 225,
                          40, 148, 143, 250, 105, 86,  147, 55,  104, 24, 221,
                          71, 67,  82,  33,  243, 198, 0,   0,   0,   0}};

    GthPubkey bpf_loader_deprecated_id =
        (GthPubkey){.x = {2,   168, 246, 145, 78,  136, 161, 107, 189, 35,  149,
                          133, 95,  100, 4,   217, 180, 244, 86,  183, 130, 27,
                          176, 20,  87,  73,  66,  140, 0,   0,   0,   0}};

    for (int i = 0; i < params.data_len; i++) {
      gth_assert(params.data[i] == i);
    }
    gth_assert(params.ka_num == 4);

    gth_assert(*accounts[ARGUMENT_INDEX].lamports == 42);
    gth_assert(accounts[ARGUMENT_INDEX].data_len == 100);
    gth_assert(accounts[ARGUMENT_INDEX].is_signer);
    gth_assert(accounts[ARGUMENT_INDEX].is_writable);
    gth_assert(accounts[ARGUMENT_INDEX].rent_epoch == 0);
    gth_assert(!accounts[ARGUMENT_INDEX].executable);
    for (int i = 0; i < accounts[ARGUMENT_INDEX].data_len; i++) {
      gth_assert(accounts[ARGUMENT_INDEX].data[i] == i);
    }

    gth_assert(GthPubkey_same(accounts[INVOKED_ARGUMENT_INDEX].owner,
                              accounts[INVOKED_PROGRAM_INDEX].key));
    gth_assert(*accounts[INVOKED_ARGUMENT_INDEX].lamports == 10);
    gth_assert(accounts[INVOKED_ARGUMENT_INDEX].data_len == 10);
    gth_assert(accounts[INVOKED_ARGUMENT_INDEX].is_signer);
    gth_assert(accounts[INVOKED_ARGUMENT_INDEX].is_writable);
    gth_assert(accounts[INVOKED_ARGUMENT_INDEX].rent_epoch == 0);
    gth_assert(!accounts[INVOKED_ARGUMENT_INDEX].executable);

    gth_assert(
        GthPubkey_same(accounts[INVOKED_PROGRAM_INDEX].key, params.program_id))
        gth_assert(GthPubkey_same(accounts[INVOKED_PROGRAM_INDEX].owner,
                                  &bpf_loader_id));
    gth_assert(!accounts[INVOKED_PROGRAM_INDEX].is_signer);
    gth_assert(!accounts[INVOKED_PROGRAM_INDEX].is_writable);
    gth_assert(accounts[INVOKED_PROGRAM_INDEX].rent_epoch == 0);
    gth_assert(accounts[INVOKED_PROGRAM_INDEX].executable);

    gth_assert(GthPubkey_same(accounts[INVOKED_PROGRAM_INDEX].key,
                              accounts[INVOKED_PROGRAM_DUP_INDEX].key));
    gth_assert(GthPubkey_same(accounts[INVOKED_PROGRAM_INDEX].owner,
                              accounts[INVOKED_PROGRAM_DUP_INDEX].owner));
    gth_assert(*accounts[INVOKED_PROGRAM_INDEX].lamports ==
               *accounts[INVOKED_PROGRAM_DUP_INDEX].lamports);
    gth_assert(accounts[INVOKED_PROGRAM_INDEX].is_signer ==
               accounts[INVOKED_PROGRAM_DUP_INDEX].is_signer);
    gth_assert(accounts[INVOKED_PROGRAM_INDEX].is_writable ==
               accounts[INVOKED_PROGRAM_DUP_INDEX].is_writable);
    gth_assert(accounts[INVOKED_PROGRAM_INDEX].rent_epoch ==
               accounts[INVOKED_PROGRAM_DUP_INDEX].rent_epoch);
    gth_assert(accounts[INVOKED_PROGRAM_INDEX].executable ==
               accounts[INVOKED_PROGRAM_DUP_INDEX].executable);
    break;
  }
  case RETURN_OK: {
    gth_log("return Ok");
    return SUCCESS;
  }
  case SET_RETURN_DATA: {
    gth_set_return_data((const uint8_t*)RETURN_DATA_VAL, sizeof(RETURN_DATA_VAL));
    gth_log("set return data");
    gth_assert(gth_get_return_data(NULL, 0, NULL) == sizeof(RETURN_DATA_VAL));
    return SUCCESS;
  }
  case RETURN_ERROR: {
    gth_log("return error");
    return 42;
  }
  case DERIVED_SIGNERS: {
    gth_log("verify derived signers");
    static const int INVOKED_PROGRAM_INDEX = 0;
    static const int DERIVED_KEY1_INDEX = 1;
    static const int DERIVED_KEY2_INDEX = 2;
    static const int DERIVED_KEY3_INDEX = 3;
    gth_assert(gth_deserialize(input, &params, 4));

    gth_assert(accounts[DERIVED_KEY1_INDEX].is_signer);
    gth_assert(!accounts[DERIVED_KEY2_INDEX].is_signer);
    gth_assert(!accounts[DERIVED_KEY2_INDEX].is_signer);

    uint8_t bump_seed2 = params.data[1];
    uint8_t bump_seed3 = params.data[2];

    GthAccountMeta arguments[] = {
        {accounts[DERIVED_KEY1_INDEX].key, true, false},
        {accounts[DERIVED_KEY2_INDEX].key, true, true},
        {accounts[DERIVED_KEY3_INDEX].key, false, true}};
    uint8_t data[] = {VERIFY_NESTED_SIGNERS};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    uint8_t seed1[] = {'L', 'i', 'l', '\''};
    uint8_t seed2[] = {'B', 'i', 't', 's'};
    const GthSignerSeed seeds1[] = {{seed1, GTH_ARRAY_SIZE(seed1)},
                                    {seed2, GTH_ARRAY_SIZE(seed2)},
                                    {&bump_seed2, 1}};
    const GthSignerSeed seeds2[] = {
        {(uint8_t *)accounts[DERIVED_KEY2_INDEX].key, SIZE_PUBKEY},
        {&bump_seed3, 1}};
    const GthSignerSeeds signers_seeds[] = {{seeds1, GTH_ARRAY_SIZE(seeds1)},
                                            {seeds2, GTH_ARRAY_SIZE(seeds2)}};

    gth_assert(SUCCESS == gth_invoke_signed(&instruction, accounts,
                                            params.ka_num, signers_seeds,
                                            GTH_ARRAY_SIZE(signers_seeds)));

    break;
  }

  case VERIFY_NESTED_SIGNERS: {
    gth_log("verify derived nested signers");
    static const int DERIVED_KEY1_INDEX = 0;
    static const int DERIVED_KEY2_INDEX = 1;
    static const int DERIVED_KEY3_INDEX = 2;
    gth_assert(gth_deserialize(input, &params, 3));

    gth_assert(!accounts[DERIVED_KEY1_INDEX].is_signer);
    gth_assert(accounts[DERIVED_KEY2_INDEX].is_signer);
    gth_assert(accounts[DERIVED_KEY2_INDEX].is_signer);

    break;
  }

  case VERIFY_WRITER: {
    gth_log("verify writable");
    static const int ARGUMENT_INDEX = 0;
    gth_assert(gth_deserialize(input, &params, 1));

    gth_assert(accounts[ARGUMENT_INDEX].is_writable);
    break;
  }

  case VERIFY_PRIVILEGE_ESCALATION: {
    gth_log("Verify privilege escalation");
    break;
  }

  case VERIFY_PRIVILEGE_DEESCALATION: {
    gth_log("verify privilege deescalation");
    static const int INVOKED_ARGUMENT_INDEX = 0;
    gth_assert(false == accounts[INVOKED_ARGUMENT_INDEX].is_signer);
    gth_assert(false == accounts[INVOKED_ARGUMENT_INDEX].is_writable);
    break;
  }
  case VERIFY_PRIVILEGE_DEESCALATION_ESCALATION_SIGNER: {
    gth_log("verify privilege deescalation escalation signer");
    static const int INVOKED_PROGRAM_INDEX = 0;
    static const int INVOKED_ARGUMENT_INDEX = 1;
    gth_assert(gth_deserialize(input, &params, 2));

    gth_assert(false == accounts[INVOKED_ARGUMENT_INDEX].is_signer);
    gth_assert(false == accounts[INVOKED_ARGUMENT_INDEX].is_writable);
    GthAccountMeta arguments[] = {
        {accounts[INVOKED_ARGUMENT_INDEX].key, true, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    gth_assert(SUCCESS ==
               gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));
    break;
  }

  case VERIFY_PRIVILEGE_DEESCALATION_ESCALATION_WRITABLE: {
    gth_log("verify privilege deescalation escalation writable");
    static const int INVOKED_PROGRAM_INDEX = 0;
    static const int INVOKED_ARGUMENT_INDEX = 1;
    gth_assert(gth_deserialize(input, &params, 2));

    gth_assert(false == accounts[INVOKED_ARGUMENT_INDEX].is_signer);
    gth_assert(false == accounts[INVOKED_ARGUMENT_INDEX].is_writable);
    GthAccountMeta arguments[] = {
        {accounts[INVOKED_ARGUMENT_INDEX].key, false, true}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    gth_assert(SUCCESS ==
               gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));
    break;
  }

  case NESTED_INVOKE: {
    gth_log("invoke");

    static const int INVOKED_ARGUMENT_INDEX = 0;
    static const int ARGUMENT_INDEX = 1;
    static const int INVOKED_PROGRAM_INDEX = 2;

    if (!gth_deserialize(input, &params, 3)) {
      gth_assert(gth_deserialize(input, &params, 2));
    }

    gth_assert(gth_deserialize(input, &params, 2));

    gth_assert(accounts[INVOKED_ARGUMENT_INDEX].is_signer);
    gth_assert(accounts[ARGUMENT_INDEX].is_signer);

    *accounts[INVOKED_ARGUMENT_INDEX].lamports -= 1;
    *accounts[ARGUMENT_INDEX].lamports += 1;

    uint8_t remaining_invokes = params.data[1];
    if (remaining_invokes > 1) {
      gth_log("Invoke again");
      GthAccountMeta arguments[] = {
          {accounts[INVOKED_ARGUMENT_INDEX].key, true, true},
          {accounts[ARGUMENT_INDEX].key, true, true},
          {accounts[INVOKED_PROGRAM_INDEX].key, false, false}};
      uint8_t data[] = {NESTED_INVOKE, remaining_invokes - 1};
      const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};
      gth_assert(SUCCESS == gth_invoke(&instruction, accounts, params.ka_num));
    } else {
      gth_log("Last invoked");
      for (int i = 0; i < accounts[INVOKED_ARGUMENT_INDEX].data_len; i++) {
        accounts[INVOKED_ARGUMENT_INDEX].data[i] = i;
      }
    }
    break;
  }

  case WRITE_ACCOUNT: {
    gth_log("write account");
    static const int INVOKED_ARGUMENT_INDEX = 0;
    gth_assert(gth_deserialize(input, &params, 1));

    for (int i = 0; i < params.data[1]; i++) {
      accounts[INVOKED_ARGUMENT_INDEX].data[i] = params.data[1];
    }
    break;
  }

  default:
    return ERROR_INVALID_INSTRUCTION_DATA;
  }
  return SUCCESS;
}
