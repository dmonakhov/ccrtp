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

#ifdef	__NAMESPACES__
namespace ost {
#endif

KeyThreads::KeyThreads() :
Keydata("/phone/threads")
{
	static KEYDEF defkeys[] = {
	{"audio", "0"},
	{"priority", "0"},
	{"rtp", "0"},
	{"gui", "0"},
	{"policy", "other"},
	{"stack", "8"},
	{NULL, NULL}};
	
	Load("~phone/threads");
	Load(defkeys);

	const char *cp = getLast("pri");
	
	if(cp)
		setValue("priority", cp);
}

KeyRTP::KeyRTP() :
Keydata("/phone/rtp")
{
	static KEYDEF defkeys[] = {
	{"interface", "*"},
	{"multicast", "*"},
	{"port", "3128"},
	{NULL, NULL}};

	Load("~phone/rtp");
	Load(defkeys);
}

KeyThreads keythreads;
KeyRTP keyrtp;

#ifdef	__NAMESPACES__
};
#endif
