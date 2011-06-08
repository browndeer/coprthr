// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

void  _libstdcl_init();
void  _libstdcl_fini();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		_libstdcl_init();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		_libstdcl_fini();
		break;
	}
	return TRUE;
}

