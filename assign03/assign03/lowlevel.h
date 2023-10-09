#ifndef LOWLEVEL_H
#define LOWLEVEL_H


// Machine registers, machine instructions, and low-level code formatting
// support for x86-64

// Enumeration for naming machine registers.
// Note that these names correspond to the full 64-bit name
// of each register. However, when used as the register number
// in an Operand, the Operand kind will select whether the
// 64-bit (Operand::MREG64), 32-bit (Operand::MREG32),
// 16-bit (Operand::MREG16), or 8-bit (Operand::MREG8)
// variant of the register is being accessed.
enum MachineReg {
  MREG_RAX,
  MREG_RBX,
  MREG_RCX,
  MREG_RDX,
  MREG_RSI,
  MREG_RDI,
  MREG_RSP,
  MREG_RBP,
  MREG_R8,
  MREG_R9,
  MREG_R10,
  MREG_R11,
  MREG_R12,
  MREG_R13,
  MREG_R14,
  MREG_R15,

  // This is not an actual machine register,
  // it is just here to have a value 1 greater than the
  // last actual machine register
  MREG_END,
};

// x86-64 assembly language instruction mnemonics.
// You may add other instructions here. If you do, you will need
// to modify the lowlevel_opcode_to_str() function to add
// support for translating the instruction's enum value to a string.
// For instructions which allow an operand size suffix, there should
// be 4 enum values, in the order "b", "w", "l", and "q".
// (The same way that the high-level instructions with operand size
// suffixes use this order.)
enum LowLevelOpcode {
  MINS_NOP,
  MINS_MOVB,
  MINS_MOVW,
  MINS_MOVL,
  MINS_MOVQ,
  MINS_ADDB,
  MINS_ADDW,
  MINS_ADDL,
  MINS_ADDQ,
  MINS_SUBB,
  MINS_SUBW,
  MINS_SUBL,
  MINS_SUBQ,
  MINS_LEAQ, // only one variant, pointers are always 64-bit
  MINS_JMP,
  MINS_JL,   // these are in the same order as the corresponding SETx instructions
  MINS_JLE,
  MINS_JG,
  MINS_JGE,
  MINS_JE,
  MINS_JNE,
  MINS_JB,
  MINS_JBE,
  MINS_JA,
  MINS_JAE,
  MINS_CMPB,
  MINS_CMPW,
  MINS_CMPL,
  MINS_CMPQ,
  MINS_CALL,
  MINS_IMULL,
  MINS_IMULQ,
  MINS_IDIVL,
  MINS_IDIVQ,
  MINS_CDQ,
  MINS_CQTO,
  MINS_PUSHQ,
  MINS_POPQ,
  MINS_RET,
  MINS_MOVSBW,
  MINS_MOVSBL,
  MINS_MOVSBQ,
  MINS_MOVSWL,
  MINS_MOVSWQ,
  MINS_MOVSLQ,
  MINS_MOVZBW,
  MINS_MOVZBL,
  MINS_MOVZBQ,
  MINS_MOVZWL,
  MINS_MOVZWQ,
  MINS_MOVZLQ,
  MINS_SETL,  // these are in the same order as the corresponding Jx instructions
  MINS_SETLE,
  MINS_SETG,
  MINS_SETGE,
  MINS_SETE,
  MINS_SETNE,
  MINS_XORB,
  MINS_XORW,
  MINS_XORL,
  MINS_XORQ,
  MINS_INCB,
  MINS_INCW,
  MINS_INCL,
  MINS_INCQ,
  MINS_DECB,
  MINS_DECW,
  MINS_DECL,
  MINS_DECQ,
};

const char *lowlevel_opcode_to_str(LowLevelOpcode opcode);

#endif // LOWLEVEL_H
