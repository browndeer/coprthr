/* util.h 
 *
 * Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* DAR */

#ifndef _UTIL
#define _UTIL

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#ifdef XCL_DEBUG

#define DEBUG(f,l,msg,...)  do { \
	fprintf(stderr, \
	"[%d.%p]xcl: debug: %s(%d): " msg "\n", \
	getpid(),pthread_self(),f,l,##__VA_ARGS__); \
	fflush(stderr); \
	} while(0)

#else

#define DEBUG(f,l,msg,...)  do {} while(0)

#endif


#if XCL_WARN || XCL_DEBUG

#define WARN(f,l,msg,...)  do { \
	fprintf(stderr,"xcl: warning: %s(%d): " msg "\n",f,l,##__VA_ARGS__); \
	fflush(stderr); \
	} while(0)

//#define ERROR(f,l,msg,...)  \
//	fprintf(stderr,"xcl: error: %s(%d): " msg "\n",f,l,##__VA_ARGS__); \
//	fflush(stderr);
	
#else

#define WARN(f,l,msg,...) do {} while(0)
//#define ERROR(f,l,msg,...) do {} while(0); 

#endif


#define ERROR(f,l,msg,...)  do { \
	fprintf(stderr,"xcl: error: %s(%d): " msg "\n",f,l,##__VA_ARGS__); \
	fflush(stderr); \
	exit(-1); \
	} while(0)
	

#define fprintb(fp,buf,len) do { \
	char* p = buf; size_t n = len; \
	while(n>0) { \
	if ((int)*p > 31) fprintf(fp,"%c",*p); \
	else fprintf(fp,"/%d",(int)*p); \
	++p; --n; \
	} \
	} while(0)


#endif
