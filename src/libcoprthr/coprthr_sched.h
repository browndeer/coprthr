/* coprthr_sched.h 
 *
 * Copyright (c) 2009-2013 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _coprthr_sched_h
#define _coprthr_sched_h

#include <sys/queue.h>

struct coprthr_event {
	struct coprthr_device* dev;
	pthread_mutex_t mtx;
	pthread_cond_t sig;
	struct cmdcall_arg* cmd_argp;
	TAILQ_ENTRY(coprthr_event) cmds;
	int cmd;
	int cmd_stat;
   unsigned long tm_queued;
   unsigned long tm_submit;
   unsigned long tm_start;
   unsigned long tm_end;
};

#if defined(__FreeBSD__)
#define PTHREAD_MUTEX_ERRORCHECK_NP PTHREAD_MUTEX_ERRORCHECK
#endif

#define __coprthr_init_event(ev) do { \
	ev = (struct coprthr_event*)malloc(sizeof(struct coprthr_event)); \
	pthread_mutexattr_t attr; \
	int attrtype = PTHREAD_MUTEX_ERRORCHECK_NP; \
	pthread_mutexattr_init(&attr); \
	pthread_mutexattr_settype(&attr,attrtype); \
	pthread_mutex_init(&((ev)->mtx),&attr); \
	pthread_cond_init(&((ev)->sig),0); \
	pthread_mutex_unlock(&((ev)->mtx)); \
	(ev)->cmd_argp = 0; \
	} while(0)

#define __coprthr_free_event(ev) do { \
	pthread_cond_destroy(&((ev)->sig)); \
	pthread_mutex_destroy(&((ev)->mtx)); \
	__free((ev)->cmd_argp); \
	} while(0)



struct coprthr_command_queue {
   pthread_t td;
   pthread_mutex_t mtx;
   pthread_cond_t sig;
   unsigned int qstat;
   struct coprthr_event* cmd_submitted;
   struct coprthr_event* cmd_running;
   TAILQ_HEAD(tailhead_cmds_queued,coprthr_event) cmds_queued;
   TAILQ_HEAD(tailhead_cmds_complete,coprthr_event) cmds_complete;
};

#define __coprthr_init_command_queue(cmdq) do { \
   pthread_mutex_init(&(cmdq)->mtx,0); \
   pthread_cond_init(&(cmdq)->sig,0); \
   (cmdq)->qstat = 0; \
   (cmdq)->cmd_submitted = (struct coprthr_event*)0; \
   (cmdq)->cmd_running = (struct coprthr_event*)0; \
   TAILQ_INIT(&(cmdq)->cmds_queued); \
   TAILQ_INIT(&(cmdq)->cmds_complete); \
   } while(0)

#define __coprthr_free_command_queue(cmdq) do { \
   pthread_cond_destroy(&(cmdq)->sig); \
   pthread_mutex_destroy(&(cmdq)->mtx); \
	__free(cmdq); \
   } while(0)


/* cl_command_type */
#define __CL_COMMAND_NDRANGE_KERNEL                   0x11F0
#define __CL_COMMAND_TASK                             0x11F1
#define __CL_COMMAND_NATIVE_KERNEL                    0x11F2
#define __CL_COMMAND_READ_BUFFER                      0x11F3
#define __CL_COMMAND_WRITE_BUFFER                     0x11F4
#define __CL_COMMAND_COPY_BUFFER                      0x11F5
#define __CL_COMMAND_READ_IMAGE                       0x11F6
#define __CL_COMMAND_WRITE_IMAGE                      0x11F7
#define __CL_COMMAND_COPY_IMAGE                       0x11F8
#define __CL_COMMAND_COPY_IMAGE_TO_BUFFER             0x11F9
#define __CL_COMMAND_COPY_BUFFER_TO_IMAGE             0x11FA
#define __CL_COMMAND_MAP_BUFFER                       0x11FB
#define __CL_COMMAND_MAP_IMAGE                        0x11FC
#define __CL_COMMAND_UNMAP_MEM_OBJECT                 0x11FD
#define __CL_COMMAND_MARKER                           0x11FE
#define __CL_COMMAND_ACQUIRE_GL_OBJECTS               0x11FF
#define __CL_COMMAND_RELEASE_GL_OBJECTS               0x1200
#define __CL_COMMAND_READ_BUFFER_RECT                 0x1201
#define __CL_COMMAND_WRITE_BUFFER_RECT                0x1202
#define __CL_COMMAND_COPY_BUFFER_RECT                 0x1203
#define __CL_COMMAND_USER                             0x1204

/* command execution status */
#define __CL_COMPLETE                                 0x0
#define __CL_RUNNING                                  0x1
#define __CL_SUBMITTED                                0x2
#define __CL_QUEUED                                   0x3


#endif

