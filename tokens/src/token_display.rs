use {
    solana_account_decoder::parse_token::real_number_string_trimmed,
    solana_sdk::native_token::weis_to_gth,
    std::{
        fmt::{Debug, Display, Formatter, Result},
        ops::Add,
    },
};

const GTH_SYMBOL: &str = "◎";

#[derive(PartialEq)]
pub enum TokenType {
    Gth,
    SplToken,
}

pub struct Token {
    amount: u64,
    decimals: u8,
    token_type: TokenType,
}

impl Token {
    fn write_with_symbol(&self, f: &mut Formatter) -> Result {
        match &self.token_type {
            TokenType::Gth => {
                let amount = weis_to_gth(self.amount);
                write!(f, "{}{}", GTH_SYMBOL, amount)
            }
            TokenType::SplToken => {
                let amount = real_number_string_trimmed(self.amount, self.decimals);
                write!(f, "{} tokens", amount)
            }
        }
    }

    pub fn gth(amount: u64) -> Self {
        Self {
            amount,
            decimals: 9,
            token_type: TokenType::Gth,
        }
    }

    pub fn spl_token(amount: u64, decimals: u8) -> Self {
        Self {
            amount,
            decimals,
            token_type: TokenType::SplToken,
        }
    }
}

impl Display for Token {
    fn fmt(&self, f: &mut Formatter) -> Result {
        self.write_with_symbol(f)
    }
}

impl Debug for Token {
    fn fmt(&self, f: &mut Formatter) -> Result {
        self.write_with_symbol(f)
    }
}

impl Add for Token {
    type Output = Token;

    fn add(self, other: Self) -> Self {
        if self.token_type == other.token_type {
            Self {
                amount: self.amount + other.amount,
                decimals: self.decimals,
                token_type: self.token_type,
            }
        } else {
            self
        }
    }
}
