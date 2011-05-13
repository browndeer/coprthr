
#ifndef _FIX_WINDOWS_H
#define _FIX_WINDOWS_H

#include <windows.h>
#include <malloc.h>
#include <direct.h>
#include <process.h>

#ifdef LIBSTDCL_EXPORTS
#define LIBSTDCL_API __declspec(dllexport)
#else
#define LIBSTDCL_API __declspec(dllimport)
#endif

#define __inline__ __inline

#define getpagesize() 4096

#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define snprintf _snprintf
#define getpid _getpid
#define strtok_r strtok_s

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

//#define STDCL_DEBUG

#endif
