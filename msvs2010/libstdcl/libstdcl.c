// libstdcl.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "libstdcl.h"


// This is an example of an exported variable
LIBSTDCL_API int nlibstdcl=0;

// This is an example of an exported function.
LIBSTDCL_API int fnlibstdcl(void)
{
	return 42;
}

/*
// This is the constructor of a class that has been exported.
// see libstdcl.h for the class definition
Clibstdcl::Clibstdcl()
{
	return;
}
*/