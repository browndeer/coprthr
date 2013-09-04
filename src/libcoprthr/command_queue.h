/* command_queue.h
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

#ifndef _command_queue_h
#define _command_queue_h

#include "xcl_structs.h"

#define __lock_cmdq(cmdq) pthread_mutex_lock(&(cmdq)->ptr_imp->mtx)

#define __unlock_cmdq(cmdq) pthread_mutex_unlock(&(cmdq)->ptr_imp->mtx)

#define __wait_cmdq(cmdq) \
	pthread_cond_wait(&(cmdq)->ptr_imp->sig,&(cmdq)->ptr_imp->mtx)

#define __sig_cmdq(cmdq) pthread_cond_signal(&(cmdq)->ptr_imp->sig)


void __do_create_command_queue( cl_command_queue cmdq );

void __do_release_command_queue( cl_command_queue cmdq );

void __do_enqueue_cmd( cl_command_queue cmdq, cl_event ev );

//void __do_cmd_set_submitted( cl_event ev );

//void __do_cmd_set_running( cl_event ev );

//void __do_cmd_set_queued( cl_event ev );

//void __do_cmd_set_submitted( cl_event ev );

void __do_finish( cl_command_queue cmdq );

#endif

