/* version.h 
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

#ifndef _VERSION_H
#define _VERSION_H


#define COPRTHR_ALPHA	1	/* @ALPHA */
#define COPRTHR_BETA		0	/* @BETA */


#define __quote(x) #x
#define __stringify(x) __quote(x)


#ifndef COPRTHR_VERSION
#define COPRTHR_VERSION 1	/* @VERSION */
#endif

#ifndef COPRTHR_REVISION
#define COPRTHR_REVISION 6 /* @REVISION */
#endif

#ifndef COPRTHR_RELEASE
#define COPRTHR_RELEASE 2 /* @RELEASE */
#endif

#ifndef COPRTHR_NAME
#define COPRTHR_NAME "Freewill"
#endif


//#if (COPRTHR_ALPHA)
//#define COPRTHR_VERSION_STRING \
//	"coprthr-"__stringify(COPRTHR_VERSION)"."__stringify(COPRTHR_REVISION) \
//	"-CURRENT ("COPRTHR_NAME")"
//#elif (COPRTHR_BETA)
//#define COPRTHR_VERSION_STRING \
//	"coprthr-"__stringify(COPRTHR_VERSION)"."__stringify(COPRTHR_REVISION) \
//	"-RC ("COPRTHR_NAME")"
//#else
//#define COPRTHR_VERSION_STRING \
//	"coprthr-"__stringify(COPRTHR_VERSION)"."__stringify(COPRTHR_REVISION) \
//	"."__stringify(COPRTHR_RELEASE)" ("COPRTHR_NAME")"
//#endif

#define COPRTHR_VERSION_STRING "coprthr-1.6.2 (Freewill)"


#define COPRTHR_VERSION_CODE \
	((COPRTHR_VERSION<<16)|(COPRTHR_REVISION<<8)|(COPRTHR_RELEASE))


#define GPL3_NOTICE \
"This program is free software; you may redistribute it under the terms of\n"\
"the GNU General Public License version 3 (GPLv3). This program has\n"\
"absolutely no warranty.\n"


#endif


