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
using namespace std;
#endif

RTPEvent *RTPEvent::first = NULL;

RTPEvent::RTPEvent()
{
	next = first;
	first = this;
}

RTPAudio::RTPAudio() :
RTPSocket(keyrtp.getInterface(), keyrtp.getPort(), keythreads.priRTP())
{
	rtp = this;
	setTimeout(keyrtp.getTimer());
	setExpired(keyrtp.getExpire());
	groups = 0;
	unicast = false;
	shutdown = false;
}

void RTPAudio::exit(const char *reason)
{
	shutdown = true;
	bye(reason);
	sleep(500);
	delete rtp;
	rtp = NULL;
}

void RTPAudio::gotHello(RTPSource &src)
{
	RTPEvent *event = RTPEvent::first;

	slog(Slog::levelDebug) << "hello(" << src.getID() << ") "
		<< src.getCNAME() << endl;

	while(event)
	{
		event->gotHello(src);
		event = event->next;
	}
}

void RTPAudio::gotGoodbye(RTPSource &src, char *reason)
{
	RTPEvent *event = RTPEvent::first;

	slog(Slog::levelDebug) << "bye(" << src.getID() << ") "
		<< src.getCNAME();
	if(reason)
		slog() << "; " << reason;
	slog() << endl;

	while(event)
	{
		event->gotGoodbye(src, reason);
		event = event->next;
	}
}
	
RTPAudio *rtp;

#ifdef	CCXX_NAMESPACES
};
#endif
