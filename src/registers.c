#include "registers.h"

#include <assert.h>

const char* Register_to_str(Register reg) {
    const char* registers[] = {
        [RAX] = "rax",
        [RBX] = "rbx",
        [RCX] = "rcx",
        [RSP] = "rsp",
        [RBP] = "rbp",
        [RDI] = "rdi",
        [RSI] = "rsi",
        [RDX] = "rdx",
        [R8] = "r8",
        [R9] = "r9",
        [R10] = "r10",
        [R11] = "r11",
        [R12] = "r12",
        [R13] = "r13",
        [R14] = "r14",
        [R15] = "r15"
    };

    return registers[reg];
}


Register Register_param_to_reg(int param) {
    Register registers[] = {
        [0] = RDI,
        [1] = RSI,
        [2] = RDX,
        [3] = RCX,
        [4] = R8,
        [5] = R9
    };

    assert(param >= 0 && param < 6);

    return registers[param];
}
