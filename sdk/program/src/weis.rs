use {crate::instruction::InstructionError, thiserror::Error};

#[derive(Debug, Error)]
pub enum WeisError {
    /// arithmetic underflowed
    #[error("Arithmetic underflowed")]
    ArithmeticUnderflow,

    /// arithmetic overflowed
    #[error("Arithmetic overflowed")]
    ArithmeticOverflow,
}

impl From<WeisError> for InstructionError {
    fn from(error: WeisError) -> Self {
        match error {
            WeisError::ArithmeticOverflow => InstructionError::ArithmeticOverflow,
            WeisError::ArithmeticUnderflow => InstructionError::ArithmeticOverflow,
        }
    }
}
