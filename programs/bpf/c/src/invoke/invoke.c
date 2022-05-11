/**
 * @brief Example C-based BPF program that tests cross-program invocations
 */
#include "../invoked/instruction.h"
#include <gth/entrypoint.h>
#include <gth/cpi.h>
#include <gth/pubkey.h>
#include <gth/log.h>
#include <gth/assert.h>
#include <gth/deserialize.h>
#include <gth/return_data.h>

static const uint8_t TEST_SUCCESS = 1;
static const uint8_t TEST_PRIVILEGE_ESCALATION_SIGNER = 2;
static const uint8_t TEST_PRIVILEGE_ESCALATION_WRITABLE = 3;
static const uint8_t TEST_PPROGRAM_NOT_EXECUTABLE = 4;
static const uint8_t TEST_EMPTY_ACCOUNTS_SLICE = 5;
static const uint8_t TEST_CAP_SEEDS = 6;
static const uint8_t TEST_CAP_SIGNERS = 7;
static const uint8_t TEST_ALLOC_ACCESS_VIOLATION = 8;
static const uint8_t TEST_INSTRUCTION_DATA_TOO_LARGE = 9;
static const uint8_t TEST_INSTRUCTION_META_TOO_LARGE = 10;
static const uint8_t TEST_RETURN_ERROR = 11;
static const uint8_t TEST_PRIVILEGE_DEESCALATION_ESCALATION_SIGNER = 12;
static const uint8_t TEST_PRIVILEGE_DEESCALATION_ESCALATION_WRITABLE = 13;
static const uint8_t TEST_WRITABLE_DEESCALATION_WRITABLE = 14;
static const uint8_t TEST_NESTED_INVOKE_TOO_DEEP = 15;
static const uint8_t TEST_EXECUTABLE_LAMPORTS = 16;
static const uint8_t TEST_CALL_PRECOMPILE = 17;
static const uint8_t ADD_LAMPORTS = 18;
static const uint8_t TEST_RETURN_DATA_TOO_LARGE = 19;
static const uint8_t TEST_DUPLICATE_PRIVILEGE_ESCALATION_SIGNER = 20;
static const uint8_t TEST_DUPLICATE_PRIVILEGE_ESCALATION_WRITABLE = 21;

static const int MINT_INDEX = 0;
static const int ARGUMENT_INDEX = 1;
static const int INVOKED_PROGRAM_INDEX = 2;
static const int INVOKED_ARGUMENT_INDEX = 3;
static const int INVOKED_PROGRAM_DUP_INDEX = 4;
static const int ARGUMENT_DUP_INDEX = 5;
static const int DERIVED_KEY1_INDEX = 6;
static const int DERIVED_KEY2_INDEX = 7;
static const int DERIVED_KEY3_INDEX = 8;
static const int SYSTEM_PROGRAM_INDEX = 9;
static const int FROM_INDEX = 10;
static const int ED25519_PROGRAM_INDEX = 11;
static const int INVOKE_PROGRAM_INDEX = 12;

uint64_t do_nested_invokes(uint64_t num_nested_invokes,
                           GthAccountInfo *accounts, uint64_t num_accounts) {
  gth_assert(accounts[ARGUMENT_INDEX].is_signer);

  *accounts[ARGUMENT_INDEX].lamports -= 5;
  *accounts[INVOKED_ARGUMENT_INDEX].lamports += 5;

  GthAccountMeta arguments[] = {
      {accounts[INVOKED_ARGUMENT_INDEX].key, true, true},
      {accounts[ARGUMENT_INDEX].key, true, true},
      {accounts[INVOKED_PROGRAM_INDEX].key, false, false}};
  uint8_t data[] = {NESTED_INVOKE, num_nested_invokes};
  const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                      arguments, GTH_ARRAY_SIZE(arguments),
                                      data, GTH_ARRAY_SIZE(data)};

  gth_log("First invoke");
  gth_assert(SUCCESS == gth_invoke(&instruction, accounts, num_accounts));
  gth_log("2nd invoke from first program");
  gth_assert(SUCCESS == gth_invoke(&instruction, accounts, num_accounts));

  gth_assert(*accounts[ARGUMENT_INDEX].lamports ==
             42 - 5 + (2 * num_nested_invokes));
  gth_assert(*accounts[INVOKED_ARGUMENT_INDEX].lamports ==
             10 + 5 - (2 * num_nested_invokes));

  return SUCCESS;
}

extern uint64_t entrypoint(const uint8_t *input) {
  gth_log("Invoke C program");

  GthAccountInfo accounts[13];
  GthParameters params = (GthParameters){.ka = accounts};

  if (!gth_deserialize(input, &params, GTH_ARRAY_SIZE(accounts))) {
    return ERROR_INVALID_ARGUMENT;
  }

  uint8_t bump_seed1 = params.data[1];
  uint8_t bump_seed2 = params.data[2];
  uint8_t bump_seed3 = params.data[3];

  switch (params.data[0]) {
  case TEST_SUCCESS: {
    gth_log("Call system program create account");
    {
      uint64_t from_lamports = *accounts[FROM_INDEX].lamports;
      uint64_t to_lamports = *accounts[DERIVED_KEY1_INDEX].lamports;
      GthAccountMeta arguments[] = {
          {accounts[FROM_INDEX].key, true, true},
          {accounts[DERIVED_KEY1_INDEX].key, true, true}};
      uint8_t data[4 + 8 + 8 + 32];
      *(uint64_t *)(data + 4) = 42;
      *(uint64_t *)(data + 4 + 8) = MAX_PERMITTED_DATA_INCREASE;
      gth_memcpy(data + 4 + 8 + 8, params.program_id, SIZE_PUBKEY);
      const GthInstruction instruction = {accounts[SYSTEM_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};
      uint8_t seed1[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                         ' ', 'b', 'u', 't', 't', 'e', 'r'};
      const GthSignerSeed seeds1[] = {{seed1, GTH_ARRAY_SIZE(seed1)},
                                      {&bump_seed1, 1}};
      const GthSignerSeeds signers_seeds[] = {{seeds1, GTH_ARRAY_SIZE(seeds1)}};
      gth_assert(SUCCESS == gth_invoke_signed(&instruction, accounts,
                                              GTH_ARRAY_SIZE(accounts),
                                              signers_seeds,
                                              GTH_ARRAY_SIZE(signers_seeds)));
      gth_assert(*accounts[FROM_INDEX].lamports == from_lamports - 42);
      gth_assert(*accounts[DERIVED_KEY1_INDEX].lamports == to_lamports + 42);
      gth_assert(GthPubkey_same(accounts[DERIVED_KEY1_INDEX].owner,
                                params.program_id));
      gth_assert(accounts[DERIVED_KEY1_INDEX].data_len ==
                 MAX_PERMITTED_DATA_INCREASE);
      gth_assert(
          accounts[DERIVED_KEY1_INDEX].data[MAX_PERMITTED_DATA_INCREASE - 1] ==
          0);
      accounts[DERIVED_KEY1_INDEX].data[MAX_PERMITTED_DATA_INCREASE - 1] = 0x0f;
      gth_assert(
          accounts[DERIVED_KEY1_INDEX].data[MAX_PERMITTED_DATA_INCREASE - 1] ==
          0x0f);
      for (uint8_t i = 0; i < 20; i++) {
        accounts[DERIVED_KEY1_INDEX].data[i] = i;
      }
    }

    gth_log("Call system program transfer");
    {
      uint64_t from_lamports = *accounts[FROM_INDEX].lamports;
      uint64_t to_lamports = *accounts[DERIVED_KEY1_INDEX].lamports;
      GthAccountMeta arguments[] = {
          {accounts[FROM_INDEX].key, true, true},
          {accounts[DERIVED_KEY1_INDEX].key, true, false}};
      uint8_t data[] = {2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
      const GthInstruction instruction = {accounts[SYSTEM_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};
      gth_assert(SUCCESS ==
                 gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));
      gth_assert(*accounts[FROM_INDEX].lamports == from_lamports - 1);
      gth_assert(*accounts[DERIVED_KEY1_INDEX].lamports == to_lamports + 1);
    }

    gth_log("Test data translation");
    {
      for (int i = 0; i < accounts[ARGUMENT_INDEX].data_len; i++) {
        accounts[ARGUMENT_INDEX].data[i] = i;
      }

      GthAccountMeta arguments[] = {
          {accounts[ARGUMENT_INDEX].key, true, true},
          {accounts[INVOKED_ARGUMENT_INDEX].key, true, true},
          {accounts[INVOKED_PROGRAM_INDEX].key, false, false},
          {accounts[INVOKED_PROGRAM_DUP_INDEX].key, false, false}};
      uint8_t data[] = {VERIFY_TRANSLATIONS, 1, 2, 3, 4, 5};
      const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};

      gth_assert(SUCCESS ==
                 gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));
    }

    gth_log("Test no instruction data");
    {
      GthAccountMeta arguments[] = {{accounts[ARGUMENT_INDEX].key, true, true}};
      uint8_t data[] = {};
      const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};

      gth_assert(SUCCESS ==
                 gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));
    }

    gth_log("Test return data");
    {
      GthAccountMeta arguments[] = {{accounts[ARGUMENT_INDEX].key, true, true}};
      uint8_t data[] = { SET_RETURN_DATA };
      uint8_t buf[100];

      const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};

      // set some return data, so that the callee can check it is cleared
      gth_set_return_data((uint8_t[]){1, 2, 3, 4}, 4);

      gth_assert(SUCCESS ==
                 gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));

      GthPubkey setter;

      uint64_t ret = gth_get_return_data(data, sizeof(data), &setter);

      gth_assert(ret == sizeof(RETURN_DATA_VAL));

      gth_assert(gth_memcmp(data, RETURN_DATA_VAL, sizeof(RETURN_DATA_VAL)));
      gth_assert(GthPubkey_same(&setter, accounts[INVOKED_PROGRAM_INDEX].key));
    }

    gth_log("Test create_program_address");
    {
      uint8_t seed1[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                         ' ', 'b', 'u', 't', 't', 'e', 'r'};
      const GthSignerSeed seeds1[] = {{seed1, GTH_ARRAY_SIZE(seed1)},
                                      {&bump_seed1, 1}};
      GthPubkey address;
      gth_assert(SUCCESS ==
                 gth_create_program_address(seeds1, GTH_ARRAY_SIZE(seeds1),
                                            params.program_id, &address));
      gth_assert(GthPubkey_same(&address, accounts[DERIVED_KEY1_INDEX].key));
    }

    gth_log("Test try_find_program_address");
    {
      uint8_t seed[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                        ' ', 'b', 'u', 't', 't', 'e', 'r'};
      const GthSignerSeed seeds[] = {{seed, GTH_ARRAY_SIZE(seed)}};
      GthPubkey address;
      uint8_t bump_seed;
      gth_assert(SUCCESS == gth_try_find_program_address(
                                seeds, GTH_ARRAY_SIZE(seeds), params.program_id,
                                &address, &bump_seed));
      gth_assert(GthPubkey_same(&address, accounts[DERIVED_KEY1_INDEX].key));
      gth_assert(bump_seed == bump_seed1);
    }

    gth_log("Test derived signers");
    {
      gth_assert(!accounts[DERIVED_KEY1_INDEX].is_signer);
      gth_assert(!accounts[DERIVED_KEY2_INDEX].is_signer);
      gth_assert(!accounts[DERIVED_KEY3_INDEX].is_signer);

      GthAccountMeta arguments[] = {
          {accounts[INVOKED_PROGRAM_INDEX].key, false, false},
          {accounts[DERIVED_KEY1_INDEX].key, true, true},
          {accounts[DERIVED_KEY2_INDEX].key, true, false},
          {accounts[DERIVED_KEY3_INDEX].key, false, false}};
      uint8_t data[] = {DERIVED_SIGNERS, bump_seed2, bump_seed3};
      const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};
      uint8_t seed1[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                         ' ', 'b', 'u', 't', 't', 'e', 'r'};
      const GthSignerSeed seeds1[] = {{seed1, GTH_ARRAY_SIZE(seed1)},
                                      {&bump_seed1, 1}};
      const GthSignerSeeds signers_seeds[] = {{seeds1, GTH_ARRAY_SIZE(seeds1)}};
      gth_assert(SUCCESS == gth_invoke_signed(&instruction, accounts,
                                              GTH_ARRAY_SIZE(accounts),
                                              signers_seeds,
                                              GTH_ARRAY_SIZE(signers_seeds)));
    }

    gth_log("Test readonly with writable account");
    {
      GthAccountMeta arguments[] = {
          {accounts[INVOKED_ARGUMENT_INDEX].key, true, false}};
      uint8_t data[] = {VERIFY_WRITER};
      const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};

      gth_assert(SUCCESS ==
                 gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));
    }

    gth_log("Test nested invoke");
    {
      gth_assert(SUCCESS == do_nested_invokes(4, accounts, params.ka_num));
    }

    gth_log("Test privilege deescalation");
    {
      gth_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_signer);
      gth_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_writable);
      GthAccountMeta arguments[] = {
          {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
      uint8_t data[] = {VERIFY_PRIVILEGE_DEESCALATION};
      const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};
      gth_assert(SUCCESS ==
                 gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));
    }

    gth_log("Verify data values are retained and updated");
    for (int i = 0; i < accounts[ARGUMENT_INDEX].data_len; i++) {
      gth_assert(accounts[ARGUMENT_INDEX].data[i] == i);
    }
    for (int i = 0; i < accounts[INVOKED_ARGUMENT_INDEX].data_len; i++) {
      gth_assert(accounts[INVOKED_ARGUMENT_INDEX].data[i] == i);
    }

    gth_log("Verify data write before ro cpi call");
    {
      for (int i = 0; i < accounts[ARGUMENT_INDEX].data_len; i++) {
        accounts[ARGUMENT_INDEX].data[i] = 0;
      }

      GthAccountMeta arguments[] = {
          {accounts[ARGUMENT_INDEX].key, false, false}};
      uint8_t data[] = {VERIFY_PRIVILEGE_DEESCALATION};
      const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, GTH_ARRAY_SIZE(arguments),
                                          data, GTH_ARRAY_SIZE(data)};
      gth_assert(SUCCESS ==
                 gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));

      for (int i = 0; i < accounts[ARGUMENT_INDEX].data_len; i++) {
        gth_assert(accounts[ARGUMENT_INDEX].data[i] == 0);
      }
    }
    break;
  }
  case TEST_PRIVILEGE_ESCALATION_SIGNER: {
    gth_log("Test privilege escalation signer");
    GthAccountMeta arguments[] = {
        {accounts[DERIVED_KEY3_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    gth_assert(SUCCESS ==
               gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));

    // Signer privilege escalation will always fail the whole transaction
    instruction.accounts[0].is_signer = true;
    gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts));
    break;
  }
  case TEST_PRIVILEGE_ESCALATION_WRITABLE: {
    gth_log("Test privilege escalation writable");
    GthAccountMeta arguments[] = {
        {accounts[DERIVED_KEY3_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    gth_assert(SUCCESS ==
               gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));

    // Writable privilege escalation will always fail the whole transaction
    instruction.accounts[0].is_writable = true;
    gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts));
    break;
  }
  case TEST_PPROGRAM_NOT_EXECUTABLE: {
    gth_log("Test program not executable");
    GthAccountMeta arguments[] = {
        {accounts[DERIVED_KEY3_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const GthInstruction instruction = {accounts[ARGUMENT_INDEX].key, arguments,
                                        GTH_ARRAY_SIZE(arguments), data,
                                        GTH_ARRAY_SIZE(data)};
    return gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts));
  }
  case TEST_EMPTY_ACCOUNTS_SLICE: {
    gth_log("Empty accounts slice");

    GthAccountMeta arguments[] = {
        {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
    uint8_t data[] = {};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};

    gth_assert(SUCCESS == gth_invoke(&instruction, 0, 0));
    break;
  }
  case TEST_CAP_SEEDS: {
    gth_log("Test cap seeds");
    GthAccountMeta arguments[] = {};
    uint8_t data[] = {};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    uint8_t seed[] = {"seed"};
    const GthSignerSeed seeds[] = {
        {seed, GTH_ARRAY_SIZE(seed)}, {seed, GTH_ARRAY_SIZE(seed)},
        {seed, GTH_ARRAY_SIZE(seed)}, {seed, GTH_ARRAY_SIZE(seed)},
        {seed, GTH_ARRAY_SIZE(seed)}, {seed, GTH_ARRAY_SIZE(seed)},
        {seed, GTH_ARRAY_SIZE(seed)}, {seed, GTH_ARRAY_SIZE(seed)},
        {seed, GTH_ARRAY_SIZE(seed)}, {seed, GTH_ARRAY_SIZE(seed)},
        {seed, GTH_ARRAY_SIZE(seed)}, {seed, GTH_ARRAY_SIZE(seed)},
        {seed, GTH_ARRAY_SIZE(seed)}, {seed, GTH_ARRAY_SIZE(seed)},
        {seed, GTH_ARRAY_SIZE(seed)}, {seed, GTH_ARRAY_SIZE(seed)},
        {seed, GTH_ARRAY_SIZE(seed)},
    };
    const GthSignerSeeds signers_seeds[] = {{seeds, GTH_ARRAY_SIZE(seeds)}};
    gth_assert(SUCCESS == gth_invoke_signed(
                              &instruction, accounts, GTH_ARRAY_SIZE(accounts),
                              signers_seeds, GTH_ARRAY_SIZE(signers_seeds)));
    break;
  }
  case TEST_CAP_SIGNERS: {
    gth_log("Test cap signers");
    GthAccountMeta arguments[] = {};
    uint8_t data[] = {};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    uint8_t seed[] = {"seed"};
    const GthSignerSeed seed1[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed2[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed3[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed4[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed5[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed6[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed7[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed8[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed9[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed10[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed11[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed12[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed13[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed14[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed15[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed16[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeed seed17[] = {{seed, GTH_ARRAY_SIZE(seed)}};
    const GthSignerSeeds signers_seeds[] = {
        {seed1, GTH_ARRAY_SIZE(seed1)},   {seed2, GTH_ARRAY_SIZE(seed2)},
        {seed3, GTH_ARRAY_SIZE(seed3)},   {seed4, GTH_ARRAY_SIZE(seed4)},
        {seed5, GTH_ARRAY_SIZE(seed5)},   {seed6, GTH_ARRAY_SIZE(seed6)},
        {seed7, GTH_ARRAY_SIZE(seed7)},   {seed8, GTH_ARRAY_SIZE(seed8)},
        {seed9, GTH_ARRAY_SIZE(seed9)},   {seed10, GTH_ARRAY_SIZE(seed10)},
        {seed11, GTH_ARRAY_SIZE(seed11)}, {seed12, GTH_ARRAY_SIZE(seed12)},
        {seed13, GTH_ARRAY_SIZE(seed13)}, {seed14, GTH_ARRAY_SIZE(seed14)},
        {seed15, GTH_ARRAY_SIZE(seed15)}, {seed16, GTH_ARRAY_SIZE(seed16)},
        {seed17, GTH_ARRAY_SIZE(seed17)}};
    gth_assert(SUCCESS == gth_invoke_signed(
                              &instruction, accounts, GTH_ARRAY_SIZE(accounts),
                              signers_seeds, GTH_ARRAY_SIZE(signers_seeds)));
    break;
  }
  case TEST_ALLOC_ACCESS_VIOLATION: {
    gth_log("Test resize violation");
    GthAccountMeta arguments[] = {
        {accounts[FROM_INDEX].key, true, true},
        {accounts[DERIVED_KEY1_INDEX].key, true, true}};
    uint8_t data[4 + 8 + 8 + 32];
    *(uint64_t *)(data + 4) = 42;
    *(uint64_t *)(data + 4 + 8) = MAX_PERMITTED_DATA_INCREASE;
    gth_memcpy(data + 4 + 8 + 8, params.program_id, SIZE_PUBKEY);
    const GthInstruction instruction = {accounts[SYSTEM_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    uint8_t seed1[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                       ' ', 'b', 'u', 't', 't', 'e', 'r'};
    const GthSignerSeed seeds1[] = {{seed1, GTH_ARRAY_SIZE(seed1)},
                                    {&bump_seed1, 1}};
    const GthSignerSeeds signers_seeds[] = {{seeds1, GTH_ARRAY_SIZE(seeds1)}};

    GthAccountInfo derived_account = {
        .key = accounts[DERIVED_KEY1_INDEX].key,
        .lamports = accounts[DERIVED_KEY1_INDEX].lamports,
        .data_len = accounts[DERIVED_KEY1_INDEX].data_len,
        // Point to top edge of heap, attempt to allocate into unprivileged
        // memory
        .data = (uint8_t *)0x300007ff8,
        .owner = accounts[DERIVED_KEY1_INDEX].owner,
        .rent_epoch = accounts[DERIVED_KEY1_INDEX].rent_epoch,
        .is_signer = accounts[DERIVED_KEY1_INDEX].is_signer,
        .is_writable = accounts[DERIVED_KEY1_INDEX].is_writable,
        .executable = accounts[DERIVED_KEY1_INDEX].executable,
    };
    const GthAccountInfo invoke_accounts[] = {
        accounts[FROM_INDEX], accounts[SYSTEM_PROGRAM_INDEX], derived_account};
    gth_assert(SUCCESS ==
               gth_invoke_signed(&instruction,
                                 (const GthAccountInfo *)invoke_accounts, 3,
                                 signers_seeds, GTH_ARRAY_SIZE(signers_seeds)));
    break;
  }
  case TEST_INSTRUCTION_DATA_TOO_LARGE: {
    gth_log("Test instruction data too large");
    GthAccountMeta arguments[] = {};
    uint8_t *data = gth_calloc(1500, 1);
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, 1500};
    const GthSignerSeeds signers_seeds[] = {};
    gth_assert(SUCCESS == gth_invoke_signed(
                              &instruction, accounts, GTH_ARRAY_SIZE(accounts),
                              signers_seeds, GTH_ARRAY_SIZE(signers_seeds)));

    break;
  }
  case TEST_INSTRUCTION_META_TOO_LARGE: {
    gth_log("Test instruction meta too large");
    GthAccountMeta *arguments = gth_calloc(40, sizeof(GthAccountMeta));
    gth_log_64(0, 0, 0, 0, (uint64_t)arguments);
    gth_assert(0 != arguments);
    uint8_t data[] = {};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, 40, data,
                                        GTH_ARRAY_SIZE(data)};
    const GthSignerSeeds signers_seeds[] = {};
    gth_assert(SUCCESS == gth_invoke_signed(
                              &instruction, accounts, GTH_ARRAY_SIZE(accounts),
                              signers_seeds, GTH_ARRAY_SIZE(signers_seeds)));

    break;
  }
  case TEST_RETURN_ERROR: {
    gth_log("Test return error");
    GthAccountMeta arguments[] = {{accounts[ARGUMENT_INDEX].key, false, true}};
    uint8_t data[] = {RETURN_ERROR};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};

    gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts));
    break;
  }
  case TEST_PRIVILEGE_DEESCALATION_ESCALATION_SIGNER: {
    gth_log("Test privilege deescalation escalation signer");
    gth_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_signer);
    gth_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_writable);
    GthAccountMeta arguments[] = {
        {accounts[INVOKED_PROGRAM_INDEX].key, false, false},
        {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_DEESCALATION_ESCALATION_SIGNER};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    gth_assert(SUCCESS ==
               gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));
    break;
  }
  case TEST_PRIVILEGE_DEESCALATION_ESCALATION_WRITABLE: {
    gth_log("Test privilege deescalation escalation writable");
    gth_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_signer);
    gth_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_writable);
    GthAccountMeta arguments[] = {
        {accounts[INVOKED_PROGRAM_INDEX].key, false, false},
        {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_DEESCALATION_ESCALATION_WRITABLE};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    gth_assert(SUCCESS ==
               gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));
    break;
  }
  case TEST_WRITABLE_DEESCALATION_WRITABLE: {
    gth_log("Test writable deescalation");
    uint8_t buffer[10];
    for (int i = 0; i < 10; i++) {
      buffer[i] = accounts[INVOKED_ARGUMENT_INDEX].data[i];
    }
    GthAccountMeta arguments[] = {
        {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
    uint8_t data[] = {WRITE_ACCOUNT, 10};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts));

    for (int i = 0; i < 10; i++) {
      gth_assert(buffer[i] == accounts[INVOKED_ARGUMENT_INDEX].data[i]);
    }
    break;
  }
  case TEST_NESTED_INVOKE_TOO_DEEP: {
    do_nested_invokes(5, accounts, params.ka_num);
    break;
  }
  case TEST_EXECUTABLE_LAMPORTS: {
    gth_log("Test executable lamports");
    accounts[ARGUMENT_INDEX].executable = true;
    *accounts[ARGUMENT_INDEX].lamports -= 1;
    *accounts[DERIVED_KEY1_INDEX].lamports +=1;
    GthAccountMeta arguments[] = {
      {accounts[ARGUMENT_INDEX].key, true, false},
      {accounts[DERIVED_KEY1_INDEX].key, true, false},
    };
    uint8_t data[] = {ADD_LAMPORTS, 0, 0, 0};
    GthPubkey program_id;
    gth_memcpy(&program_id, params.program_id, sizeof(GthPubkey));
    const GthInstruction instruction = {&program_id,
					arguments, GTH_ARRAY_SIZE(arguments),
					data, GTH_ARRAY_SIZE(data)};
    gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts));
    *accounts[ARGUMENT_INDEX].lamports += 1;
    break;
  }
  case TEST_CALL_PRECOMPILE: {
    gth_log("Test calling precompile from cpi");
    GthAccountMeta arguments[] = {};
    uint8_t data[] = {};
    const GthInstruction instruction = {accounts[ED25519_PROGRAM_INDEX].key,
					arguments, GTH_ARRAY_SIZE(arguments),
					data, GTH_ARRAY_SIZE(data)};
    gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts));
    break;
  }
  case ADD_LAMPORTS: {
    *accounts[0].lamports += 1;
     break;
  }
  case TEST_RETURN_DATA_TOO_LARGE: {
    gth_log("Test setting return data too long");
    // The actual buffer doesn't matter, just pass null
    gth_set_return_data(NULL, 1027);
    break;
  }
  case TEST_DUPLICATE_PRIVILEGE_ESCALATION_SIGNER: {
    gth_log("Test duplicate privilege escalation signer");
    GthAccountMeta arguments[] = {
        {accounts[DERIVED_KEY3_INDEX].key, false, false},
        {accounts[DERIVED_KEY3_INDEX].key, false, false},
        {accounts[DERIVED_KEY3_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    gth_assert(SUCCESS ==
               gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));

    // Signer privilege escalation will always fail the whole transaction
    instruction.accounts[1].is_signer = true;
    gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts));
    break;
  }
  case TEST_DUPLICATE_PRIVILEGE_ESCALATION_WRITABLE: {
    gth_log("Test duplicate privilege escalation writable");
    GthAccountMeta arguments[] = {
        {accounts[DERIVED_KEY3_INDEX].key, false, false},
        {accounts[DERIVED_KEY3_INDEX].key, false, false},
        {accounts[DERIVED_KEY3_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const GthInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, GTH_ARRAY_SIZE(arguments),
                                        data, GTH_ARRAY_SIZE(data)};
    gth_assert(SUCCESS ==
               gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts)));

    // Writable privilege escalation will always fail the whole transaction
    instruction.accounts[1].is_writable = true;
    gth_invoke(&instruction, accounts, GTH_ARRAY_SIZE(accounts));
    break;
  }

  default:
    gth_panic();
  }

  return SUCCESS;
}
