##############
## Makefile ##
##############

include ../Makefile.inc

CFLAGS=-g -O3 -c -I ./ -I ../include -I ../include/util  $(ELF_INC) $(UNWIND_INC) #$(ZLIB_INC)
CXXFLAGS=$(CFLAGS)

LFLAGS=-O3 -g -lm $(ELF_LIB) $(UNWIND_LIB) #$(ZLIB_LIB)

SRC_DIR=../src
UTIL_DIR=../src/util

all: test

.cpp.o: 
	$(CXX) $(CXXFLAGS) $<  -o $@

test: StackMap ElfData


StackMap: $(UTIL_DIR)/StackMap.o StackMapTest.o
	$(CXX) $(LFLAGS) $(UTIL_DIR)/StackMap.o StackMapTest.o -o $@
	
ElfData: $(UTIL_DIR)/util.o $(UTIL_DIR)/ElfData.o ElfDataTest.o
	$(CXX) $(LFLAGS) $^ -o $@


clean::
	rm -f *~
	rm -f *.o
	rm -f StackMap ElfData


