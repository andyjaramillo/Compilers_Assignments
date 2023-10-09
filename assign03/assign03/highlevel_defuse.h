#ifndef HIGHLEVEL_DEFUSE_H
#define HIGHLEVEL_DEFUSE_H

class Instruction;

namespace HighLevel {

bool is_def(Instruction *ins);
bool is_use(Instruction *ins, unsigned operand_index);

};

#endif // HIGHLEVEL_DEFUSE_H
