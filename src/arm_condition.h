#pragma once

#include "arm_cpu.h"
#include "ir.h"

inline bool conditionPassed(
    Condition cond,
    const ARMCPU& cpu)
{
    switch (cond) {

        case Condition::EQ:
            return cpu.Z;

        case Condition::NE:
            return !cpu.Z;

        case Condition::CS:
            return cpu.C;

        case Condition::CC:
            return !cpu.C;

        case Condition::MI:
            return cpu.N;

        case Condition::PL:
            return !cpu.N;

        case Condition::VS:
            return cpu.V;

        case Condition::VC:
            return !cpu.V;

        case Condition::HI:
            return cpu.C && !cpu.Z;

        case Condition::LS:
            return !cpu.C || cpu.Z;

        case Condition::GE:
            return cpu.N == cpu.V;

        case Condition::LT:
            return cpu.N != cpu.V;

        case Condition::GT:
            return !cpu.Z && (cpu.N == cpu.V);

        case Condition::LE:
            return cpu.Z || (cpu.N != cpu.V);

        case Condition::AL:
            return true;

        case Condition::NV:
            return false;
    }

    return false;
}