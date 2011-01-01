/* platform.h 
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

#ifndef _platform_h
#define _platform_h

#include "xcl_structs.h"


void __do_discover_platforms();

void __do_get_nplatforms_avail(cl_uint* n);

void __do_get_platforms(cl_uint n, cl_platform_id* p_id);

void __do_get_default_platformid( cl_platform_id* );

void __do_get_platform_profile(cl_platform_id id, char** p_str);

void __do_get_platform_version(cl_platform_id id, char** p_str);

void __do_get_platform_name(cl_platform_id id, char** p_str);

void __do_get_platform_vendor(cl_platform_id id, char** p_str);

void __do_get_platform_extensions(cl_platform_id id, char** p_str);

#endif

