# WMTools #

Warwick Memory Tools - Suite of tools for parallel application memory consumption analysis

The project consists of a shard library for tracing the application in question, and a set of applications to analyse the resulting output. 
The tracing library is designed for MPI applications based on standard POSIX memory management (C, C++ and Fortran), and requires no application recompilation.

## Contact ##

For any more information regarding the project or usage please use the contact on the [github project page](https://github.com/Warwick-PCAV/WMTools).

# INSTALL #


## Dependencies ##

Different tools within the WMTools suite depend on different third party libraries

* WMTrace
  * libz - The ZLib compression library
  * libunwind - The call stack traversal library
  * libelf - The binary header reader
* WMAnalysis & WMModel
  * libz - The ZLib compression library
* WMHeatMap
  * libz - The ZLib compression library
  * libsilo - The Silo file format generator (Depends on HDF5)

## Makefile.inc ##

The Makefile.inc file contains the paths to all of the third party libraries in addition to some additional configuration options. This includes the prefix path to install WMTools after build.

The simply run:
`make clean all`

This should produce:
* lib/
  * WMTrace.so
* bin/
  * WMAnalysis
  * WMHeatMap

# Running #

Most of the tools are developed with a dependency on MPI, and as such require a suitable execution environment.

## Tracing with WMTrace ##

Attaching WMTrace to an application is very simple, you simply export the library in the LD_PRELOAD envionment variable, then run the code as normal:

`mpirun -np <x> ./<application> <application arguments>`

The library will only attach on MPI_Init, so additional codes will not be affected by it's presence, but to limit it to only the desired application then include it on the MPI run command.

`mpirun -np <x> -x LD_PRELOAD=<path to lib folder>/WMTrace.so ./<application> <application arguments>`

## Runtime Configurations ##

WMTrace has a number of different operating modes. These are specified in a configuration file at runtime.
The library reads the hidden file .WMToolsConfig from either the current working directory, or if not found then the users home directory (allowing for both global and local configurations).

This file can contain the following options (each specified on a new line):
* --WMTOOLSCOMPLEX 

  This enables call stack tracing, which can incur a significant performance hit in some circumstances.
* --WMTOOLSPOSTPROCESS

  This option enables automated post processing at the end of execution. This uses all the available processes of the job to post process all of the trace files, to provide instantanious memory consumption HWM data.

## Output ##

WMTrace outputs a trace file per process in a uniquely named folder per run. 

`WMTrace0001`

File names ahve the extension .z to indicate they are zlib compressed files.

`WMTrace0001/trace-0.z`

# WMAnalysis #

WMAnalysis has server different modes of operation. As a serial application it can be used to replay a single trace file for memory consumption statistics.

To analyse a specific trace file:

`mpirun -np 1 WMAnalysis WMTrace0001/trace-0.z`

WMAnalysis can also be run in parallel to analyse a full jobs worth of files. Note the job size for postprocession does not have to be the same as that of the original job, it will distribute the workload over all available processes until finished.

To analyse a specific folder of trace files:

`mpirun -np <x> WMAnalysis WMTrace0001`

## Serial Analysis ##

For running the analysis outside of the MPI environment there is a serial version of WMAnalysis with no dependency on the MPI library.
To build simply run:
`cd src; make clean WMAnalysisSerial;`

This builds a `bin/WMAnalysisSerial` binary.

This binary is also built with the standard make all command:

`make clean all`

The binary can be run as before:

`WMAnalysisSerial WMTrace0001/trace-0.z`


## Command line arguments ##

WMAnalysis takes a few optional command line arguments to generate additional information.

* `--graph`

  This option will produce gnuplot graph scipts for every trace file provided.
  This requies two passes of the trace file and so can be quite slow.
  The file generated will be named with a .graph extension, and when run will generate a png.
* `--functions`

  This option produces an orderd list of functions consumption at point of high water mark - ordered by size.
  The file generated will be named with a .functions extension, but will be a text file.

# WMHeatMap #

WMHeatMap can produce a VisIt visualisation of memory consumption over time, where ranks are grouped by node, to indicate distribution.

Again WMHeatMap can be run in either serial or parallel mode, though parallel mode is encouraged due to the volume of processing required. Again the job size for post processing does not need to match that of the original execution.

To run simply provide the folder containing the trace files:

`mpirun -np <x> WMHeatMap WMTrace0001`

## Command Line Arguments ##

WMHeatMap can be configured with a number of simple command line arguments.

* `-s=<x>`

  Specify the number of sample points for the trace, defaults to 1000.
* `-o=<output dir>`

  Specify an output directory. Defaults to a uniquely named WMHeatMap0001 folder.
 
## VisIt Output ##


To view the output in VisIt first open VisIt. 

To open from the output directory WMHeatMap.hdf50001 open:

`WMHeatMap.hdf50001/WMHeatMap.visit`

Then click Add:

* Mesh -> quadmesh
* Label -> Rank
* Pseudocolor -> Memory
* Subsets -> domains

Configure Pseudocolor - Memory:
* Limits -> Minimum 0 & Maximum 1
  
Configure Subsets - domains:
* Subset colors -> Single
* Options -> Wireframe
* Point/Line Style -> Line Width 5

Apply settings then Draw.
  
# WMModel #
 
 WMModel is still slightly experimental and is not included in this current build.
 
 Watch this space for more updates. 