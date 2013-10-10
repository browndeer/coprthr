/* xcommand_queue.c
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

#include <CL/cl.h>

#include "xcl_structs.h"
#include "printcl.h"
#include "command_queue.h"
#include "xcmdsched.h"



void __do_create_command_queue_1( struct coprthr_device* dev ) 
{
	struct coprthr_command_queue* cmdq1 = dev->devstate->cmdq;

	if (!cmdq1)	/* XXX is this check necessary? -DAR */
		cmdq1 = (struct coprthr_command_queue*)
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

	dev->devstate->cmdq = cmdq1;

	printcl( CL_DEBUG "signaled cmdq1 with qstat ->1\n");
}

void __do_create_command_queue( cl_command_queue cmdq ) 
{

	__do_create_command_queue_1( cmdq->devid->codev );
	cmdq->ptr_imp = cmdq->devid->codev->devstate->cmdq;

#if(0)
	if (!cmdq->devid->codev->devstate->cmdq)	
		cmdq->devid->codev->devstate->cmdq = (struct coprthr_command_queue*)
			malloc( sizeof(struct coprthr_command_queue));
	__coprthr_init_command_queue(cmdq->devid->codev->devstate->cmdq);
	cmdq->ptr_imp = cmdq->devid->codev->devstate->cmdq;

	__lock_cmdq(cmdq);
	cmdq->ptr_imp->qstat = 0;
	__unlock_cmdq(cmdq);

	/* fork cmdq sched thread */
	int err; 
	if (err = pthread_create(&cmdq->ptr_imp->td,0,cmdqx0,(void*)cmdq) ) {
		printcl( CL_ERR "__do_create_command_queue:"
			" pthread_create failed");
		exit(1);
	}

	__lock_cmdq(cmdq);
	cmdq->ptr_imp->qstat = 1;
	__sig_cmdq(cmdq);
	__unlock_cmdq(cmdq);

	printcl( CL_DEBUG "signaled cmdq with qstat ->1\n");
#endif

}



void __do_release_command_queue( cl_command_queue cmdq ) 
{
	__lock_cmdq(cmdq);

	cmdq->ptr_imp->qstat = 2;

	/* check if the queue is empty and warn if its not */

	if (cmdq->ptr_imp->cmds_queued.tqh_first) {

		printcl( CL_WARNING 
			"__do_release_command_queue: cmds_queued not empty");

		cl_event ev;
		for(ev=cmdq->ptr_imp->cmds_queued.tqh_first; ev!=0; ev=ev->imp.cmds.tqe_next)
			printcl( CL_DEBUG "cmds_queued: ev %p\n",ev);

	}

	__sig_cmdq(cmdq);

	__unlock_cmdq(cmdq);

		
	/* now join the cmdq sched thread */
	void* st;

	pthread_join(cmdq->ptr_imp->td,&st);

	printcl( CL_DEBUG "__do_release_command_queue:"
		" cmdq->td joinied with status %d",st);	

}



void __do_enqueue_cmd( cl_command_queue cmdq, cl_event ev ) 
{
	struct timeval tv;

	printcl( CL_DEBUG "__do_enqueue_cmd: ev %p",ev);

	printcl( CL_DEBUG "cmdq1 %p", cmdq->ptr_imp);

	__lock_cmdq(cmdq);

	ev->cmdq = cmdq;

	ev->cmd_stat = CL_QUEUED;

	TAILQ_INSERT_TAIL(&cmdq->ptr_imp->cmds_queued,ev,imp.cmds);

	__sig_cmdq(cmdq);

	__unlock_cmdq(cmdq);

	gettimeofday(&tv,0);

	ev->tm_queued = tv.tv_sec * 1000000000 + tv.tv_usec * 1000;

}

/*
void __do_set_cmd_submitted( cl_event ev ) 
{
	__lock_cmdq(ev->cmdq);

	TAILQ_REMOVE(&ev->cmdq->imp.cmds_queued,ev,imp.cmds);

	ev->cmdq->imp.cmd_submitted = ev;

	ev->cmd_stat = CL_SUBMITTED;

	__unlock_cmdq(ev->cmdq);
}

void __do_cmd_set_running( cl_event ev ) 
{
	__lock_cmdq(ev->cmdq);

	ev->cmdq->imp.cmd_submitted = (cl_event)0;

	ev->cmdq->imp.cmd_running = ev;

	ev->cmd_stat = CL_RUNNING;

	__unlock_cmdq(ev->cmdq);
}

void __do_cmd_set_complete( cl_event ev ) 
{
	__lock_cmdq(ev->cmdq);

	ev->cmdq->imp.cmd_running = (cl_event)0;

	ev->cmd_stat = CL_COMPLETE;

	TAILQ_INSERT_TAIL(&ev->cmdq->imp.cmds_complete,ev,imp.cmds);

	__unlock_cmdq(ev->cmdq);
}
*/

void __do_finish( cl_command_queue cmdq )
{
       __lock_cmdq(cmdq);

       while ( !TAILQ_EMPTY(&cmdq->ptr_imp->cmds_queued)
               || cmdq->ptr_imp->cmd_submitted || cmdq->ptr_imp->cmd_running
       ) {

               printcl( CL_DEBUG "empty %d submitted %p running %p",
                       TAILQ_EMPTY(&cmdq->ptr_imp->cmds_queued),
                       cmdq->ptr_imp->cmd_submitted,cmdq->ptr_imp->cmd_running);

               __wait_cmdq(cmdq);


       }

       __unlock_cmdq(cmdq);

}

