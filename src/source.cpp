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
// RTPSource class implementation
//
#include "private.h"

#ifdef	__NAMESPACES__
namespace ost {
#endif

const char *const RTPSource::unknown("unknown");

RTPSource::RTPSource(uint32 ssrc):
	ssrc(ssrc),
	packet_count(0), kitchensize(0), valid(false), active_sender(false),
	prev(NULL), next(NULL), first(NULL), last(NULL), nextcollis(NULL),
	sender_info(new SenderInfo()), receiver_info(new ReceiverInfo()),
	sdes_items(NULL)
{
	sdes_items = new char* [RTCP_SDES_ITEM_H323_CADDR];
	for (int i = 0; i < RTCP_SDES_ITEM_H323_CADDR; i++)
		sdes_items[i] = NULL;

	initial_timestamp = 0;
	last_time.tv_sec = last_time.tv_usec = 0;
	expectedseqnum = 0; 
	flag = false;
}

RTPSource::~RTPSource()
{
	endSource();
}

bool RTPSource::getHello(void)
{
	if(flag)
		return false;

	flag = true;
	return true;
}

bool RTPSource::getGoodbye(void)
{
	if(!flag)
		return false;

	flag = false;
	return true;
}	

void
RTPSource::endSource()
{
	valid = active_sender = false;
	prev = next = NULL;
	first = last = NULL;
	for (int i = 0; i < RTCP_SDES_ITEM_H323_CADDR; i++)
		if ( sdes_items[i] != unknown )
			try {
				delete sdes_items[i];
			} catch (...) { };
	try {
		delete sdes_items;
	} catch (...) { };
	
	flag = false;
}

RTPSource::RTPSource(const RTPSource &origin)
{
	// for now, it makes no sense
	// TODO: assign a lot of things
}

uint32 
RTPSource::getRate() const
{

}

void
RTPSource::recordReception(IncomingRTPPkt& p)
{
	packet_count++;
	setExpectedSeqNum(p.getSeqNum() + 1);
	
	if ( !isSender() )
		setSender(true);

	if ( packet_count == 1 ) {
		// ooops, it's the first packet from this source
		setInitialTimestamp(p.getRawTimestamp());
	}
	// we record the last time a packet from this source was
	// received, this has statistical interest and is needed to
	// time out old senders that are no sending any longer.
	last_time = p.getRecvTimestamp();
}

void
RTPSource::recordInsertion(IncomingRTPPkt &pkt)
{
	setCurrentKitchenSize(getCurrentKitchenSize() - pkt.getPayloadSize());
}

void
RTPSource::recordExtraction(IncomingRTPPkt &pkt)
{
	setCurrentKitchenSize(getCurrentKitchenSize() - pkt.getPayloadSize());
}

const char* const
RTPSource::getSDESItem(sdes_item_type_t type) const
{
	return sdes_items[type - 1];
}

void 
RTPSource::setSDESItem(sdes_item_type_t type, const char *const value)
{
	if ( type > RTCP_SDES_ITEM_END && type <= RTCP_SDES_ITEM_H323_CADDR ) {
		// the first item (END)
		type = static_cast<sdes_item_type_t>
			(static_cast<int>(type - 1)); 
		delete [] sdes_items[type];
		sdes_items[type] = new char[strlen(value) + 1];
		memcpy(sdes_items[type],value,strlen(value) + 1);
	} else 
		I ( false );
	// FIX: exception
}

#ifdef	__NAMESPACES__
};
#endif

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
