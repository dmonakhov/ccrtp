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

#ifdef	__NAMESPACES__
namespace ost {
#endif

RTPAudio::RTPAudio() :
RTPSocket(keyrtp.getInterface(), keyrtp.getPort(), keythreads.priRTP())
{
	rtp = this;
}

RTPAudio *rtp;

#ifdef	__NAMESPACES__
};
#endif
