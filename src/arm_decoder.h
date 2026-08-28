#pragma once

#include <cstdint>
#include "ir.h"


class ARMDecoder
{
public:

    static IRInstruction decode(
        uint32_t instruction
    );


private:

    // Operand2
    static Operand2 decodeOperand2(
        uint32_t instruction
    );


    // Condition field
    static Condition condition(
        uint32_t instruction
    );

    // Instruction classification

    static bool isBX(
        uint32_t instruction
    );

    static bool isMultiply(
        uint32_t instruction
    );

    static IRInstruction decodeMultiply(
        uint32_t instruction
    );

    static bool isSWI(
        uint32_t instruction
    );

    static bool isMSR(
        uint32_t instruction
    );

    static bool isMRS(
        uint32_t instruction
    );

    static bool isBranch(
        uint32_t instruction
    );

    static bool isBlockTransfer(
        uint32_t instruction
    );

    static bool isDataProcessing(
        uint32_t instruction
    );


    static bool isLoadStore(
        uint32_t instruction
    );


    static bool isHalfwordTransfer(
        uint32_t instruction
    );


    // Instruction decoders

    static IRInstruction decodeBX(
        uint32_t instruction
    );

    static IRInstruction decodeMRS(
        uint32_t instruction
    );

    static IRInstruction decodeMSR(
        uint32_t instruction
    );

    static IRInstruction decodeSWI(
        uint32_t instruction
    );

    static IRInstruction decodeBranch(
        uint32_t instruction
    );


    static IRInstruction decodeDataProcessing(
        uint32_t instruction
    );


    static IRInstruction decodeLoadStore(
        uint32_t instruction
    );


    static IRInstruction decodeHalfwordTransfer(
        uint32_t instruction
    );

    static IRInstruction decodeBlockTransfer(
        uint32_t instruction
    );

};