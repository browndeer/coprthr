
#ifndef _UTIL
#define _UTIL

#include <stdio.h>

#ifdef _WIN64
#define ERROR CLERROR
#endif

#ifdef STDCL_DEBUG

#define DEBUG(f,l,msg,...)  \
	fprintf(stderr,"stdcl: debug: %s(%d): " msg "\n",f,l,##__VA_ARGS__); \
	fflush(stderr);

#else

#define DEBUG(f,l,msg,...)  do {} while(0); 

#endif


//#if STDCL_WARN || STDCL_DEBUG
#ifdef STDCL_DEBUG

#define WARN(f,l,msg)  \
	fprintf(stderr,"stdcl: warning: %s(%d): " msg "\n",f,l); 

#define ERROR(f,l,msg) \
	fprintf(stderr,"stdcl: error: %s(%d): " msg "\n",f,l); 
	
#else

#define WARN(f,l,msg) do {} while(0); 
#define ERROR(f,l,msg) do {} while(0); 

#endif


#endif
