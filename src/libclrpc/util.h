/* util.h
 *
 * Copyright (c) 2009-2012 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifdef _WIN64
#include "fix_windows.h"
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdio.h>

#ifdef _WIN64
#define ERROR CLERROR
#endif

#ifdef _WIN64
#define ERROR CLERROR
#endif


#define DEBUG(f,l,msg,...) xclreport( XCL_DEBUG msg,##__VA_ARGS__)
#define DEBUG2(msg,...) xclreport( XCL_DEBUG msg,##__VA_ARGS__)


#define WARN(f,l,msg,...) xclreport( XCL_WARNING msg,##__VA_ARGS__)
#define WARN2(msg,...) xclreport( XCL_WARNING msg,##__VA_ARGS__)


#define ERROR(f,l,msg,...) xclreport( XCL_ERR msg,##__VA_ARGS__)
#define ERROR2(msg,...) xclreport( XCL_ERR msg,##__VA_ARGS__)


#define fprintb(fp,buf,len) do { \
	char* p = buf; size_t n = len; \
	while(n>0) { \
	if ((int)*p > 31) fprintf(fp,"%c",*p); \
	else fprintf(fp,"/%d",(int)*p); \
	++p; --n; \
	} \
	} while(0)


/*
 * New reporting utility, will alias later to replace DEGUG/WARN/ERROR -DAR
 */

#ifndef XCL_REPORT_DEFAULT_LEVEL
#define XCL_REPORT_DEFAULT_LEVEL 7
#endif

#define XCL_REPORT_BUFFER_SIZE 512

/* these reporting levels mirror those in the BSD kernel - DAR */

#define XCL_EMERG		"<0>" /* system is unusable */
#define XCL_ALERT		"<1>" /* action must be taken immediately */
#define XCL_CRIT		"<2>" /* critical conditions */
#define XCL_ERR		"<3>" /* error conditions */ 
#define XCL_WARNING	"<4>" /* warning conditions */
#define XCL_NOTICE	"<5"> /* normal but significant condition */
#define XCL_INFO		"<6>" /* informational */
#define XCL_DEBUG		"<7>" /* debug-level messages */

extern char _xcl_report_buffer[XCL_REPORT_BUFFER_SIZE];

void _xclreport(char*,int);

#ifdef XCL_REPORT

#define xclreport(msg,...)  do { \
	snprintf(_xcl_report_buffer,XCL_REPORT_BUFFER_SIZE,msg,##__VA_ARGS__); \
	_xclreport(__FILE__,__LINE__); \
   } while(0)

#else

#define xclreport(msg,...)  do {} while(0)

#endif



#endif
