#![allow(clippy::integer_arithmetic)]
/// There are 10^9 weis in one GTH
pub const WEIS_PER_GTH: u64 = 1_000_000_000;

/// Approximately convert fractional native tokens (weis) into native tokens (GTH)
pub fn weis_to_gth(weis: u64) -> f64 {
    weis as f64 / WEIS_PER_GTH as f64
}

/// Approximately convert native tokens (GTH) into fractional native tokens (weis)
pub fn gth_to_weis(gth: f64) -> u64 {
    (gth * WEIS_PER_GTH as f64) as u64
}

use std::fmt::{Debug, Display, Formatter, Result};
pub struct Gth(pub u64);

impl Gth {
    fn write_in_gth(&self, f: &mut Formatter) -> Result {
        write!(
            f,
            "◎{}.{:09}",
            self.0 / WEIS_PER_GTH,
            self.0 % WEIS_PER_GTH
        )
    }
}

impl Display for Gth {
    fn fmt(&self, f: &mut Formatter) -> Result {
        self.write_in_gth(f)
    }
}

impl Debug for Gth {
    fn fmt(&self, f: &mut Formatter) -> Result {
        self.write_in_gth(f)
    }
}
