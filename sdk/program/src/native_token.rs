#![allow(clippy::integer_arithmetic)]
/// There are 10^9 lamports in one GTH
pub const LAMPORTS_PER_GTH: u64 = 1_000_000_000;

/// Approximately convert fractional native tokens (lamports) into native tokens (GTH)
pub fn lamports_to_gth(lamports: u64) -> f64 {
    lamports as f64 / LAMPORTS_PER_GTH as f64
}

/// Approximately convert native tokens (GTH) into fractional native tokens (lamports)
pub fn gth_to_lamports(gth: f64) -> u64 {
    (gth * LAMPORTS_PER_GTH as f64) as u64
}

use std::fmt::{Debug, Display, Formatter, Result};
pub struct Gth(pub u64);

impl Gth {
    fn write_in_gth(&self, f: &mut Formatter) -> Result {
        write!(
            f,
            "◎{}.{:09}",
            self.0 / LAMPORTS_PER_GTH,
            self.0 % LAMPORTS_PER_GTH
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
