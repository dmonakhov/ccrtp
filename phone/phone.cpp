// Copyright (C) 2000-2001 Open Source Telecom Corporation.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "server.h"
#include <getopt.h>

#ifdef	__NAMESPACES__
namespace ost {
#endif

bool daemon = true;

static void initial(int argc, char **argv)
{
	static bool usage = false;

	static struct option long_options[] = {
		{"background", 0, 0, 'D'},
                {"foreground", 0, 0, 'F'},
                {"daemon", 0, 0, 'D'},
                {"help", 0, 0, 'h'},
                {"priority", 1, 0, 'p'},
		{0, 0, 0, 0}};
		
	int idx, opt;	
		
	while(EOF != (opt = getopt_long(argc, argv, "p:FDh", long_options, &idx)))
	{
		switch(opt)
		{
		case 'p':
			keythreads.setValue("priority", optarg);
			break;	
		case 'F':
			daemon = false;
			break;
		case 'D':
			daemon = true;
			break;
		default:
			usage = true;
		}	
	}
	if(usage)
	{
		cerr << "use: phone [options] [parties...]" << endl;
		exit(-1);
	}
}
		
static int getPid() 
{
	int pid, fd;
	char buf[20];
	
	fd = ::open(".phonepid", O_RDONLY);
	if(fd < 0)
		return 0;
		
	::read(fd, buf, 16);
	buf[10] = 0;
	::close(fd);
	pid = atol(buf);
	if(kill(pid, 0))
		return 0;
	return pid;
}	

#ifdef	__NAMESPACES__
extern "C" {
#endif

int main(int argc, char **argv)
{
	int pid = 0;
	
	chdir(getenv("HOME"));
	if(canAccess(".phonepid"))
		if(canModify(".phonectrl"))
			pid = getPid();		

	initial(argc, argv);

	if(!pid)
	{
		pid = fork();
		if(!pid)
		{
//			server();
			exit(0);
		}
		sleep(2);
	}
	exit(0);
}

#ifdef	__NAMESPACES__
}; };
#endif
