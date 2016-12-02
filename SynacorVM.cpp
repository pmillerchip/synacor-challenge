//----------------------------------------------------------------------------
// SynacorVM
//----------------------------------------------------------------------------

#include "SynacorVM.h"

//----------------------------------------------------------------------------
// Constructor
SynacorVM::SynacorVM()
{
  programCounter = 0;
}

//----------------------------------------------------------------------------
// Destructor
SynacorVM::~SynacorVM()
{
}

//-----------------------------------------------------------------------------
unsigned int
SynacorVM::readMemory(unsigned int address) const
{
  address = (address & 0xffff) * 2;
  return(vmMemory[address] | (vmMemory[address + 1] << 8));
}

//-----------------------------------------------------------------------------
void
SynacorVM::writeMemory(unsigned int address, unsigned int value)
{
  address = (address & 0xffff) * 2;
  vmMemory[address] = value & 0xff;
  vmMemory[address + 1] = (value >> 8) & 0xff;
}

//-----------------------------------------------------------------------------
unsigned int
SynacorVM::readFromPC()
{
  unsigned int rv = readMemory(programCounter);
  programCounter = (programCounter + 1) & 0xffff;
  return(rv);
}

//-----------------------------------------------------------------------------
void
SynacorVM::writeToPC(unsigned int value)
{
  writeMemory(programCounter, value);
  programCounter = (programCounter + 1) & 0xffff;
}


//-----------------------------------------------------------------------------
static std::string
disParam(unsigned int value)
{
  char tmpc[32];
  if ((value & 0x8000) == 0)
  {
    snprintf(tmpc, 32, "%04x", value & 0x7fff);
  }
  else
  {
    snprintf(tmpc, 32, "R%d", value & 0x7fff);
  }
  return(std::string(tmpc));
}

//-----------------------------------------------------------------------------
std::string
SynacorVM::disassemble(unsigned int addr, unsigned int* np) const
{
  unsigned int opcode = readMemory(addr);
  int numParams;
  std::string paramA = disParam(readMemory(addr + 1));
  std::string paramB = disParam(readMemory(addr + 2));
  std::string paramC = disParam(readMemory(addr + 3));

  std::string rv;
  char tmpc[64];
  snprintf(tmpc, 64, "%04x: ", addr);
  rv += tmpc;
  
  switch(opcode)
  {
    case OP_HALT:
      snprintf(tmpc, 64, "HALT");
      numParams = 0;
      break;

    case OP_SET:
      snprintf(tmpc, 64, "SET %s, %s", paramA.data(), paramB.data());
      numParams = 2;
      break;

    case OP_PUSH:
      snprintf(tmpc, 64, "PUSH %s", paramA.data());
      numParams = 1;
      break;

    case OP_POP:
      snprintf(tmpc, 64, "POP %s", paramA.data());
      numParams = 1;
      break;

    case OP_EQ:
      snprintf(tmpc, 64, "EQ %s, %s, %s", paramA.data(), paramB.data(), paramC.data());
      numParams = 3;
      break;

    case OP_GT:
      snprintf(tmpc, 64, "GT %s, %s, %s", paramA.data(), paramB.data(), paramC.data());
      numParams = 3;
      break;

    case OP_JMP:
      snprintf(tmpc, 64, "JMP %s", paramA.data());
      numParams = 1;
      break;

    case OP_JT:
      snprintf(tmpc, 64, "JT %s, %s", paramA.data(), paramB.data());
      numParams = 2;
      break;

    case OP_JF:
      snprintf(tmpc, 64, "JF %s, %s", paramA.data(), paramB.data());
      numParams = 2;
      break;

    case OP_ADD:
      snprintf(tmpc, 64, "ADD %s, %s, %s", paramA.data(), paramB.data(), paramC.data());
      numParams = 3;
      break;

    case OP_MULT:
      snprintf(tmpc, 64, "MULT %s, %s, %s", paramA.data(), paramB.data(), paramC.data());
      numParams = 3;
      break;

    case OP_MOD:
      snprintf(tmpc, 64, "MOD %s, %s, %s", paramA.data(), paramB.data(), paramC.data());
      numParams = 3;
      break;

    case OP_AND:
      snprintf(tmpc, 64, "AND %s, %s, %s", paramA.data(), paramB.data(), paramC.data());
      numParams = 3;
      break;

    case OP_OR:
      snprintf(tmpc, 64, "OR %s, %s, %s", paramA.data(), paramB.data(), paramC.data());
      numParams = 3;
      break;

    case OP_NOT:
      snprintf(tmpc, 64, "NOT %s, %s", paramA.data(), paramB.data());
      numParams = 2;
      break;

    case OP_RMEM:
      snprintf(tmpc, 64, "RMEM %s, %s", paramA.data(), paramB.data());
      numParams = 2;
      break;

    case OP_WMEM:
      snprintf(tmpc, 64, "WMEM %s, %s", paramA.data(), paramB.data());
      numParams = 2;
      break;

    case OP_CALL:
      snprintf(tmpc, 64, "CALL %s", paramA.data());
      numParams = 1;
      break;

    case OP_RET:
      snprintf(tmpc, 64, "RET");
      numParams = 0;
      break;

    case OP_OUT:
      snprintf(tmpc, 64, "OUT %s", paramA.data());
      numParams = 1;
      break;

    case OP_IN:
      snprintf(tmpc, 64, "IN %s", paramA.data());
      numParams = 1;
      break;

    case OP_NOOP:
      snprintf(tmpc, 64, "NOOP");
      numParams = 0;
      break;

    default:
      snprintf(tmpc, 64, "Opcode %u", opcode);
      numParams = 0;
  }

  rv += tmpc;
  if (np != nullptr) *np = numParams;
  
  return(rv);
}
