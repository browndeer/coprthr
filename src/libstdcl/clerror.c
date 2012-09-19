/* clerror.c
 *
 * Copyright (c) 2012 Brown Deer Technology, LLC.  All Rights Reserved.
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

#include <stdio.h>

int oclerrno;
int clerrno;

static const char* __oclerror_strtab[] = {
	"OpenCL: CL_SUCCESS",                           			/* 0 */
	"OpenCL: CL_DEVICE_NOT_FOUND",                  			/* -1 */
	"OpenCL: CL_DEVICE_NOT_AVAILABLE",              			/* -2 */
	"OpenCL: CL_COMPILER_NOT_AVAILABLE",            			/* -3 */
	"OpenCL: CL_MEM_OBJECT_ALLOCATION_FAILURE",     			/* -4 */
	"OpenCL: CL_OUT_OF_RESOURCES",                  			/* -5 */
	"OpenCL: CL_OUT_OF_HOST_MEMORY",                			/* -6 */
	"OpenCL: CL_PROFILING_INFO_NOT_AVAILABLE",      			/* -7 */
	"OpenCL: CL_MEM_COPY_OVERLAP",                  			/* -8 */
	"OpenCL: CL_IMAGE_FORMAT_MISMATCH",             			/* -9 */
	"OpenCL: CL_IMAGE_FORMAT_NOT_SUPPORTED",        			/* -10 */
	"OpenCL: CL_BUILD_PROGRAM_FAILURE",             			/* -11 */
	"OpenCL: CL_MAP_FAILURE",                       			/* -12 */
	"OpenCL: CL_MISALIGNED_SUB_BUFFER_OFFSET",      			/* -13 */
	"OpenCL: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST",	/* -14 */
	"(error undefined)",													/* -15 */
	"(error undefined)",													/* -16 */
	"(error undefined)",													/* -17 */
	"(error undefined)",													/* -18 */
	"(error undefined)",													/* -19 */
	"(error undefined)",													/* -20 */
	"(error undefined)",													/* -21 */
	"(error undefined)",													/* -22 */
	"(error undefined)",													/* -23 */
	"(error undefined)",													/* -24 */
	"(error undefined)",													/* -25 */
	"(error undefined)",													/* -26 */
	"(error undefined)",													/* -27 */
	"(error undefined)",													/* -28 */
	"(error undefined)",													/* -29 */
	"OpenCL: CL_INVALID_VALUE",                     			/* -30 */
	"OpenCL: CL_INVALID_DEVICE_TYPE",               			/* -31 */
	"OpenCL: CL_INVALID_PLATFORM",                  			/* -32 */
	"OpenCL: CL_INVALID_DEVICE",                    			/* -33 */
	"OpenCL: CL_INVALID_CONTEXT",                   			/* -34 */
	"OpenCL: CL_INVALID_QUEUE_PROPERTIES",          			/* -35 */
	"OpenCL: CL_INVALID_COMMAND_QUEUE",             			/* -36 */
	"OpenCL: CL_INVALID_HOST_PTR",                  			/* -37 */
	"OpenCL: CL_INVALID_MEM_OBJECT",                			/* -38 */
	"OpenCL: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",   			/* -39 */
	"OpenCL: CL_INVALID_IMAGE_SIZE",                			/* -40 */
	"OpenCL: CL_INVALID_SAMPLER",                   			/* -41 */
	"OpenCL: CL_INVALID_BINARY",                    			/* -42 */
	"OpenCL: CL_INVALID_BUILD_OPTIONS",             			/* -43 */
	"OpenCL: CL_INVALID_PROGRAM",                   			/* -44 */
	"OpenCL: CL_INVALID_PROGRAM_EXECUTABLE",        			/* -45 */
	"OpenCL: CL_INVALID_KERNEL_NAME",               			/* -46 */
	"OpenCL: CL_INVALID_KERNEL_DEFINITION",         			/* -47 */
	"OpenCL: CL_INVALID_KERNEL",                    			/* -48 */
	"OpenCL: CL_INVALID_ARG_INDEX",                 			/* -49 */
	"OpenCL: CL_INVALID_ARG_VALUE",                 			/* -50 */
	"OpenCL: CL_INVALID_ARG_SIZE",                  			/* -51 */
	"OpenCL: CL_INVALID_KERNEL_ARGS",               			/* -52 */
	"OpenCL: CL_INVALID_WORK_DIMENSION",            			/* -53 */
	"OpenCL: CL_INVALID_WORK_GROUP_SIZE",           			/* -54 */
	"OpenCL: CL_INVALID_WORK_ITEM_SIZE",            			/* -55 */
	"OpenCL: CL_INVALID_GLOBAL_OFFSET",             			/* -56 */
	"OpenCL: CL_INVALID_EVENT_WAIT_LIST",           			/* -57 */
	"OpenCL: CL_INVALID_EVENT",                     			/* -58 */
	"OpenCL: CL_INVALID_OPERATION",                 			/* -59 */
	"OpenCL: CL_INVALID_GL_OBJECT",                 			/* -60 */
	"OpenCL: CL_INVALID_BUFFER_SIZE",               			/* -61 */
	"OpenCL: CL_INVALID_MIP_LEVEL",                 			/* -62 */
	"OpenCL: CL_INVALID_GLOBAL_WORK_SIZE",          			/* -63 */
	"OpenCL: CL_INVALID_PROPERTY"                 				/* -64 */
};

static const char* __clerror_strtab[] = {
	"CL: CL_SUCCESS",														/* 0 */
	"CL: CL_ERROR"															/* -1 */
};


const char __clerrno_out_of_range[] = "CL: (error undefined)";

const char* 
oclerror_str( int n )
{
	if ( n >= 0 || -n > sizeof(__oclerror_strtab)/sizeof(char*) ) return 0;
	return __oclerror_strtab[-n];
}


const char* 
clerror_str( int n )
{
	if ( n >= 0 || -n > sizeof(__clerror_strtab)/sizeof(char*) ) 
		return __clerrno_out_of_range;

	return __clerror_strtab[-n];
}


void oclperror( const char* s )
{
	if (s) 	
		fprintf( stderr, "%s: %s\n",s,oclerror_str( oclerrno ) );
	else
		fprintf( stderr, "%s\n",oclerror_str( oclerrno ) );
}

void clperror( const char* s )
{
	if (s)
		fprintf( stderr, "%s: %s\n",s,clerror_str( clerrno ) );
	else
		fprintf( stderr, "%s\n",clerror_str( clerrno ) );
}


