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

#include <cc++/slog.h>
#include "server.h"

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

void server(void)
{	
	const char *reason = "exiting";
	char *cp, *ep;
	fstream fifo;
	new RTPAudio;

	::signal(SIGPIPE, SIG_IGN);

	int fd;
	char buf[256];

	::remove(".phonepid");

	if(daemon)
	{
		close(0);
		close(1);
		close(2);
		pdetach();
		open("/dev/null", O_RDWR);
		open("/dev/null", O_RDWR);
		open("/dev/null", O_RDWR);
		slog.open("phone", SLOG_DAEMON);
		slog.level(SLOG_NOTICE);
		slog(SLOG_NOTICE) << "daemon mode started" << endl;
	}
	else
	{
		slog.open("phone", SLOG_DAEMON);
		slog.level(SLOG_DEBUG);
		slog(SLOG_NOTICE) << "server starting..." << endl;
	}
	snprintf(buf, 11, "%d", getpid());
	fd = ::creat(".phonepid", 0660);
	if(fd > -1)
	{
		::write(fd, buf, 10);
		::close(fd);
	}
	fifo.open(".phonectrl", ios::in | ios::out);
	if(!fifo.is_open())
	{
		slog(SLOG_ERROR) << "fifo failed; exiting" << endl;
		exit(-1);
	}

	rtp->Start();	// we assume it's always running

	while(!fifo.eof())
	{
		fifo.getline(buf, 256);
		cp = buf;
		while(isspace(*cp))
			++cp;
		ep = strrchr(cp, '\n');
		if(ep)
			*ep = 0;
		if(!*cp)
			continue;	
		slog(SLOG_DEBUG) << "fifo: " << cp << endl;
		if(!strnicmp(cp, "exit", 4))
			break;

	}
	rtp->Exit(reason);
	fifo.close();
	slog(SLOG_WARNING) << "server exiting..." << endl;
	exit(0);
}

#ifdef	CCXX_NAMESPACES
};
#endif
