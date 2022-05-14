import React from "react";
import { BigNumber } from "bignumber.js";
import { GthBalance } from "utils";

export function BalanceDelta({
  delta,
  isGth = false,
}: {
  delta: BigNumber;
  isGth?: boolean;
}) {
  let gths;

  if (isGth) {
    gths = <GthBalance weis={delta.toNumber()} />;
  }

  if (delta.gt(0)) {
    return (
      <span className="badge bg-success-soft">
        +{isGth? gths : delta.toString()}
      </span>
    );
  } else if (delta.lt(0)) {
    return (
      <span className="badge bg-warning-soft">
        {isGth? <>-{gths}</> : delta.toString()}
      </span>
    );
  }

  return <span className="badge bg-secondary-soft">0</span>;
}
