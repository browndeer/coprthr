/* platform.c
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

#include <CL/cl.h>

#include "xcl_structs.h"
#include "platform.h"
#include "device.h"


static struct _cl_platform_id* __ptab = 0;
unsigned int __nplatforms = 0;


static unsigned int __ndevices = 0;
static struct _cl_device_id* __dtab = 0;
static struct _strtab_entry __dstrtab;


void __do_discover_platforms() 
{
	int i;

	if (__nplatforms > 0) return;

	__do_discover_devices(&__ndevices,&__dtab,&__dstrtab);

	__nplatforms = 1;
	__ptab = (struct _cl_platform_id*)malloc(sizeof(struct _cl_platform_id));
	
	__ptab[0].imp = (struct _imp_platform){
		"<profile>",
		"0.0.0",
		"coprthr",
		"Brown Deer Technology, LLC.",
		"<extensions>",
		__ndevices,__dtab,&__dstrtab
	};

}


void __do_release_platforms()
{
	__do_release_devices(__dtab,&__dstrtab);
}


void __do_get_nplatforms_avail(cl_uint* n)
{
	*n = __nplatforms;
}


void __do_get_platforms(cl_uint n, cl_platform_id* platformid)
{
	int i;
	for(i=0;i<n;i++) platformid[i] = &__ptab[i];
}


void __do_get_default_platformid( cl_platform_id* platformid )
{
	ERROR(__FILE__,__LINE__," no default platformid, crashing here");
	exit(-1);
//	*platformid = (cl_platform_id);
}


void __do_get_platform_profile(cl_platform_id platformid, char** p_str)
{
	*p_str = __resolve_platformid(platformid,profile);
}


void __do_get_platform_version(cl_platform_id platformid, char** p_str)
{
	*p_str = __resolve_platformid(platformid,version);
}


void __do_get_platform_name(cl_platform_id platformid, char** p_str)
{
	*p_str = __resolve_platformid(platformid,name);
}


void __do_get_platform_vendor(cl_platform_id platformid, char** p_str)
{
	*p_str = __resolve_platformid(platformid,vendor);
}


void __do_get_platform_extensions(cl_platform_id platformid, char** p_str)
{
	*p_str = __resolve_platformid(platformid,extensions);
}




