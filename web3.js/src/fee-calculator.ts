import * as BufferLayout from '@solana/buffer-layout';

/**
 * https://github.com/1111-1993/solana/blob/90bedd7e067b5b8f3ddbb45da00a4e9cabb22c62/sdk/src/fee_calculator.rs#L7-L11
 *
 * @internal
 */
export const FeeCalculatorLayout = BufferLayout.nu64('weisPerSignature');

/**
 * Calculator for transaction fees.
 */
export interface FeeCalculator {
  /** Cost in weis to validate a signature. */
  weisPerSignature: number;
}
