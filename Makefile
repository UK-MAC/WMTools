##############
## Makefile ##
##############

include Makefile.inc

SRC_DIR=src
TEST_DIR=tests

all :
	$(MAKE) -C $(SRC_DIR) all
	$(MAKE) -C $(SRC_DIR) clean
	$(MAKE) -C $(SRC_DIR) WMAnalysisSerial WMModel

WMTrace : 
	$(MAKE) -C $(SRC_DIR) WMTrace

WMAnalysis : 
	$(MAKE) -C $(SRC_DIR) WMAnalysis

WMHeatMap : 
	$(MAKE) -C $(SRC_DIR) WMHeatMap

WMComparisons : 
	$(MAKE) -C $(SRC_DIR) WMComparisons
	
clean :
	$(MAKE) -C $(SRC_DIR) clean
	$(MAKE) -C $(TEST_DIR) clean
	rm -rf bin/*
	rm -rf lib/*
	rm -rf tests/MemSpeed

cleaner : clean
	$(MAKE) -C $(SRC_DIR) cleaner
	
test :	
	$(MAKE) -C $(TEST_DIR) test
	
documentation :
	doxygen Doxyfile

install :
	mkdir -p  $(PREFIX)
	cp -r bin $(PREFIX)
	cp -r lib $(PREFIX)
	cp -r docs $(PREFIX)
	#chgrp -R $(GROUP) $(PREFIX)
	chmod a+rX -R $(PREFIX)
