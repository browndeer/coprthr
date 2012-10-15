/* util.c
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

#include <stdlib.h>
#include <stdio.h>

#include "util.h"

int _xcl_report_level = -1;

char _xcl_report_buffer[XCL_REPORT_BUFFER_SIZE];

char* _xcl_report_tag[] = { "EMERGENCY", "ALERT", "CRITICAL", "ERROR", 
	"WARNING", "notice", "info", "debug"
};

void _xclreport(char* file, int line)
{
	if (_xcl_report_level < 0) {
		char* envset = getenv("COPRTHR_XCL_REPORT_LEVEL");
		_xcl_report_level = (envset)? atoi(envset) : XCL_REPORT_DEFAULT_LEVEL;
		fprintf(stderr,"xcl: _xcl_report_level set to %d\n",_xcl_report_level);
	}

	int level = 7;
	char* buf = _xcl_report_buffer;

	if ( _xcl_report_buffer[0] == '<' && _xcl_report_buffer[1] != '\0' 
		&& _xcl_report_buffer[2] == '>') {

		level = _xcl_report_buffer[1] - '0';

		if (level<0 || level>7) level = 7;
	
		buf += 3;

	}

	if (level <= _xcl_report_level) {

		fprintf(stderr, "[%d]xcl: %s: %s(%d): %s\n", 
			getpid(),
			_xcl_report_tag[level],
			file,line,buf);

		fflush(stderr);

	}

}

