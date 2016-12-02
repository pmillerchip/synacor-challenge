CPP=g++
CPPFLAGS=-g -std=c++11

all: vm synasm

vm: vm.o SynacorVM.o
	$(CPP) $(CPPFLAGS) vm.o SynacorVM.o -o $@

synasm: synasm.o SynacorVM.o
	$(CPP) $(CPPFLAGS) synasm.o SynacorVM.o -o $@

clean:
	rm -f vm synasm *.o

.SUFFIXES: .cpp .o

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $<

