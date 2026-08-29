#pragma once

#include <cstdint>

#include "ir.h"


class ThumbDecoder
{
public:

    static IRInstruction decode(
        uint16_t instruction
    );


private:

    static IRInstruction decodeMoveImmediate(
        uint16_t instruction
    );

    static IRInstruction decodeCompareImmediate(
        uint16_t instruction
    );

    static IRInstruction decodeAddImmediate(
        uint16_t instruction
    );

    static IRInstruction decodeSubImmediate(
        uint16_t instruction
    );

    static IRInstruction decodeALU(
        uint16_t instruction
    );

    static IRInstruction decodeHighRegister(
        uint16_t instruction
    );

    static IRInstruction decodeLoadStoreImmediate(
        uint16_t instruction
    );

    static IRInstruction decodeBranch(
        uint16_t instruction
    );
};