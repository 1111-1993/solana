import { expect } from "chai";
import { weisToGth, WEIS_PER_GTH } from "utils";
import BN from "bn.js";

describe("weisToGth", () => {
  it("0 weis", () => {
    expect(weisToGth(new BN(0))).to.eq(0.0);
  });

  it("1 wei", () => {
    expect(weisToGth(new BN(1))).to.eq(0.000000001);
    expect(weisToGth(new BN(-1))).to.eq(-0.000000001);
  });

  it("1 GTH", () => {
    expect(weisToGth(new BN(WEIS_PER_GTH))).to.eq(1.0);
    expect(weisToGth(new BN(-WEIS_PER_GTH))).to.eq(-1.0);
  });

  it("u64::MAX weis", () => {
    expect(weisToGth(new BN(2).pow(new BN(64)))).to.eq(
      18446744073.709551615
    );
    expect(weisToGth(new BN(2).pow(new BN(64)).neg())).to.eq(
      -18446744073.709551615
    );
  });
});
