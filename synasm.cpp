#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include "SynacorVM.h"

SynacorVM vm;

enum PassMode
{
  PASSMODE_SYMBOLS,  // Find the location of symbols (labels)
  PASSMODE_ASSEMBLE  // Assemble code using previous labels
};

std::map<std::string, unsigned int> labels;

//-----------------------------------------------------------------------------
bool
runPass(const std::string& filename, PassMode passMode)
{
  std::ifstream file(filename);
  if (!file)
  {
    fprintf(stderr, "Cannot open file '%s'\n", filename.data());
    return(false);
  }

  vm.programCounter = 0;

  printf("Processing file '%s'\n", filename.data());
  std::string line; 
  std::string instr; 
  size_t pos;
  unsigned int currentPC;
  while (std::getline(file, line))
  {
    currentPC = vm.programCounter;

    // Strip comments
    line = line.substr(0, line.find('#'));

    // Get instruction
    pos = line.find(' ');
    instr = line.substr(0, pos);
    if (pos != std::string::npos)
    {
      line = line.substr(pos + 1);
    }

    // Do the opcode
    bool isData = false;
    if      ((instr == "HALT") || (instr == "halt")) vm.writeToPC(OP_HALT);
    else if ((instr == "SET")  || (instr == "set"))  vm.writeToPC(OP_SET);
    else if ((instr == "PUSH") || (instr == "push")) vm.writeToPC(OP_PUSH);
    else if ((instr == "POP")  || (instr == "pop"))  vm.writeToPC(OP_POP);
    else if ((instr == "EQ")   || (instr == "eq"))   vm.writeToPC(OP_EQ);
    else if ((instr == "GT")   || (instr == "gt"))   vm.writeToPC(OP_GT);
    else if ((instr == "JMP")  || (instr == "jmp"))  vm.writeToPC(OP_JMP);
    else if ((instr == "JT")   || (instr == "jt"))   vm.writeToPC(OP_JT);
    else if ((instr == "JF")   || (instr == "jf"))   vm.writeToPC(OP_JF);
    else if ((instr == "ADD")  || (instr == "add"))  vm.writeToPC(OP_ADD);
    else if ((instr == "MULT") || (instr == "mult")) vm.writeToPC(OP_MULT);
    else if ((instr == "MOD")  || (instr == "mod"))  vm.writeToPC(OP_MOD);
    else if ((instr == "AND")  || (instr == "and"))  vm.writeToPC(OP_AND);
    else if ((instr == "OR")   || (instr == "or"))   vm.writeToPC(OP_OR);
    else if ((instr == "NOT")  || (instr == "not"))  vm.writeToPC(OP_NOT);
    else if ((instr == "RMEM") || (instr == "rmem")) vm.writeToPC(OP_RMEM);
    else if ((instr == "WMEM") || (instr == "wmem")) vm.writeToPC(OP_WMEM);
    else if ((instr == "CALL") || (instr == "call")) vm.writeToPC(OP_CALL);
    else if ((instr == "RET")  || (instr == "ret"))  vm.writeToPC(OP_RET);
    else if ((instr == "OUT")  || (instr == "out"))  vm.writeToPC(OP_OUT);
    else if ((instr == "IN")   || (instr == "in"))   vm.writeToPC(OP_IN);
    else if ((instr == "NOOP") || (instr == "noop")) vm.writeToPC(OP_NOOP);
    else if ((instr == "DATA") || (instr == "data")) isData = true;
    else if (instr[0] == '.')
    {
      // Defining a label
      std::string name = instr.substr(1);
      labels[name] = currentPC;
      printf(".%s: %04x\n", name.data(), currentPC);
    }

    if ((currentPC != vm.programCounter) || isData)
    {
      // Process the parameters
      while(line != "")
      {
        // Get instruction
        pos = line.find(' ');
        instr = line.substr(0, pos);
        if (pos != std::string::npos)
        {
          line = line.substr(pos + 1);
        }
        else
        {
          line = "";
        }

        if (instr != "")
        {
          if ((instr[0] == 'r') || (instr[0] == 'R'))
          {
            // Register
            vm.writeToPC((atoi(instr.data() + 1) & 0x7fff) | 0x8000);
          }
          else if (instr[0] == '\'')
          {
            // Character constant
            vm.writeToPC(instr[1]);
          }
          else if (instr[0] == '.')
          {
            // Label
            std::string label = instr.substr(1);
            unsigned int dest = 0;
            if (passMode == PASSMODE_ASSEMBLE)
            {
              if (labels.find(label) == labels.end())
              {
                fprintf(stderr, "Error: Label '%s' not defined\n", label.data());
                exit(1);
              }
              dest = labels[label];
            }
            vm.writeToPC(dest);
          }
          else
          {
            // Constant
            char* endptr;
            vm.writeToPC(strtoul(instr.data(), &endptr, 0) & 0x7fff);
          }
        }
      }

      if (passMode == PASSMODE_ASSEMBLE)
      {
        // Show the assembled instruction
        if (isData)
        {
          printf("%04x: DATA %04x\n", currentPC, vm.readMemory(currentPC));
        }
        else
        {
          printf("%s\n", vm.disassemble(currentPC, nullptr).data());
        }
      }
    }
  }

  return(true);
}

//-----------------------------------------------------------------------------
int
main(int argc, char** argv)
{
  if (argc < 3)
  {
    printf("Synacore assembler\n");
    printf("Usage: %s <source file> <output file>\n", argv[0]);
    return(1);
  }

  std::string filename = argv[1];

  // Resolve all the symbols
  if (!runPass(filename, PASSMODE_SYMBOLS)) return(1);

  // Do the assembly
  if (!runPass(filename, PASSMODE_ASSEMBLE)) return(1);

  // Save VM memory
  printf("VM image is %u VM words (%u bytes)\n", vm.programCounter, vm.programCounter * 2);
  FILE* fd = fopen(argv[2], "wb");
  if (fd == NULL)
  {
    fprintf(stderr, "Error: Cannot open output file '%s'\n", argv[2]);
    return(1);
  }
  fwrite(vm.vmMemory, vm.programCounter * 2, 1, fd);
  fclose(fd);
  return(0);
}
