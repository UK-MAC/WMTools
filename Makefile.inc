##############
## Makefile ##
##############

## Version ##

VERSION=1.3
MPI_VERSION=$(HPC_MPI)
#intelmpi-4.1.1
COMPILER_VERSION=$(HPC_COMPILER)
#intel-13.1
## Prefix ##


LIB_PREFIX=/opt/lib
CODES_PREFIX=/opt/codes
PREFIX=$(LIB_PREFIX)/WMTools/$(COMPILER_VERSION)/$(MPI_VERSION)/$(VERSION)


## Groups ##

GROUP=

## Variables ##


#Define Parallel compilers
MPICC=mpiicc
MPICXX=mpiicpc

#Define Serial Compilers
SERIALCC=icc
SERIALCXX=icpc

# Default to Parallel compilers
CC=$(MPICC)
CXX=$(MPICXX)
 
LIB_DIR=-L
LIB=-l
INC_DIR=-I

## Directories ##

#Unwind
UNWIND_VERSION=1.0.1
UNWIND_DIR=$(LIB_PREFIX)/unwind/$(COMPILER_VERSION)/$(UNWIND_VERSION)
UNWIND_INC=$(INC_DIR)$(UNWIND_DIR)/include
UNWIND_LIB=$(LIB_DIR)$(UNWIND_DIR)/lib $(LIB)unwind-x86_64 

#ZLib
ZLIB_DIR= #
ZLIB_INC= #$(INC_DIR)$(ZLIB_DIR)/include
ZLIB_LIB=$(LIB)z

#Elf
ELF_VERSION=0.8.9
ELF_DIR=$(LIB_PREFIX)/libelf/$(COMPILER_VERSION)/$(ELF_VERSION)
ELF_INC=$(INC_DIR)$(ELF_DIR)/include
ELF_LIB=$(LIB_DIR)$(ELF_DIR)/lib $(LIB)elf



#HDF5
HDF5_DIR=$(CODES_PREFIX)/visit/hdf5/1.8.7/linux-x86_64_gcc-4.4
HDF5_INC=$(INC_DIR)$(HDF5_DIR)/include
HDF5_LIB=$(LIB_DIR)$(HDF5_DIR)/lib $(LIB)hdf5

#Silo - Depends on HDF5
SILO_DIR=$(CODES_PREFIX)/visit/silo/4.8/linux-x86_64_gcc-4.4
SILO_INC=$(INC_DIR)$(SILO_DIR)/include $(HDF5_INC)
SILO_LIB=$(LIB_DIR)$(SILO_DIR)/lib $(LIB)siloh5 $(HDF5_LIB) 


