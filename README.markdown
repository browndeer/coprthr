% Release Notes for the COPRTHR SDK version 1.5.0 (Marathon)
% Copyright (c) 2013 Brown Deer Technology, LLC
% *Verbatim copying and distribution of this entire document is
  permitted in any medium, provided this notice is preserved.*

------------------------------------------------------------------------

The CO-PRocessing THReads (COPRTHR) SDK provides 
libraries and tools for application developers targeting multi-core and 
many-core parallel co-processing computer platforms.

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


Revised 9 June 2013 by DAR

