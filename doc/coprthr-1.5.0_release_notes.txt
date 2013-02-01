% COPRTHR version 1.5 (Marathon) Release Notes
% Copyright &copy; 2013 Brown Deer Technology, LLC
% *Verbatim copying and distribution of this entire document is
  permitted in any medium, provided this notice is preserved.*

------------------------------------------------------------------------

The CO-PRocessing THReads&trade; (COPRTHR&trade;) SDK 
provides software libraries 
and tools that leverage OpenCL to support application development for 
heterogeneous parallel co-processing architectures.
The provided support includes an implementation of the STDCL&trade; API.

This software is freely distributed under the GPLv3 open-source license.

------------------------------------------------------------------------

# New In Version 1.5 (Marathon) release

* *OpenCL Remote Procedure Call (RPC) implementation (CLRPC)*

	- Stand-alone CLRPC server (clrpcd) for exporting OpenCL platforms over
	  a network connection

	- Client-side OpenCL RPC implementation (`libclrpc`)

* *New OpenCL loader (`libocl`) with advanced functionlity*

	- Backward-compatible with Khronos ICD loader

	- Supports precise platform configuration with ocl.conf files

	- Integrated CLRPC support for access to remote CLRPC servers

	- Suppors process accounting and extensible host call hooks

* *New error reporting and debugging infrastructure*

	- OpenCL and STDCL error reporting via `oclerrno` and `clerrno`

	- Run-time selection of reporting level from all tools and libraries

* *New processor support for Adapteva Epiphany/Parallella*

	- Full SDK support for the Adapteva Epiphany processor and 
	  Parallella platform

	- Includes OpenCL and STDCL support for Parallella Epiphany 
	  and ARM processors

	- Includes extensions for the Epiphany architecture

* *STDCL CONTEXT `stdnpu` for networked compute devices (preview feature)*

	- Provides a single default compute context for accessing all 
	  networked devices available via CLRPC

* *New tools and commands*

	- `cltop` for monitoring co-processing similar to UNIX top command

	- `cldebug` front-end for application debugging


------------------------------------------------------------------------


# Support and Requirements

Full SDK support is provided for Linux, Ubuntu, and FreeBSD operating systems.  Additionally, the STDCL&trade; API is supported for Windows 7. 
Supported hardware includes AMD and Nvidia GPUs, Intel and AMD x86 
multicore CPUs, ARM multicore CPUs, and Epiphany multicore processors.
The COPRTHR SDK leverages vendor OpenCL GPU implementations and also 
provides OpenCL implementions for multicore processors to provide truly 
cross-vendor/cross-device support for heterogeneous computing platforms.

Package dependencies (O=Optional, R=Required)

1. libelf-0.8.13 
   [www.mr511.de/software/libelf-0.8.13.tar.gz](http://www.mr511.de/software/libelf-0.8.13.tar.gz) 
   [Required]
2. libconfig-1.4.8 or later 
   [www.hyperrealm.com/libconfig/libconfig-1.4.9.tar.gz](www.hyperrealm.com/libconfig/libconfig-1.4.9.tar.gz)
   [Required]
3. libevent-2.0.18 or later 
   [github.com/downloads/libevent/libevent/libevent-2.0.21-stable.tar.gz](github.com/downloads/libevent/libevent/libevent-2.0.21-stable.tar.gz)
   [Required]
4. GCC 4.6 or later [Required]

Additionally, vendor support is needed for certain specific OpenCL devices

5. AMD APP 
   [developer.amd.com/sdks/AMDAPPSDK/downloads](developer.amd.com/sdks/AMDAPPSDK/downloads)
   [Optional]
6. Nvidia CUDA 4 
   [developer.nvidia.com/cuda-toolkit-40](developer.nvidia.com/cuda-toolkit-40)
   [Optional]
7. Intel OCL 1.5 
   [software.intel.com/en-us/articles/vcsource-tools-opencl-sdk](software.intel.com/en-us/articles/vcsource-tools-opencl-sdk) 
   [Optional]
8. Adapteva Epiphany SDK for Epiphany and Parallella support [Optional]

Please take note that libelf 1.x branch found on most Linux distributions 
is *not a valid substitute* for libelf-0.8.13 since they lack the required 
features and exhibit undocumented broken behavior.


------------------------------------------------------------------------


# Important Notes

* On Windows 7 platforms the function init_stdcl() must be called to
  initialize the STDCL API. This call is unnecessary for Linux and
  FreeBSD, but there is no harm in including it since it will default
  to an empty macro.

* The "debug libraries" have been eliminated with the new debug and reporting structure.

* The version of the boost library (1.47.0) used here for development
  unfortunately has a very minor bug that prevents it from working
  with MSVS 2010. A fix to the boost package is provided in this
  release. Just follow the instructions under the
  `msvs2010/boost_1_47_0-multi_array-iterator-fix/` directory. (All you
  need to do is copy over a replacement for iterator.hpp.)

* Some example kernels include the stdcl.h header. Including this
  header for OpenCL kernels allows the use of alternate syntax that
  avoids breaking C. Programmers are encouraged to begin this practice
  now. A future release will formalize syntax corrections to OpenCL to
  produce a C compliant language for programming thread functions
  (kernels). The alternate syntax will be completely backward
  compatable with OpenCL.

* When installing COPRTHR SDK on Linux Angstrom for ARM, be aware of the 
  following required changes to the standard Linux Angstrom distribution. 
  All busybox substitutes for UNIX commands that are encountered should be 
  replaced with real implementations that actually work. Specifically, GNU
  coreutils and binutils should be installed and used to provide relevant
  commands. The command /usr/bin/time should be replaced with a command that
  works properly to execute the standard tests that come with COPRTHR. The
  standard shell /bin/sh should be replaced with a real implementation of
  bash which actually works. Finally, you must also re-install bison-2.5, 
  flex-2.5.35, and m4-1.4.16 using the GNU distributions of these packages
  since the installed version of bison is broken. 

------------------------------------------------------------------------


# Frequently Asked Questions

Below are answers to frequently asked questions regarding COPRTHR SDK
and STDCL.

* Does STDCL require the BDT OpenCL run-time?

    No. The basic installation of libstdcl.so will work with any
    compliant OpenCL installation including the latest implementations
    from AMD, Nvidia and Intel.

* Will using STDCL reduce performance or limit access to OpenCL functionality?

    No. STDCL is implemented as a very light-weight interface, does not
    restrict access to direct OpenCL and fully supports asynchronous
    operations across multiple devices.

* Are STDCL calls simply wrappers for OpenCL calls?

    No. There is a bit more to the interface than wrapping OpenCL calls.
    For the curious, take a look at the source code.

------------------------------------------------------------------------


# More Information

Additional information including installation instructions and examples
may be found in The COPRTHR Primer version 1.5 along with more
detailed documentation and examples.

------------------------------------------------------------------------

Revised 1 February 2013 by DAR

