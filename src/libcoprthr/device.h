/* device.h 
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

#ifndef _device_h
#define _device_h

#include <CL/cl.h>

#include "xcl_structs.h"

void __do_discover_devices( unsigned int* ndevices, 
	struct _cl_device_id**, struct _strtab_entry*, int flags );

//void __do_release_devices(struct _cl_device_id*, struct _strtab_entry*);

void __do_get_ndevices(cl_platform_id, cl_device_type, cl_uint*);

void __do_get_devices(cl_platform_id, cl_device_type, cl_uint, cl_device_id*);

#endif

