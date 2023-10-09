#ifndef LOWLEVEL_DEFUSE_H
#define LOWLEVEL_DEFUSE_H

#include <vector>
#include "lowlevel.h"
class Instruction;

namespace LowLevel {

bool is_def(Instruction *ins);

// Unfortunately, because an x86-64 instruction can modify
// multiple registers, we need a way of knowing explicitly
// *which* registers are modified by a def instruction
std::vector<MachineReg> get_def_mregs(Instruction *ins);

// A similar issue exists for uses: some instructions have
// implicit uses that aren't explicit operands
std::vector<MachineReg> get_use_mregs(Instruction *ins);

}

#endif // LOWLEVEL_DEFUSE_H
