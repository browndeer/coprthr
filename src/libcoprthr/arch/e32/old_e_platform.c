

#include "old_e_platform.h"

#include <string.h>

#include "printcl.h"

//#ifndef ENABLE_EMEK_BUILD
//#ifdef USE_OLD_ESDK
//#include "e_host.h"
//#else
//#include "e-hal.h"
//#endif
//
//#endif


int old_e_get_platform_info( 
	Epiphany_t* edev, 
	struct old_e_platform_info_struct* info )
{

#if defined(COMPILE_FOR_LEXINGTON)

	strncpy(info->e_platform_name,"E16G Lexington",32);
	info->e_device_id = 9001;
	info->e_global_mem_base = (void*)0x81000000;
	info->e_global_mem_size = 0x01000000;
	info->e_core_local_mem_size = 0x8000;
	info->e_core_base_addr = (void*)0x82400000;
	info->e_array_ncol = 4;
	info->e_array_nrow = 4;

#elif defined(COMPILE_FOR_NEEDHAM)

	strncpy(info->e_platform_name,"E16G Needham",32);
	info->e_device_id = 9002;
//	info->e_global_mem_base = (void*)0x8e000000;
	info->e_global_mem_base = (void*)0x8e100000;
//	info->e_global_mem_size = 0x02000000;
	info->e_global_mem_size = 0x01f00000;
	info->e_core_local_mem_size = 0x8000;
	info->e_core_base_addr = (void*)0x80800000;
	info->e_array_ncol = 4;
	info->e_array_nrow = 4;

#elif defined(COMPILE_FOR_NEEDHAMPRO)

	strncpy(info->e_platform_name,"E16G Needham Pro",32);
	info->e_device_id = 9003;
	info->e_global_mem_base = (void*)0x8e000000;
	info->e_global_mem_size = 0x02000000;
	info->e_core_local_mem_size = 0x8000;
	info->e_core_base_addr = (void*)0x80800000;
	info->e_array_ncol = 8;
	info->e_array_nrow = 8;

#elif defined(COMPILE_FOR_BLANK)

	strncpy(info->e_platform_name,"(blank)",32);
	info->e_device_id = 9004;
	info->e_global_mem_base = (void*)0x8e000000;
	info->e_global_mem_size = 0x02000000;
	info->e_core_local_mem_size = 0x8000;
	info->e_core_base_addr = (void*)0x80800000;
	info->e_array_ncol = 3;
	info->e_array_nrow = 3;


#else
#error No architecture specified
#endif

	return(0);	
}


