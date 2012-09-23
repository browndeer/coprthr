/* printcl.h
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

#ifndef _PRINTCL
#define _PRINTCL

#ifndef ENABLE_SILENT
#define ENABLE_PRINTCL
#endif

#if MAX_CLMESG_LEVEL == 0
#define __MAX_CLMESG_LEVEL_CHR '0'
#elif MAX_CLMESG_LEVEL == 1
#define __MAX_CLMESG_LEVEL_CHR '1'
#elif MAX_CLMESG_LEVEL == 2
#define __MAX_CLMESG_LEVEL_CHR '2'
#elif MAX_CLMESG_LEVEL == 3
#define __MAX_CLMESG_LEVEL_CHR '3'
#elif MAX_CLMESG_LEVEL == 4
#define __MAX_CLMESG_LEVEL_CHR '4'
#elif MAX_CLMESG_LEVEL == 5
#define __MAX_CLMESG_LEVEL_CHR '5'
#elif MAX_CLMESG_LEVEL == 6
#define __MAX_CLMESG_LEVEL_CHR '6'
#else
#define __MAX_CLMESG_LEVEL_CHR '7'
#endif


#ifdef _WIN64
#include "fix_windows.h"
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef _WIN64
#define ERROR CLERROR
#endif

#ifdef _WIN64
#define ERROR CLERROR
#endif


#define DEBUG(f,l,msg,...) printcl( CL_DEBUG msg,##__VA_ARGS__)
#define DEBUG2(msg,...) printcl( CL_DEBUG msg,##__VA_ARGS__)


#define WARN(f,l,msg,...) printcl( CL_WARNING msg,##__VA_ARGS__)
#define WARN2(msg,...) printcl( CL_WARNING msg,##__VA_ARGS__)


#define ERROR(f,l,msg,...) printcl( CL_ERR msg,##__VA_ARGS__)
#define ERROR2(msg,...) printcl( CL_ERR msg,##__VA_ARGS__)


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

#ifndef DEFAULT_CLMESG_LEVEL
#define DEFAULT_CLMESG_LEVEL 6
#endif

/* these reporting levels mirror those in the BSD kernel - DAR */

#define CL_EMERG		"<0>" /* system is unusable */
#define CL_ALERT		"<1>" /* action must be taken immediately */
#define CL_CRIT		"<2>" /* critical conditions */
#define CL_ERR			"<3>" /* error conditions */ 
#define CL_WARNING	"<4>" /* warning conditions */
#define CL_NOTICE		"<5"> /* normal but significant condition */
#define CL_INFO		"<6>" /* informational */
#define CL_DEBUG		"<7>" /* debug-level messages */

#define PRINTCL_TAG	"clmesg"

static void _printcl(char*,int, const char*, const char*, ...);

#ifdef ENABLE_PRINTCL

#define printcl(msg,...)  do { \
	if (msg[1] <= __MAX_CLMESG_LEVEL_CHR) \
	_printcl(__FILE__,__LINE__,PRINTCL_TAG,msg,##__VA_ARGS__); \
   } while(0)

#else

#define printcl(msg,...)  do {} while(0)

#endif


static int __printcl_level = -1;

#ifndef PRINTCL_BUFFER_SIZE
#define PRINTCL_BUFFER_SIZE 1024
#endif

static void 
_printcl(char* file, int line, const char* tag, const char* msg, ... )
{
	char* msg_type[] = { 
		"EMERGENCY", "ALERT", "CRITICAL", "ERROR", 
		"WARNING", "notice", "info", "debug"
	};

	va_list ap;
	va_start( ap, msg );

	char buf[PRINTCL_BUFFER_SIZE];

	if (__printcl_level < 0) {
		char* envset = getenv("COPRTHR_CLMESG_LEVEL");
		__printcl_level = (envset)? atoi(envset) : DEFAULT_CLMESG_LEVEL;
//		fprintf(stderr,"printcl: default level set to %d\n", __printcl_level);
	}

	int level = 7;
	char* pbuf = buf;

	vsnprintf( buf, PRINTCL_BUFFER_SIZE, msg, ap );

	if ( buf[0] == '<' && buf[1] != '\0' && buf[2] == '>') {

		level = buf[1] - '0';

		if (level<0 || level>7) level = 7;
	
		pbuf += 3;

	}

	if (level <= __printcl_level) {

		fprintf(stderr, "[%d] %s %s: %s(%d): %s\n", 
			getpid(), tag, msg_type[level], file,line, pbuf );

		fflush(stderr);

	}

}


#endif
