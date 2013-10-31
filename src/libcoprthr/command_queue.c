/* command_queue.c
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

#include <sys/time.h>
#include <pthread.h>

#include "printcl.h"
#include "command_queue.h"
#include "cmdsched.h"

#include "coprthr_sched.h"


void __do_create_command_queue_1( struct coprthr_device* dev ) 
{
	struct coprthr_command_queue* cmdq1 = dev->devstate->cmdq;

//	if (!cmdq1)	/* XXX is this check necessary? -DAR */
	if (cmdq1) {
		printcl( CL_WARNING "__do_create_command_queue_1: cmdq exists" );
		return;
	}

	dev->devstate->cmdq = cmdq1 = (struct coprthr_command_queue*)
			malloc( sizeof(struct coprthr_command_queue));

	__coprthr_init_command_queue(cmdq1);

	__lock_cmdq1(cmdq1);
	cmdq1->qstat = 0;
	__unlock_cmdq1(cmdq1);

	/* fork cmdq sched thread */
	int err; 
	if (err = pthread_create(&(cmdq1->td),0,cmdqx1,(void*)dev) ) {
		printcl( CL_ERR "__do_create_command_queue_1:"
			" pthread_create failed");
		exit(1);
	}

	__lock_cmdq1(cmdq1);
	cmdq1->qstat = 1;
	__sig_cmdq1(cmdq1);
	__unlock_cmdq1(cmdq1);

//	dev->devstate->cmdq = cmdq1;

	printcl( CL_DEBUG "signaled cmdq1 with qstat ->1\n");
}


void __do_release_command_queue_1( struct coprthr_device* dev ) 
{
	struct coprthr_command_queue* cmdq1 = dev->devstate->cmdq;
	
	__lock_cmdq1(cmdq1);

	cmdq1->qstat = 2;

	/* check if the queue is empty and warn if its not */

	if (cmdq1->cmds_queued.tqh_first) {

		printcl( CL_WARNING 
			"__do_release_command_queue_1: cmds_queued not empty");

		struct coprthr_event* ev1;
		for(ev1=cmdq1->cmds_queued.tqh_first; ev1!=0; ev1=ev1->cmds.tqe_next)
			printcl( CL_DEBUG "cmds_queued: ev %p\n",ev1);

	}

	__sig_cmdq1(cmdq1);

	__unlock_cmdq1(cmdq1);

		
	/* now join the cmdq sched thread */
	void* st;

	pthread_join(cmdq1->td,&st);

	printcl( CL_DEBUG "__do_release_command_queue_1:"
		" cmdq->td joinied with status %d",st);	

}


void __do_enqueue_cmd_1( struct coprthr_device* dev, 
	struct coprthr_event* ev1 ) 
{
	struct coprthr_command_queue* cmdq1 = dev->devstate->cmdq;
	
	struct timeval tv;

	printcl( CL_DEBUG "__do_enqueue_cmd_1: ev %p",ev1);

	printcl( CL_DEBUG "cmdq1 %p", cmdq1);

	__lock_cmdq1(cmdq1);

	ev1->dev = dev;

	ev1->cmd_stat = __CL_QUEUED;

	TAILQ_INSERT_TAIL(&cmdq1->cmds_queued,ev1,cmds);

	__sig_cmdq1(cmdq1);

	__unlock_cmdq1(cmdq1);

	gettimeofday(&tv,0);

	ev1->tm_queued = tv.tv_sec * 1000000000 + tv.tv_usec * 1000;

}


void __do_finish_1( struct coprthr_device* dev )
{
	struct coprthr_command_queue* cmdq1 = dev->devstate->cmdq;

       __lock_cmdq1(cmdq1);

       while ( !TAILQ_EMPTY(&cmdq1->cmds_queued)
               || cmdq1->cmd_submitted || cmdq1->cmd_running
       ) {

               printcl( CL_DEBUG "empty %d submitted %p running %p",
                       TAILQ_EMPTY(&cmdq1->cmds_queued),
                       cmdq1->cmd_submitted,cmdq1->cmd_running);

               __wait_cmdq1(cmdq1);


       }

       __unlock_cmdq1(cmdq1);

}


void __do_exec_cmd_1( struct coprthr_device* dev, 
	struct coprthr_event* ev1 ) 
{
//	cmdcall_t* cmdcall = dev->devops->v_cmdcall;
	cmdcall_t* cmdcall = dev->v_cmdcall;

	struct timeval tv;

	printcl( CL_DEBUG "__do_exec_cmd_1: ev %p",ev1);

	ev1->dev = dev;

	ev1->tm_submit = 0;
	ev1->tm_queued = 0;

	ev1->cmd_stat = __CL_RUNNING;

	gettimeofday(&tv,0);
	ev1->tm_start = tv.tv_sec * 1000000000 + tv.tv_usec * 1000;

	cmdcall[ev1->cmd-CLCMD_OFFSET](dev,ev1->cmd_argp);

	gettimeofday(&tv,0);
	ev1->tm_end = tv.tv_sec * 1000000000 + tv.tv_usec * 1000;

	ev1->cmd_stat = __CL_COMPLETE;

}

