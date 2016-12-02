//----------------------------------------------------------------------------
// SynacorVM
//----------------------------------------------------------------------------

#ifndef _INCL_SYNACORVM_H
#define _INCL_SYNACORVM_H 1

#include <string>
#include <vector>

enum OpCode
{
  OP_HALT,
  OP_SET,
  OP_PUSH,
  OP_POP,
  OP_EQ,
  OP_GT,
  OP_JMP,
  OP_JT,
  OP_JF,
  OP_ADD,
  OP_MULT,
  OP_MOD,
  OP_AND,
  OP_OR,
  OP_NOT,
  OP_RMEM,
  OP_WMEM,
  OP_CALL,
  OP_RET,
  OP_OUT,
  OP_IN,
  OP_NOOP
};

//----------------------------------------------------------------------------
class SynacorVM
{
  public:
                              SynacorVM();
                              ~SynacorVM();

    // Read VM's memory space
    unsigned int              readMemory(unsigned int address) const;

    // Write the given value to the VM's memory space
    void                      writeMemory(unsigned int address, unsigned int value);

    // Read a value from the address pointed to by the program counter, and
    // then increment the program counter.
    unsigned int              readFromPC();

    // Write a value to the address pointed to by the program counter, and
    // then increment the program counter.
    void                      writeToPC(unsigned int value);

    // Disassemble at the given address
    std::string               disassemble(unsigned int address, unsigned int* numParams) const;

    // Variables
    unsigned char             vmMemory[0xffff * 2];
    unsigned int              vmRegisters[8];
    std::vector<unsigned int> vmStack;
    unsigned int              programCounter;

  private:
};

#endif
