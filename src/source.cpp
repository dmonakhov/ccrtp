// Copyright (C) 2001,2002,2003 Federico Montesino <fedemp@altern.org>
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
// 
// As a special exception to the GNU General Public License, permission is 
// granted for additional uses of the text contained in its release 
// of ccRTP.
// 
// The exception is that, if you link the ccRTP library with other
// files to produce an executable, this does not by itself cause the
// resulting executable to be covered by the GNU General Public License.
// Your use of that executable is in no way restricted on account of
// linking the ccRTP library code into it.
// 
// This exception does not however invalidate any other reasons why
// the executable file might be covered by the GNU General Public License.
// 
// This exception applies only to the code released under the 
// name ccRTP.  If you copy code from other releases into a copy of
// ccRTP, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
// 
// If you write modifications of your own for ccRTP, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.  

/**
 * @file control.cpp
 *
 * @short SDESItemsHolder, RTPSource and Participant classes implementation.
 **/

#include <cc++/process.h>
#include "private.h"
#include <ccrtp/sources.h>

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

#ifdef WIN32
int gettimeofday(struct timeval *tv_,  void *tz_)
{
	// We could use _ftime(), but it is not available on WinCE.
	// (WinCE also lacks time.h)
	// Note also that the average error of _ftime is around 20 ms :)
	DWORD ms = GetTickCount();
	tv_->tv_sec = ms / 1000;
	tv_->tv_usec = ms * 1000;
	return 0;
}
#endif //WIN32

static void
findusername(std::string &username);

#ifndef WIN32
static void
findusername(std::string &username)
{
	// LOGNAME environment var has two advantages:
	// 1) avoids problems of getlogin(3) and cuserid(3)
	// 2) unlike getpwuid, takes into account user
	//    customization of the environment.
	// Try both LOGNAME and USER env. var.
	const char *user = Process::getEnv("LOGNAME");
	if ( !strcmp(user,"") )
		user = Process::getEnv("USER");
	username = user;
}

#else

static void
findusername(std::string &username)
{
	unsigned long len = 0;
	if ( GetUserName(NULL,&len) && (len > 0) ) {
		char *n = new char[len];	 
		GetUserName(n,&len);
		username = n;
		delete [] n;
	} else {
		username = "unidentified";
	}
}
#endif // #ifndef WIN32

void 
SDESItemsHolder::setItem(SDESItemType item, const std::string& val)
{
	if ( item > SDESItemTypeEND && item <= SDESItemTypeH323CADDR ) {
		sdesItems[item] = val;
	}
}
	
const std::string&
SDESItemsHolder::getItem(SDESItemType type) const
{
	if ( type > SDESItemTypeEND && type <= SDESItemTypeH323CADDR ) {
		return sdesItems[type];
	} else
		return sdesItems[SDESItemTypeCNAME];
}

SyncSource::SyncSource(uint32 ssrc):
	state(stateUnknown), SSRC(ssrc), participant(NULL),
	networkAddress("0"), dataTransportPort(0), controlTransportPort(0)
{
}

SyncSource::~SyncSource()
{
	activeSender = false;
	state = statePrevalid;
}

Participant::Participant(const std::string& cname) : SDESItemsHolder()
{
	SDESItemsHolder::setItem(SDESItemTypeCNAME,cname);
}

Participant::~Participant()
{
}

const size_t RTPApplication::defaultParticipantsNum = 11;

RTPApplication& defaultApplication()
{
	// default application CNAME is automatically assigned.
	static RTPApplication defApp("");

	return defApp; 
}

RTPApplication::RTPApplication(const std::string& cname) :
		SDESItemsHolder(),
		participants( new Participant* [defaultParticipantsNum] ),
		firstPart(NULL), lastPart(NULL)
{
	// guess CNAME, in the form of user@host_fqn
	if ( cname.length() > 0 )
		SDESItemsHolder::setItem(SDESItemTypeCNAME,cname);
	else
		findCNAME();
}

RTPApplication::~RTPApplication()
{
	ParticipantLink *p;
	while ( NULL != firstPart ) {
		p = firstPart;
		firstPart = firstPart->getNext();
		try {
			delete p;
		} catch (...) {}
	}
	lastPart = NULL;
	try {
		delete [] participants;
	} catch (...) {}
}

void
RTPApplication::addParticipant(Participant& part)
{
	ParticipantLink* pl = new ParticipantLink(part,NULL);
	if ( NULL == firstPart )
		firstPart = pl;
	else
		lastPart->setNext(pl);
	lastPart = pl;
}

void
RTPApplication::removeParticipant(ParticipantLink* pl)
{
	if ( NULL == pl )
		return;
	if ( pl->getPrev() )
		pl->getPrev()->setNext(pl->getNext());
	if ( pl->getNext() )
		pl->getNext()->setPrev(pl->getPrev());
	delete pl;
}

void
RTPApplication::findCNAME()
{
	// build string username@host_fqn
	std::string username;
	findusername(username);
	
	setSDESItem(SDESItemTypeCNAME, 
		    username + "@" + InetHostAddress().getHostname());
}

#ifdef	CCXX_NAMESPACES
};
#endif

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
