/* clproc.h
 *
 * Copyright (c) 2012 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _clproc_h
#define _clproc_h

//#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

struct clproc_process_struct {
	pid_t pid;
	uid_t uid;
	char command[32];
};

struct clproc_state_struct {
	int status;

	int nplat;
	int nctx;
	int ndev;
	int ncmdq;
	int nprg;
	int nkrn;
	int nbuf;

	struct timeval tstart;
	struct timeval telapsed;
	struct timeval twait;

	size_t gmem;

	int cmds;
	int cmds_queued;

	int krns;
	int krns_queued;

	int nerrs;
	int last_err;
};

#define CLPROC_STATUS_UNKNOWN		0
#define CLPROC_STATUS_RUNNING		1
#define CLPROC_STATUS_WAITING		2
#define CLPROC_STATUS_STOPPED		3
#define CLPROC_STATUS_COMPLETED	4

#endif

