
    COPRTHR version 1.4 (echo) Release Notes

Copyright © 2011-2012 Brown Deer Technology, LLC

Verbatim copying and distribution of this entire document is
permitted in any medium, provided this notice is preserved.

------------------------------------------------------------------------
The CO-PRocessing THReads (COPRTHR) SDK provides several OpenCL? related
libraries and tools for developers targeting many-core compute
technology and hybrid CPU/GPU/APU computing architectures.
------------------------------------------------------------------------

  * 1 New in version 1.4 (echo) release 
  * 2 Support and Requirements 
  * 3 Important Notes 
  * 4 Frequently Asked Questions 
  * 5 More Information 

------------------------------------------------------------------------


1 New In Version 1.4 (echo release)

 * Offline OpenCL compiler supporting a real compilation model (clcc,clld,clnm)

   - Supports cross-platform multi-device compilation using an open
     ELF-based specification (CL-ELF)

   - Compile multiple OpenCL kernel files for multiple devices into a
     single linkable ELF object file

   - Redesigned STDCL dynamic loader makes embedded kernels available
     with a single call

 * Redesigned OpenCL implementation for x86_64 (libcoprthr)

   - Eliminates complex dependencies, requires only GCC for compilation

   - Light-weight, high-performance design faster than vendor SDKs on
     some HPC benchmarksa

 * OpenCL implementation for multicore ARM processors (libcoprthr)

   - Includes NEON SIMD instructions with GCC vectorization

   - Includes full COPRTHR port to Linux Angstrom for embedded processors

 * Preview of replacement for Khronos ICD loader (libocl)

   - Open specification with flexible system configuration options

   - Backward compatibility with libOpenCL.so

 * Improved build system

 * Updated examples and documentation

 * Supports Linux, FreeBSD and Windows 7 with MSVS 2010

 * STDCL is tested against AMD SDK v2.4/v2.5, Nvidia CUDA 4.0, and
   Intel OCL SDK v1.5

------------------------------------------------------------------------


2 Support and Requirements

Support continues for most modern Linux distributions including
CentOS 5/6, and OpenSuSE 11.3. Full support for FreeBSD-8 also
continues. The basic functionality provided by the STDCL interface is
provided for Windows 7 using MSVS 2010. Specific feature support by
operating system is shown in the table below.

COPRTHR Feature 	Description 	         Linux 	FreeBSD 	Windows
libstdcl 	      STDCL interface         x        x        x
libcoprthr        OpenCL implementation   x 	      x 	
libocl 	         OpenCL platform loader  x 	      x 	
clcc,clld,clnm 	Offline OpenCL compiler	x 		
cltrace 	         Tracing tool 	         x        x 	

This release is compatible with OpenCL implementations provided by AMD
APP v2.4/2.5, Nvidia CUDA-4 and Intel OCL SDK v1.5. In addition, an
open-source OpenCL run-time implementation for x86_64 and ARM multi-core
processors is provided as part of the COPRTHR SDK, which may be used on
platforms for which no vendor support is available. The COPRTHR OpenCL
implementation may also be of interest since it exhibits better
performance than vendor implementations on some real-world benchmarks.

This release supports x86_64 CPUs from AMD and Intel as well as GPUs
from AMD and Nvidia, and has been tested successfully on the following
graphics cards: AMD Radeon HD 5870, 5970, 6970, AMD FirePro V8800,
Nvidia Tesla S1070, C2050, and C2070. AMD A-series APUs are also
supported. Support has been extended to include multicore ARM processors
running Linux Angstrom.

The table below provides a comprehensive matrix of optional and required 
packages matched to a specific platform and feature set. 

Please take note that libelf 1.x branch found on most Linux distributions 
is not a valid substitute for libelf-0.8.13 since they lack the required 
features and exhibit undocumented broken behavior.

*Linux CentOS 5/6, OpenSuSE 11.3*

(O) AMD APP v2.4/2.5 (developer.amd.com/sdks/AMDAPPSDK/downloads)

(O) Nvidia CUDA 4 (developer.nvidia.com/cuda-toolkit-40)

(O) Intel OCL 1.5 (software.intel.com/en-us/articles/vcsource-tools-opencl-sdk)

(R) libelf 0.8.13 (www.mr511.de/software/libelf-0.8.13.tar.gz)

(R) libssl (www.openssl.org)

*FreeBSD-8*

(R) libelf 0.8.13 (www.mr511.de/software/libelf-0.8.13.tar.gz)

(R) libssl (www.openssl.org)

*Windows 7*

(O) AMD APP v2.4/2.5 (developer.amd.com/sdks/AMDAPPSDK/downloads)

(O) Nvidia CUDA 4 (developer.nvidia.com/cuda-toolkit-40)

(O) Intel OCL 1.5 (software.intel.com/en-us/articles/vcsource-tools-opencl-sdk)

O=Optional, R=Required

------------------------------------------------------------------------


3 Important Notes

  * On Windows 7 platforms the function init_stdcl() must be called to
    initialize the STDCL API. This call is unnecessary for Linux and
    FreeBSD, but there is no harm in including it since it will default
    to an empty macro.

  * The libraries libstdcl, libcoprthr and libocl are provided with
    debug versions libstdcl_d, libcoprthr_d and libocl_d,
    respectively. Linking against these libraries can be very useful for
    debugging as well as understanding how each library operates.

  * The platform that you select may not support certain types of
    devices, e.g., the Nvidia SDK does not suport CPUs. This may cause
    problems with some configurations. The easiest solution is to
    disable the context for which no devices are supported, e.g.,
    setting the environment variable STDCPU=0 will cause the runtime to
    ignore the CPU context. For vendor implementations that incur a
    non-negligible cost for creating contexts, this can also improve the
    start-up performance of some applications.

  * The version of the boost library (1.47.0) used here for development
    unfortunately has a very minor bug that prevents it from working
    with MSVS 2010. A fix to the boost package is provided in this
    release. Just follow the instructions under the
    msvs2010/boost_1_47_0-multi_array-iterator-fix/ directory. (All you
    need to do is copy over a replacement for iterator.hpp.)

  * Configure script options have been revised to provide better
    automatic platform detection and better user control over platform
    selection. Additionally, the libelf dependency is more carefully
    checked to avoid a previously common installation error.

  * A new platform selection policy is used that allows a
    priority-ordered comma separated list of platform names that are
    used to determine which platform should be used for default
    contexts. The new policy is more robust and will only fail if no
    platform is found capable of supporting at least one context device
    type.

  * The operations CL_MCTL_SET_USERFLAGS and CL_MCTL_CLR_USERFLAGS have
    been added to the clmctl() call to allow setting vendor/device
    specific additional cl_mem_flags not directly controlled by STDCL.
    These operations cna only be performed on a detached allocation and
    will be applied by the calls clmattach() or clmrealloc(). The
    behavior of the specified flags is not checked therefore these
    operations should be used with caution.

  * Fortran bindings for STDCL require GCC 4.4 or later since they
    utilize the ISO C binding support.

  * Optimial performance using the COPRTHR OCL implementation
    (libcoprthr) requires GCC 4.6 and the 'no-template' model. This can
    be configured using --enable-libcoprthr --with-libcoprthr-cc=gcc46
    --with-libcoprthr-cxx=g++46 --with-libcoprthr-model=no-template .

  * Some example kernels include the stdcl.h header. Including this
    header for OpenCL kernels allows the use of alternate syntax that
    avoids breaking C. Programmers are encouraged to begin this practice
    now. A future release will formalize syntax corrections to OpenCL to
    produce a C compliant language for programming thread functions
    (kernels). The alternate syntax will be completely backward
    compatable with OpenCL.

------------------------------------------------------------------------


4 Frequently Asked Questions

Below are answers to frequently asked questions regarding COPRTHR SDK
and STDCL.

Does STDCL require the BDT OpenCL run-time?

    No. The basic installation of libstdcl.so will work with any
    compliant OpenCL installation including the latest implementations
    from AMD, Nvidia and Intel.

Will using STDCL reduce performance or limit access to OpenCL functionality?

    No. STDCL is implemented as a very light-weight interface, does not
    restrict access to direct OpenCL and fully supports asynchronous
    operations across multiple devices.

Are STDCL calls simply wrappers for OpenCL calls?

    No. There is a bit more to the interface than wrapping OpenCL calls.
    For the curious, take a look at the source code.

------------------------------------------------------------------------


5 More Information

Additional information including installation instructions and examples
may be found in The COPRTHR Primer revision 1.4 along with more
detailed documentation and examples.

------------------------------------------------------------------------
revised 3 March 2012 by DAR
