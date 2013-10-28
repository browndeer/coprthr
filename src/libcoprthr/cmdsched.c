/* cmdsched.c 
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

#define _GNU_SOURCE
#include <sched.h>
#include <sys/time.h>
#include <pthread.h>

#include <assert.h>

#include "printcl.h"
#include "cpuset_type.h"
#include "cmdsched.h"
#include "command_queue.h"
#include "cmdcall.h"
#include "event.h"

#include "coprthr_sched.h"

#define __CL_COMMAND_OFFSET (__CL_COMMAND_NDRANGE_KERNEL-1)

void* cmdqx1( void* argp )
{

	struct timeval tv;

	struct coprthr_device* dev = (struct coprthr_device*)argp;
	struct coprthr_command_queue* cmdq1 = dev->devstate->cmdq;


	/* XXX set cpu affinity of td based on device recommendation -DAR */
	
	pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),
		&(dev->devstate->cpumask));


	printcl( CL_DEBUG "cmdqx1: cmdq1=%p dev=%p cpumask_count=?",
		dev->devstate->cmdq,dev);

/* XXX most of the useful cpu_set macros are missing from rhel/centos 
 * maybe they will get to that after they upgrade to a modern GCC version -DAR
 */


//	cmdcall_t* cmdcall = dev->devops->v_cmdcall;
	cmdcall_t* cmdcall = dev->v_cmdcall;

	assert(__CL_COMMAND_RELEASE_GL_OBJECTS - CLCMD_OFFSET == CLCMD_NUM);

	struct coprthr_event* ev1;

	__lock_cmdq1(cmdq1);

	printcl( CL_DEBUG "cmdqx1: idle");

	while (cmdq1->qstat == 0) {
		printcl( CL_DEBUG "idle-sleep\n");
		__wait_cmdq1(cmdq1);
		printcl( CL_DEBUG "idle-wake\n");
	}

	__unlock_cmdq1(cmdq1);


	__lock_cmdq1(cmdq1);

	printcl( CL_INFO "cmdqx1: run");

	while (cmdq1->qstat == 1) {

		printcl( CL_DEBUG "cmdqx1:" " queued %p",cmdq1->cmds_queued.tqh_first);

		while (ev1 = cmdq1->cmds_queued.tqh_first) {


			printcl( CL_DEBUG "cmdqx1:" " submit cmd ev1 %p",ev1);

			/* 
			 * submit cmd 
			 */

			printcl( CL_DEBUG "cmdqx1: attempt __lock_event1");
			__lock_event1(ev1);

			TAILQ_REMOVE(&(cmdq1->cmds_queued),ev1,cmds);
			cmdq1->cmd_submitted = ev1;
			ev1->cmd_stat = __CL_SUBMITTED;
			gettimeofday(&tv,0);
			ev1->tm_submit = tv.tv_sec * 1000000000 + tv.tv_usec * 1000;
			ev1->tm_start = tv.tv_sec * 1000000000 + tv.tv_usec * 1000;

			__unlock_cmdq1(cmdq1);
			printcl( CL_DEBUG "%p: submitted %x\n",ev1,ev1->cmd);
			cmdcall[ev1->cmd-CLCMD_OFFSET](dev,ev1->cmd_argp);
			__lock_cmdq1(cmdq1);

			__sig_event1(ev1);

			__unlock_event1(ev1);


			/* XXX cmdcall should either be moved down to running, OR the call
			 * XXX should be executed via fork so that event can be marked running
			 * XXX when it is really running.  issue is confusion over submit/run
			 * XXX which i guess means that a cmd can be submitted and held for
			 * XXX execution in a derivative device queue. -DAR */

			/* 
			 * cmd running 
			 */

			__lock_event1(ev1);

			ev1 = cmdq1->cmd_submitted;
			cmdq1->cmd_submitted = 0;
			cmdq1->cmd_running = ev1;
			ev1->cmd_stat = __CL_RUNNING;

			__unlock_cmdq1(cmdq1);
			printcl( CL_DEBUG "%p: running %x\n",ev1,ev1->cmd);
			__lock_cmdq1(cmdq1);

			__sig_event1(ev1);

			__unlock_event1(ev1);



			/* 
			 * cmd complete 
			 */

			__lock_event1(ev1);

			ev1 = cmdq1->cmd_running;
			cmdq1->cmd_running = 0;
			TAILQ_INSERT_TAIL(&(cmdq1->cmds_complete),ev1,cmds);
			ev1->cmd_stat = __CL_COMPLETE;
			gettimeofday(&tv,0);
			ev1->tm_end = tv.tv_sec * 1000000000 + tv.tv_usec * 1000;

			__sig_event1(ev1);

			__unlock_event1(ev1);

			printcl( CL_DEBUG "%p: complete %x\n",ev1,ev1->cmd);
			
		}

		/* XXX added to wake up thread that might be blocking on finish(cmdq) */
		__sig_cmdq1(cmdq1);

		printcl( CL_DEBUG "run-cmdqx1 sleep\n");
		__wait_cmdq1(cmdq1);
		printcl( CL_DEBUG "run-cmdqx1 wake\n");

	}

	__unlock_cmdq1(cmdq1);
	
	printcl( CL_INFO "cmdqx1: shutdown");
	
	pthread_exit((void*)0);
}

