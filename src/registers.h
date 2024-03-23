#ifndef REGISTERS_H
#define REGISTERS_H

typedef enum Register {
    RAX = 1, // Accumulator
    RBX, // Base
    RCX, // Counter
    RSP, // Stack Pointer
    RBP, // Stack Base Pointer
    RDI, // Destination
    RSI, // Source
    RDX, // Data

    // General Purpose Registers
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15
} Register;

/**
 * @brief Convert a register to a string
 * 
 * @param reg 
 * @return const char* 
 */
const char* Register_to_str(Register reg);

/**
 * @brief Returns the register corresponding to the
 * function argument position:
 * 
 * rdi, rsi, rdx, rcx, r8, r9
 * @param param 
 * @return Register 
 */
Register Register_param_to_reg(int param);

#endif
