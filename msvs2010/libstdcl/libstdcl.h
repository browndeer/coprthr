// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBSTDCL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBSTDCL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBSTDCL_EXPORTS
#define LIBSTDCL_API __declspec(dllexport)
#else
#define LIBSTDCL_API __declspec(dllimport)
#endif

/*
// This class is exported from the libstdcl.dll
class LIBSTDCL_API Clibstdcl {
public:
	Clibstdcl(void);
	// TODO: add your methods here.
};
*/

extern LIBSTDCL_API int nlibstdcl;

LIBSTDCL_API int fnlibstdcl(void);
