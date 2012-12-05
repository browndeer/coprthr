
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>

#include "clproc.h"

/*** brief
  PID|USERNAME|STATE| GMEM| NDEV|CMDS/Q  |KRNS/Q  |ERRS|ERR|TWAIT|COMMAND      |
    5        8     5     5     5    4/3      4/3      4/3       5 13
 ***/

#define TABLE_HEADER_BRIEF \
	"  PID USERNAME STATE  GMEM  NDEV CMDS/Q   KRNS/Q   ERRS/ERR TWAIT COMMAND"

#define FORMAT_BRIEF "%5d %8s %5s %5s %5d %4s/%-3d %4s/%-3d %4s/%-3d %5s "

void snprintf_units( char* buf, size_t n, unsigned int val)
{
	if (val >= 1000000)
		snprintf(buf,n,"%3dM",val/1000000);
	if (val >= 1000)
		snprintf(buf,n,"%3dK",val/1000);
	else
		snprintf(buf,n,"%3d",val);
}

void snprintf_timeval( char* buf, size_t n, unsigned int sec, unsigned int usec)
{
	if (sec==0)
		if (usec==0)
			snprintf(buf,n,"    0");
		else if (usec<1000) 
			snprintf(buf,n,"%3dus",usec);
		else
			snprintf(buf,n,"%3dms",usec/1000);
	else 
		snprintf(buf,n,"%2d:%02d",sec/60,sec%60);
}

		
int main()
{
	char* buffer = (char*)malloc(256);
	char* filename = (char*)malloc(sizeof(struct clproc_state_struct));
	struct clproc_state_struct state;

	char* str_state = (char*)malloc(5+1);
	char* str_gmem = (char*)malloc(5+1);
	char* str_cmds = (char*)malloc(4+1);
	char* str_krns = (char*)malloc(4+1);
	char* str_errs = (char*)malloc(4+1);
	char* str_twait = (char*)malloc(5+1);

	for(;;) {

		fprintf(stdout,"\33[2J\33[0;0H" );

		fprintf( stdout, TABLE_HEADER_BRIEF "\n" );
	
		DIR* dirp = opendir( "/var/clproc/" );

		struct dirent* dp;

		if (dirp) while ( (dp=readdir(dirp)) ) {

			pid_t pid = atoi(dp->d_name);

			if (!pid) continue;

			snprintf(filename,64,"/var/clproc/%d/state",(int)pid);

			struct stat fs;
			struct passwd* pwd = (struct passwd*)malloc(sizeof(struct passwd));
			struct passwd* pwd_ret;

			int fd = open(filename,O_RDONLY);
			if ( fstat(fd,&fs) );
			uid_t uid = fs.st_uid;
			read(fd,&state,sizeof(struct clproc_state_struct));
			close(fd);

			getpwuid_r(uid,pwd,buffer,256,&pwd_ret);

			strcpy(str_state,"     ");

			switch(state.status) {
				case(CLPROC_STATUS_RUNNING):		str_state[4] = 'R'; break;
				case(CLPROC_STATUS_WAITING):		str_state[4] = 'W'; break;
				case(CLPROC_STATUS_STOPPED):		str_state[4] = 'S'; break;
				case(CLPROC_STATUS_COMPLETED):	str_state[4] = 'C'; break;
				default: str_state[4] = '?';
			}

			if (state.status != CLPROC_STATUS_COMPLETED && kill(pid,0)) {
				str_state[4] = 'X';
			}

			snprintf_units( str_gmem, 6, state.gmem);
			snprintf_units( str_cmds, 5, state.cmds);
			snprintf_units( str_krns, 5, state.krns);
			snprintf_units( str_errs, 5, state.errs);
			snprintf_timeval(str_twait,6,state.twait.tv_sec,state.twait.tv_usec);

			fprintf( stdout,
				FORMAT_BRIEF "\n",
				pid,pwd->pw_name,str_state,
				str_gmem,
				state.ndev,
				str_cmds,state.cmds_queued,
				str_krns,state.krns_queued,
				str_errs,state.errno,
				str_twait);

		}

		closedir(dirp);

	sleep(1);

	}

	free(filename);
	free(str_state);
	free(str_gmem);

}

