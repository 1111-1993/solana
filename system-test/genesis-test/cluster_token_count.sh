#!/usr/bin/env bash

# shellcheck disable=SC1090
# shellcheck disable=SC1091
source "$(dirname "$0")"/get_program_accounts.sh

usage() {
  exitcode=0
  if [[ -n "$1" ]]; then
    exitcode=1
    echo "Error: $*"
  fi
  cat <<EOF
usage: $0 [cluster_rpc_url]

 Report total token distribution of a running cluster owned by the following programs:
   STAKE
   SYSTEM
   VOTE
   CONFIG

 Required arguments:
   cluster_rpc_url  - RPC URL and port for a running Solana cluster (ex: http://34.83.146.144:8899)
EOF
  exit $exitcode
}

function get_cluster_version {
  clusterVersion="$(curl -s -X POST -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","id":1, "method":"getVersion"}' "$url" | jq '.result | ."solana-core" ')"
  echo Cluster software version: "$clusterVersion"
}

function get_token_capitalization {
  totalSupplyWeis="$(curl -s -X POST -H "Content-Type: application/json" -d '{"jsonrpc":"2.0","id":1, "method":"getTotalSupply"}' "$url" | cut -d , -f 2 | cut -d : -f 2)"
  totalSupplySol=$((totalSupplyWeis / WEIS_PER_GTH))

  printf "\n--- Token Capitalization ---\n"
  printf "Total token capitalization %'d GTH\n" "$totalSupplySol"
  printf "Total token capitalization %'d Weis\n" "$totalSupplyWeis"

}

function get_program_account_balance_totals {
  PROGRAM_NAME="$1"

  # shellcheck disable=SC2002
  accountBalancesWeis="$(cat "${PROGRAM_NAME}_account_data.json" | \
    jq '.result | .[] | .account | .weis')"

  totalAccountBalancesWeis=0
  numberOfAccounts=0

  # shellcheck disable=SC2068
  for account in ${accountBalancesWeis[@]}; do
    totalAccountBalancesWeis=$((totalAccountBalancesWeis + account))
    numberOfAccounts=$((numberOfAccounts + 1))
  done
  totalAccountBalancesSol=$((totalAccountBalancesWeis / WEIS_PER_GTH))

  printf "\n--- %s Account Balance Totals ---\n" "$PROGRAM_NAME"
  printf "Number of %s Program accounts: %'.f\n" "$PROGRAM_NAME" "$numberOfAccounts"
  printf "Total token balance in all %s accounts: %'d GTH\n" "$PROGRAM_NAME" "$totalAccountBalancesSol"
  printf "Total token balance in all %s accounts: %'d Weis\n" "$PROGRAM_NAME" "$totalAccountBalancesWeis"

  case $PROGRAM_NAME in
    SYSTEM)
      systemAccountBalanceTotalSol=$totalAccountBalancesSol
      systemAccountBalanceTotalWeis=$totalAccountBalancesWeis
      ;;
    STAKE)
      stakeAccountBalanceTotalSol=$totalAccountBalancesSol
      stakeAccountBalanceTotalWeis=$totalAccountBalancesWeis
      ;;
    VOTE)
      voteAccountBalanceTotalSol=$totalAccountBalancesSol
      voteAccountBalanceTotalWeis=$totalAccountBalancesWeis
      ;;
    CONFIG)
      configAccountBalanceTotalSol=$totalAccountBalancesSol
      configAccountBalanceTotalWeis=$totalAccountBalancesWeis
      ;;
    *)
      echo "Unknown program: $PROGRAM_NAME"
      exit 1
      ;;
  esac
}

function sum_account_balances_totals {
  grandTotalAccountBalancesSol=$((systemAccountBalanceTotalGth+ stakeAccountBalanceTotalGth+ voteAccountBalanceTotalGth+ configAccountBalanceTotalSol))
  grandTotalAccountBalancesWeis=$((systemAccountBalanceTotalWeis + stakeAccountBalanceTotalWeis + voteAccountBalanceTotalWeis + configAccountBalanceTotalWeis))

  printf "\n--- Total Token Distribution in all Account Balances ---\n"
  printf "Total GTH in all Account Balances: %'d\n" "$grandTotalAccountBalancesSol"
  printf "Total Weis in all Account Balances: %'d\n" "$grandTotalAccountBalancesWeis"
}

url=$1
[[ -n $url ]] || usage "Missing required RPC URL"
shift

WEIS_PER_GTH=1000000000 # 1 billion

stakeAccountBalanceTotalSol=
systemAccountBalanceTotalSol=
voteAccountBalanceTotalSol=
configAccountBalanceTotalSol=

stakeAccountBalanceTotalWeis=
systemAccountBalanceTotalWeis=
voteAccountBalanceTotalWeis=
configAccountBalanceTotalWeis=

echo "--- Querying RPC URL: $url ---"
get_cluster_version

get_program_accounts STAKE "$STAKE_PROGRAM_PUBKEY" "$url"
get_program_accounts SYSTEM "$SYSTEM_PROGRAM_PUBKEY" "$url"
get_program_accounts VOTE "$VOTE_PROGRAM_PUBKEY" "$url"
get_program_accounts CONFIG "$CONFIG_PROGRAM_PUBKEY" "$url"

write_program_account_data_csv STAKE
write_program_account_data_csv SYSTEM
write_program_account_data_csv VOTE
write_program_account_data_csv CONFIG

get_token_capitalization

get_program_account_balance_totals STAKE
get_program_account_balance_totals SYSTEM
get_program_account_balance_totals VOTE
get_program_account_balance_totals CONFIG

sum_account_balances_totals
