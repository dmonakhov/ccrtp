// Copyright (C) 2001 Federico Montesino <p5087@quintero.fie.us.es>
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

//
// MembershipControl class implementation
//
#include "private.h"

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

const RTPSource MembershipControl::dummysource(0);

MembershipControl::MembershipControl(uint32 initial_size):
	SOURCE_BUCKETS(initial_size), 
	sources( new RTPSource* [SOURCE_BUCKETS] ),
	first(NULL), last(NULL)
{
	for ( uint32 i = 0; i < SOURCE_BUCKETS; i++ )
		sources[i] = NULL;
}

MembershipControl::~MembershipControl()
{
	endMembers();
}

void 
MembershipControl::endMembers()
{
	RTPSource *s, *prev;
	for( s = first; s; ) {
		prev = s;
		s = s->next;
		try {
			delete prev;
		} catch (...) {}
	}
	
	try {
		delete [] sources;
	} catch (...) {}
}

RTPSource & 
MembershipControl::addNewSource(uint32 ssrc)
{
	RTPSource *newsource = new RTPSource(ssrc);

	// FIX: Use a nicer hashing. 
	uint32 index = (ssrc + (ssrc >> 24)) % SOURCE_BUCKETS; 
	RTPSource *pos = sources[index];
	// first, insert into the hash table
	if ( pos == NULL ) {
		// there was no one in this collision list as yet
		sources[index] = newsource;
	} else {
		bool inserted = false;
		// The collision list is ordered ascendently
		RTPSource *prevpos = NULL;
		while ( pos != NULL ) {
			if ( pos->ssrc == ssrc ) {
				// FIX: solve collision: RAISE
				// EXCEPTION or call plug-in
				break;
			} else if ( pos->ssrc > ssrc ) {
				// insert
				if ( prevpos )
					prevpos->nextcollis = newsource;
				newsource->nextcollis = pos;
				sources[index] = newsource;
				inserted = true;
				break;
			} else {
				// keep on searching
				prevpos = pos;
				pos = pos->nextcollis;
			}
		}
		// insert last in this list
		if ( !inserted) {
			newsource->nextcollis = NULL;
			prevpos->nextcollis = newsource;
		}				
	}
	// then, insert into the list of sources
	if ( first ) {
		last = last->next = newsource;
	} else {
		first = last = newsource;
	}
	increaseMembersCount();
	newsource->setState(RTPSOURCE_STATE_PREVALID);
	return *newsource;
}

RTPSource &
MembershipControl::getSourceBySSRC(uint32 ssrc, bool create) 
{  
	RTPSource *result = sources[ (ssrc + (ssrc >> 24)) % SOURCE_BUCKETS ]; 
	
	while ( result != NULL ) {
		if ( result->ssrc == ssrc ) {
			// we found it!
			break;              
		} else if ( result->ssrc > ssrc ) {
			// it isn't here
			if ( create )
				result = &addNewSource(ssrc);  
			else
				result = const_cast<RTPSource *>(&dummysource);
			break;
		} else {
			// keep on searching
			result = result->nextcollis;
		}
	}

	if ( result == NULL && create ){
		if ( create )
			result = &addNewSource(ssrc);
		else
			result = const_cast<RTPSource *>(&dummysource);
	}
	return *result;
}

bool
MembershipControl::BYESource(uint32 ssrc) 
{  
	bool found = false;
	// If the source identified by ssrc is in the table, mark it
	// as leaving the session. If it was not, do nothing.
	RTPSource &src = getSourceBySSRC(ssrc);
	if ( src != dummysource ) {
		found = true;
		src.setState(RTPSOURCE_STATE_SAYINGBYE);
		decreaseMembersCount();
	}
	return found;
}

bool
MembershipControl::removeSource(uint32 ssrc) 
{  
	bool removed = false;
	RTPSource* old = NULL, 
		* s = sources[ (ssrc + (ssrc >> 24)) % SOURCE_BUCKETS ]; 
	while ( s != NULL ){
		if ( s->ssrc == ssrc ) {
			// we found it
			if ( old )
				old->nextcollis = s->nextcollis;
			if ( s->prev )
				s->prev->next = s->next;
			if ( s->next )
				s->next->prev = s->prev;
			decreaseMembersCount();
			if ( s->isSender() )
				decreaseSendersCount();
			delete s;
			removed = true;
			break;              
		} else if ( s->ssrc > ssrc ) {
			// it wasn't here
			break;
		} else {
			// keep on searching
			old = s;
			s = s->nextcollis;
		}
	}
	return removed;
}

const RTPSource&
MembershipControl::getFirstPlayer()
{
	playerslock.enterMutex();

	playerslock.leaveMutex();
	return dummysource;
}

const RTPSource&
MembershipControl::getLastPlayer()
{
	playerslock.enterMutex();

	playerslock.leaveMutex();
	return dummysource;
}

const RTPSource&
MembershipControl::getNextPlayer()
{
	playerslock.enterMutex();

	playerslock.leaveMutex();
	return dummysource;
}

const RTPSource&
MembershipControl::getCurrentPlayer()
{
	playerslock.enterMutex();

	playerslock.leaveMutex();
	return dummysource;
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
