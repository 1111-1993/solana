import { expect } from "chai";
import { lamportsToGth, LAMPORTS_PER_GTH } from "utils";
import BN from "bn.js";

describe("lamportsToGth", () => {
  it("0 lamports", () => {
    expect(lamportsToGth(new BN(0))).to.eq(0.0);
  });

  it("1 lamport", () => {
    expect(lamportsToGth(new BN(1))).to.eq(0.000000001);
    expect(lamportsToGth(new BN(-1))).to.eq(-0.000000001);
  });

  it("1 GTH", () => {
    expect(lamportsToGth(new BN(LAMPORTS_PER_GTH))).to.eq(1.0);
    expect(lamportsToGth(new BN(-LAMPORTS_PER_GTH))).to.eq(-1.0);
  });

  it("u64::MAX lamports", () => {
    expect(lamportsToGth(new BN(2).pow(new BN(64)))).to.eq(
      18446744073.709551615
    );
    expect(lamportsToGth(new BN(2).pow(new BN(64)).neg())).to.eq(
      -18446744073.709551615
    );
  });
});
