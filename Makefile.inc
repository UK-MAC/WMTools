##############
## Makefile ##
##############

## Version ##

VERSION=1.2.3
MPI_VERSION=ompi-1.6.2

## Prefix ##

PREFIX=/opt/lib/gnu/wmtools/$(MPI_VERSION)/$(VERSION)


## Groups ##

GROUP=

## Variables ##


#Define Parallel compilers
MPICC=mpicc
MPICXX=mpicxx

#Define Serial Compilers
SERIALCC=gcc
SERIALCXX=g++

# Default to Parallel compilers
CC=$(MPICC)
CXX=$(MPICXX)
 
LIB_DIR=-L
LIB=-l
INC_DIR=-I

## Directories ##

#Unwind
UNWIND_DIR=/opt/lib/gnu/unwind/1.0.1
UNWIND_INC=$(INC_DIR)$(UNWIND_DIR)/include
UNWIND_LIB=$(LIB_DIR)$(UNWIND_DIR)/lib $(LIB)unwind-x86_64

#ZLib
ZLIB_DIR= #
ZLIB_INC= #$(INC_DIR)$(ZLIB_DIR)/include
ZLIB_LIB=$(LIB)z

#Elf
ELF_DIR=/opt/lib/gnu/elf/0.8.9
ELF_INC=$(INC_DIR)$(ELF_DIR)/include
ELF_LIB=$(LIB_DIR)$(ELF_DIR)/lib $(LIB)elf

#DWARF
DWARF_DIR=#/opt/lib/gnu/dwarf
DWARF_LIB=#$(LIB_DIR)$(DWARF_DIR)/lib $(LIB)dwarf

#Stackwalker API
#DYNA_DIR=
#DYNA_INC=$(INC_DIR)$(DYNA_DIR)/include
#DYNA_LIB=$(LIB_DIR)$(DYNA_DIR)/x86_64-unknown-linux2.4/lib $(LIB)stackwalk $(LIB)symtabAPI $(LIB)common $(LIB)iberty_pic  $(DWARF_LIB)


#HDF5
HDF5_DIR=/opt/lib/gnu/hdf5/parallel/$(MPI_VERSION)/1.8.8
HDF5_INC=$(INC_DIR)$(HDF5_DIR)/include
HDF5_LIB=$(LIB_DIR)$(HDF5_DIR)/lib $(LIB)hdf5

#Silo - Depends on HDF5
SILO_DIR=/opt/lib/gnu/silo/$(MPI_VERSION)/4.8
SILO_INC=$(INC_DIR)$(SILO_DIR)/include $(HDF5_INC)
SILO_LIB=$(LIB_DIR)$(SILO_DIR)/lib $(LIB)siloh5 $(HDF5_LIB)


