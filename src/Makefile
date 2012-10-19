##############
## Makefile ##
##############

include ../Makefile.inc
 

DEFINE=

UTIL_DIR=util

WMTOOLS_BIN_DIR=../bin/
WMTRACE_LIB_DIR=../lib/
LIBNAME=WMTrace.so
FULLLIBNAME=$(WMTRACE_LIB_DIR)$(LIBNAME)

TESTS=tests



CFLAGS=-g -O3 -fPIC -shared -c -I ./ -fbounds-check $(DEFINE)  $(ELF_INC) $(UNWIND_INC) $(DYNA_INC) $(SILO_INC) #$(ZLIB_INC)
SERIALCFLAGS=$(CFLAGS) -D NO_MPI=1
CXXFLAGS=$(CFLAGS)

LFLAGS=-O3 -shared -fPIC -ldl -g -fbounds-check  -lrt

BLFLAGS=-O3  -g 

HMLFLAGS=-O3  -g

#Define libs for each app
WMAnalysisCPP_LIBS=$(LIB)m $(LIB)z $(ELF_LIB)
WMTraceCPP_LIBS=$(WMAnalysisCPP_LIBS) $(UNWIND_LIB) $(DYNA_LIB)
WMModel_LIBS=$(LIB)m $(LIB)z
WMHeatMap_LIBS=$(WMAnalysisCPP_LIBS) $(SILO_LIB)



.PHONY: clean cleaner WMAnalysisSerialBuild WMAnalysisSerial SERIALENV .FORCE


all: WMTrace WMAnalysis WMHeatMap

.cpp.o: 
	$(CXX) $(CXXFLAGS) $<  -o $@
	
Util.o: .FORCE
    
.c.o:
	$(CC) $(CFLAGS) $< -o $@
     
	

WMTraceCPP_OBJS=timers.o $(UTIL_DIR)/ElfData.o $(UTIL_DIR)/Util.o $(UTIL_DIR)/ConsumptionGraph.o $(UTIL_DIR)/ConsumptionTracker.o $(UTIL_DIR)/FunctionObj.o $(UTIL_DIR)/FunctionMap.o $(UTIL_DIR)/StackProcessingMap.o $(UTIL_DIR)/TraceReader.o WMAnalysis.o $(UTIL_DIR)/Compress.o $(UTIL_DIR)/Decompress.o $(UTIL_DIR)/FrameData.o $(UTIL_DIR)/VirtualMemoryData.o $(UTIL_DIR)/TraceBuffer.o $(UTIL_DIR)/CallStackTraversal.o $(UTIL_DIR)/StackMap.o MemoryFunction.o WMTrace.o 

WMTrace: $(WMTraceCPP_OBJS) $(WMTRACE_LIB_DIR)
	$(CXX) $(LFLAGS) $(WMTraceCPP_OBJS)  -Wl,-soname,$(FULLLIBNAME).$(VERSION) -o $(FULLLIBNAME).$(VERSION) $(WMTraceCPP_LIBS)
	rm -rf $(FULLLIBNAME)
	ln -s $(FULLLIBNAME).$(VERSION) $(FULLLIBNAME)

Reader_OBJS=$(UTIL_DIR)/Util.o $(UTIL_DIR)/FrameData.o $(UTIL_DIR)/Decompress.o $(UTIL_DIR)/ElfData.o $(UTIL_DIR)/ConsumptionGraph.o $(UTIL_DIR)/ConsumptionTracker.o  $(UTIL_DIR)/FunctionObj.o $(UTIL_DIR)/FunctionMap.o $(UTIL_DIR)/StackProcessingMap.o $(UTIL_DIR)/TraceReader.o

WMAnalysisCPP_OBJS= $(Reader_OBJS) WMAnalysis.o

WMAnalysis: $(WMAnalysisCPP_OBJS)  ParallelAnalysis.o $(WMTOOLS_BIN_DIR)
	$(CXX) $(BLFLAGS) $(WMAnalysisCPP_OBJS)  ParallelAnalysis.o -o $(WMTOOLS_BIN_DIR)$@ $(WMAnalysisCPP_LIBS)
	

WMAnalysisSerial_OBJS= $(Reader_OBJS) WMAnalysis.o SerialAnalysis.o

SERIALENV::
	$(eval CXXFLAGS=$(SERIALCFLAGS))
	$(eval CC=$(SERIALCC))
	$(eval CXX=$(SERIALCXX))


WMAnalysisSerial: SERIALENV $(WMAnalysisSerial_OBJS) $(WMTOOLS_BIN_DIR)
	$(CXX) $(BLFLAGS) $(WMAnalysisSerial_OBJS) -o $(WMTOOLS_BIN_DIR)$@ $(WMAnalysisCPP_LIBS)
	
WMHeatMap_OBJS=$(UTIL_DIR)/SiloHMWriter.o $(Reader_OBJS) WMHeatMap.o

WMHeatMap: $(WMHeatMap_OBJS) $(WMTOOLS_BIN_DIR)
	$(CXX) $(HMLFLAGS) $(WMHeatMap_OBJS) -o $(WMTOOLS_BIN_DIR)$@ $(WMHeatMap_LIBS)

WMModel_OBJS=$(Reader_OBJS)  $(UTIL_DIR)/CallStackMapper.o $(UTIL_DIR)/ConsumptionMap.o  WMModel.o

WMModel: SERIALENV $(WMModel_OBJS) $(WMTOOLS_BIN_DIR)
	$(CXX) $(BLFLAGS) $(WMModel_OBJS) -o $(WMTOOLS_BIN_DIR)$@ $(WMAnalysisCPP_LIBS)
	
$(WMTRACE_LIB_DIR): 
	mkdir -p $(WMTRACE_LIB_DIR)

 $(WMTOOLS_BIN_DIR): 
	mkdir -p $(WMTOOLS_BIN_DIR)

clean::
	rm -f *~
	rm -f *.o
	rm -f util/*.o
	
cleaner::
	rm -f *.so
	rm -f *.so.*
	rm -f *~
	rm -f *.o
	rm -f $(WMTRACE_LIB_DIR)/*
	rm -f util/*.o
	
.FORCE:
