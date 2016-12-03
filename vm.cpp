#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "SynacorVM.h"

SynacorVM vm;
std::string cachedLine;
unsigned int lineLength;
unsigned int lineOffset;
unsigned int old1;
unsigned int old2;
unsigned int old3;
bool trace = false;

//-----------------------------------------------------------------------------
unsigned int
vmReadParam(unsigned int value)
{
  if ((value & 0x8000) == 0) return(value & 0x7fff);
  unsigned int registerNum = value & 0x7fff;
  if (registerNum > 7)
  {
    fprintf(stderr, ">>> ERROR: Attempt to read register %u\n", registerNum);
    exit(1);
  }
  return(vm.vmRegisters[registerNum]);
}

//-----------------------------------------------------------------------------
unsigned int
vmReadRegisterNum(unsigned int value)
{
  if ((value & 0x8000) == 0)
  {
    fprintf(stderr, ">>> ERROR: Constant %u when register expected\n", value);
    exit(1);
  }
  unsigned int registerNum = value & 0x7fff;
  if (registerNum > 7)
  {
    fprintf(stderr, ">>> ERROR: Attempt to reference register %u", registerNum);
    exit(1);
  }
  return(registerNum);
}

//-----------------------------------------------------------------------------
unsigned int
vmIn()
{
  if (lineOffset < cachedLine.size()) return(cachedLine[lineOffset++]);
  
  // Need to read a line
  size_t sz;
  char* lineBuffer;
  bool done = false;
  while(!done)
  {
    lineBuffer = nullptr;
    lineLength = getline(&lineBuffer, &sz, stdin);
    cachedLine = lineBuffer;
    free(lineBuffer);
    if (cachedLine == "exit\n")
    {
      exit(0);
    }
    else if (cachedLine == "show regs\n")
    {
      for(int i=0; i < 8; ++i) printf(">>> R%d=%u\n", i, vm.vmRegisters[i]);
      printf(">>> PC=%u\n", vm.programCounter);
    }
    else if (cachedLine == "set r7\n")
    {
      vm.vmRegisters[7] = 1;
    }
    else if (cachedLine == "dump\n")
    {
      unsigned int addr = 5483;
      unsigned int numParams;
      for(int i=0; i < 1000; ++i)
      {
        printf(">>> %s\n", vm.disassemble(addr, &numParams).data());
        addr += numParams;
      }
      exit(0);
    }
    else if (cachedLine == "hack\n")
    {
      //writeMemory(4550, OP_WMEM);
      //writeMemory(4551, 3952);
      //writeMemory(4552, 30);
      //writeMemory(4553, OP_SET);
      //writeMemory(4554, 32768);
      //writeMemory(4555, 30);
      //writeMemory(4556, OP_NOOP);
      old1 = vm.readMemory(5485);
      old2 = vm.readMemory(5489);
      old3 = vm.readMemory(5490);
      vm.writeMemory(5485, 6);
      vm.writeMemory(5489, OP_NOOP);
      vm.writeMemory(5490, OP_NOOP);
      vm.vmRegisters[7] = 25734;
    }
    else if (cachedLine == "set weight\n")
    {
      vm.writeMemory(3952, 30);
      vm.writeMemory(5485, old1);
      vm.writeMemory(5489, old2);
      vm.writeMemory(5490, old3);
    }
    else if (cachedLine == "trace\n")
    {
      trace = true;
    }
    else
    {
      done = true;
    }
  }
  lineOffset = 1;
  return(cachedLine[0]);
}

//-----------------------------------------------------------------------------
int
main(int argc, char** argv)
{
  std::string filename = "challenge.bin";

  if (argc > 1) filename = argv[1];

  FILE* fd = fopen(filename.data(), "r");
  if (fd == nullptr)
  {
    fprintf(stderr, "Cannot open program '%s'\n", filename.data());
    return(1);
  }
  
  fseek(fd, 0, SEEK_END);
  long fileSize = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  if (fread(vm.vmMemory, fileSize, 1, fd) != 1)
  {
    fprintf(stderr, "Cannot load program\n");
    return(1);
  }
  fclose(fd);
  
  printf(">>> Loaded %ld bytes, starting...\n", fileSize);
  
  vm.programCounter = 0;
  lineLength = 0;
  lineOffset = 0;

  unsigned int opcode;
  unsigned int currentPC;
  unsigned int paramA;
  unsigned int paramB;
  unsigned int paramC;
  bool running = true;
  trace = false;
  while(running)
  {
    if (trace) printf(">>> %s\n", vm.disassemble(vm.programCounter, nullptr).data());

    currentPC = vm.programCounter;
    opcode = vm.readFromPC();
    if (opcode > 21)
    {
      fprintf(stderr, ">>> ERROR: Invalid opcode %u at address 0x%04x\n",
        opcode, currentPC);
      return(1);
    }
    
    switch(opcode)
    {
      case OP_HALT:
        printf(">>> HALT at address %u\n", currentPC);
        running = false;
        break;

      case OP_SET:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = paramB;
        break;

      case OP_PUSH:
        paramA = vmReadParam(vm.readFromPC());
        vm.vmStack.push_back(paramA);
        break;

      case OP_POP:
        paramA = vmReadRegisterNum(vm.readFromPC());
        if (vm.vmStack.empty())
        {
          fprintf(stderr, ">>> ERROR: Pop empty stack at address 0x%04x\n",
            currentPC);
          return(1);
        }
        vm.vmRegisters[paramA] = vm.vmStack.back();
        vm.vmStack.pop_back();
        break;

      case OP_EQ:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        paramC = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = ((paramB == paramC) ? 1: 0);
        break;

      case OP_GT:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        paramC = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = ((paramB > paramC) ? 1: 0);
        break;

      case OP_JMP:
        vm.programCounter = vmReadParam(vm.readFromPC());
        break;

      case OP_JT:
        paramA = vmReadParam(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        if (paramA != 0) vm.programCounter = paramB;
        break;

      case OP_JF:
        paramA = vmReadParam(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        if (paramA == 0) vm.programCounter = paramB;
        break;

      case OP_ADD:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        paramC = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = (paramB + paramC) & 0x7fff;
        break;

      case OP_MULT:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        paramC = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = (paramB * paramC) & 0x7fff;
        break;

      case OP_MOD:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        paramC = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = (paramB % paramC) & 0x7fff;
        break;

      case OP_AND:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        paramC = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = (paramB & paramC) & 0x7fff;
        break;

      case OP_OR:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        paramC = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = (paramB | paramC) & 0x7fff;
        break;

      case OP_NOT:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = (~paramB) & 0x7fff;
        break;

      case OP_RMEM:
        paramA = vmReadRegisterNum(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        vm.vmRegisters[paramA] = vm.readMemory(paramB);
        break;

      case OP_WMEM:
        paramA = vmReadParam(vm.readFromPC());
        paramB = vmReadParam(vm.readFromPC());
        vm.writeMemory(paramA, paramB);
        break;

      case OP_CALL:
        paramA = vmReadParam(vm.readFromPC());
        vm.vmStack.push_back(vm.programCounter);
        vm.programCounter = paramA;
        break;

      case OP_RET:
        if (vm.vmStack.empty())
        {
          printf(">>> Normal return HALT\n");
          running = false;
        }
        else
        {
          vm.programCounter = vm.vmStack.back();
          vm.vmStack.pop_back();
        }
        break;

      case OP_OUT:
        putchar(vmReadParam(vm.readFromPC()));
        break;

      case OP_IN:
        paramA = vmReadRegisterNum(vm.readFromPC());
        vm.vmRegisters[paramA] = vmIn();
        break;

      case OP_NOOP:
        break;

      default:
        fprintf(stderr, ">>> ERROR: Unimplemented opcode %u at address 0x%04x\n",
          opcode, currentPC);
        return(1);
    }
  }
  
  printf(">>> VM Exit\n");
  printf("VM registers are:\n");
  for(int i = 0; i < 8; ++i)
  {
    printf("R%d = 0x%04x\n", i, vm.vmRegisters[i]);
  }

  return(0);
}
