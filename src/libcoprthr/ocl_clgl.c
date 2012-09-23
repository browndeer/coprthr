/* ocl_clgl.c 
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


#include <CL/cl.h>
#include <CL/cl_gl.h>

#include "xcl_structs.h"
#include "printcl.h"

// CL-GL Interoperability API calls


cl_mem 
_clCreateFromGLBuffer(
	cl_context ctx, cl_mem_flags flags, cl_GLuint dummy1, int* err_ret
)
{
	printcl( CL_WARNING "unsupported");

	return((cl_mem)0);
}

cl_mem 
_clCreateFromGLTexture2D(
	cl_context ctx, cl_mem_flags flags, 
	cl_GLenum dummy1, cl_GLint dummy2, cl_GLuint dummy3, cl_int* err_ret
)
{
	printcl( CL_WARNING "unsupported");

	return((cl_mem)0);
}

cl_mem 
_clCreateFromGLTexture3D(
	cl_context ctx, cl_mem_flags flags, 
	cl_GLenum dummy1, cl_GLint dummy2, cl_GLuint dummy3, cl_int* err_ret
)
{
	printcl( CL_WARNING "unsupported");

	return((cl_mem)0);
}

cl_mem 
_clCreateFromGLRenderbuffer(
	cl_context ctx, cl_mem_flags flags, cl_GLuint dummy, cl_int* err_ret
) 
{
	printcl( CL_WARNING "unsupported");

	return((cl_mem)0);
}

cl_int 
_clGetGLObjectInfo(
	cl_mem membuf, cl_gl_object_type* dummy1, cl_GLuint* dummy2
)
{
   printcl( CL_WARNING "unsupported");

   return(CL_SUCCESS);
}

                  
cl_int 
_clGetGLTextureInfo(
	cl_mem membuf, cl_gl_texture_info dummy1, size_t dummy2, 
	void* dummy3, size_t* dummy4
)
{
	printcl( CL_WARNING "unsupported");

	return(CL_SUCCESS);
}

cl_int 
_clEnqueueAcquireGLObjects(
	cl_command_queue cmdq, cl_uint dummy1, const cl_mem* dummy2, 
	cl_uint dummy3, const cl_event* dummy4, 
	cl_event* ev
)
{
	printcl( CL_WARNING "unsupported");

	return(CL_SUCCESS);
}

cl_int 
_clEnqueueReleaseGLObjects(
	cl_command_queue cmdq, 
	cl_uint dummy1, const cl_mem* dummy2, cl_uint dummy3, 
	const cl_event* dummy4, cl_event* dummy5
)
{
	printcl( CL_WARNING "unsupported");

	return(CL_SUCCESS);
}

cl_int 
_clGetGLContextInfoKHR(
	const cl_context_properties* ctxprop, 
	cl_gl_context_info dummy, size_t dummy2, void* dummy3, size_t* dummy4
)
{
	printcl( CL_WARNING "unsupported");

	return(CL_SUCCESS);
}

