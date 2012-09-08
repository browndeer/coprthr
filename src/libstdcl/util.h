
#ifndef _UTIL
#define _UTIL

#ifdef _WIN64
#include "fix_windows.h"
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdio.h>

//#ifdef _WIN64
//#define ERROR CLERROR
//#endif
//
//#ifdef _WIN64
//#define ERROR CLERROR
//#endif

#define __set_oclerrno( n )  if (n) oclerrno = (n)
#define __set_clerrno( n )  if (n) clerrno = (n)

#endif
