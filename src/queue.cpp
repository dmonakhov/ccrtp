// Copyright (C) 1999-2001 Open Source Telecom Corporation.
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
// RTPQueue class implementation
//
#include "private.h"

#ifdef  CCXX_NAMESPACES
namespace ost {
#endif

static uint32 random32();
static uint16 random16();

static uint16 random16()
{
	uint16 s;
	int fd = open("/dev/urandom",O_RDONLY);
	read(fd,&s,2);
	close(fd);
	return s; 
}

static uint32 random32()
{
	uint32 s;
	int fd = open("/dev/urandom",O_RDONLY);
	read(fd,&s,4);
	close(fd);
	return s; 
}

uint32
RTPQueue::payload_rate[] = {
	8000,   // RTP_PAYLOAD_PCMU = 0,   
	8000,   // RTP_PAYLOAD_1016,       
	8000,   // RTP_PAYLOAD_G726,       
	8000,   // RTP_PAYLOAD_GSM,        
	8000,   // RTP_PAYLOAD_G723,       
	8000,   // RTP_PAYLOAD_DVI4_8000  
	16000,  // RTP_PAYLOAD_DVI4_16000
	8000,   // RTP_PAYLOAD_LPC,        
	8000,   // RTP_PAYLOAD_PCMA,       
	8000,   // RTP_PAYLOAD_G722,       
	44100,  // RTP_PAYLOAD_L16_DUAL,   
	44100,  // RTP_PAYLOAD_L16_MONO,   
	8000,   // RTP_PAYLOAD_QCELP,      
	0,      // unassigned : 13
	90000,  // RTP_PAYLOAD_MPA = 14,	
	8000,   // RTP_PAYLOAD_G728,       
	11025,  // RTP_PAYLOAD_DVI4_11025, 
	22050,  // RTP_PAYLOAD_DVI4_22050, 
	8000,   // RTP_PAYLOAD_G729,       
	0,      // unassigned : 19
	0,      // unassigned : 20
	0,      // unassigned : 21
	0,      // unassigned : 22
	0,      // unassigned : 23
	0,      // unassigned : 24
	90000,  // RTP_PAYLOAD_CELB = 25,  
	90000,  // RTP_PAYLOAD_JPEG,       
	0,      // unassigned : 27
	90000,  // RTP_PAYLOAD_NV = 28,    
	0,      // unassigned : 29
	0,      // unassigned : 30
	90000,  // RTP_PAYLOAD_H261 = 31,  
	90000,  // RTP_PAYLOAD_MPV,	
	90000,  // RTP_PAYLOAD_MP2T,	
	90000   // RTP_PAYLOAD_H263,       
	// from 36 to 71 : unassigned
	// from 72 to 76 : reserved
	// from 77 to 95 : unassigned
};

RTPQueue::RTPQueue(int pri, uint32 size) :
	Thread(pri), MembershipControl(size) 
{
	initQueue(random32());
} 

RTPQueue::RTPQueue(uint32 ssrc, int pri, uint32 size):
	Thread(pri), MembershipControl(size)
{
	initQueue(ssrc);
}

// Initialize everything
void 
RTPQueue::initQueue(uint32 localssrc)
{
	active = false;
	sendcount = 0;
	octetcount = 0;
	current_rate = 80000;                  // assume a default rate
	type_of_service = BEST_EFFORT_SERVICE; // assume a best effort network
	sessionbw = 0;
	sendfirst = sendlast = NULL; 
	recvfirst = recvlast = NULL;
	sendseq = random16(); 	              // random initial sequence number
	sendcc = 0;    // initially, 0 CSRC identifiers follow the fixed header
	segment = 8000; // segment data in packets of no more than 8000 octets
	marked = false;
	complete = true;
	timeout = 8000;   // schedule at 8 ms
	expired = 40000;  // packets unsent will expire after 40 ms
	e2edelay = 15000000; // maximun end to end delay: 15 seconds 
	kitchensize = 0;  // kitchen is octets long

	// assume unicast initially
	sessioncast = CAST_UCAST;
	// the local source is the first contributing source
	sendsources[0] = localssrc;
	// creation time
	gettimeofday(&initial_time,NULL);
	initial_timestamp = random32();
	// this will be an accumulator for the successive cycles of timestamp
	overflow_time.tv_sec = initial_time.tv_sec;
	overflow_time.tv_usec = initial_time.tv_usec;

	// add the local source as the first identified source
	localsrc = &addNewSource(localssrc);
}

RTPQueue::~RTPQueue()
{
	endQueue();
}

void 
RTPQueue::endQueue(void)
{
	// stop running the service thread
	active = false;
	terminate();
	// purge both sending and receiving queues
	purge(RTP_PURGE_BOTH);
}

void 
RTPQueue::purge(rtp_purge_t flag)
{
	if (flag == RTP_PURGE_SEND || flag == RTP_PURGE_BOTH)
	{
		OutgoingRTPPkt *sendnext;
		// flush the sending queue (delete outgoing packets
		// unsent so far)
		sendlock.enterMutex();
 		while ( sendfirst )
 		{
 			sendnext = sendfirst->next;
 			delete sendfirst;
 			sendfirst = sendnext;
 		}
 		sendlock.leaveMutex();
 	}

 	if (flag == RTP_PURGE_RECV || flag == RTP_PURGE_BOTH)
 	{
		IncomingRTPPkt *recnext;
		// flush the reception queue (incoming packets not yet
		// retrieved)
 		recvlock.enterMutex();
 		while( recvfirst )
 		{
 			recnext = recvfirst->next;

			// nullify source specific packet list
			RTPSource &s = MembershipControl::
				getSourceBySSRC(recvfirst->getSSRC());
			s.first = s.last = NULL;


 			delete recvfirst;
 			recvfirst = recnext;
 		}
 		recvlock.leaveMutex();
 	}
}

uint32
RTPQueue::getRate(rtp_payload_t pt) const
{
	I( pt < 72 || pt > 76 );
	if ( pt == RTP_PAYLOAD_EMPTY ) {
		if ( recvfirst )
			return getRate(recvfirst->getPayloadType());
		else
			return 0;
	} else if ( pt < 96 ) {
		return payload_rate[pt];
	} else {
		// HERE IS WHERE NEW DYNAMIC PTs MUST BE DEALT WITH
		switch ( pt ) {
		case RTP_PAYLOAD_INVALID:
			return 0;
		case RTP_PAYLOAD_G726_40:
		case RTP_PAYLOAD_G726_24:
		case RTP_PAYLOAD_G726_16:
		case RTP_PAYLOAD_G729D:
		case RTP_PAYLOAD_G729E:
		case RTP_PAYLOAD_GSM_EFR:
			return 8000;
		case RTP_PAYLOAD_BT656:
		case RTP_PAYLOAD_H263-1998:
		case RTP_PAYLOAD_MP1S:
		case RTP_PAYLOAD_MP2P:
		case RTP_PAYLOAD_BMPEG:
			return 90000;
		case RTP_PAYLOAD_L8:
		case RTP_PAYLOAD_RED:
		case RTP_PAYLOAD_VDVI:
			// FIX: this must be clarified
			return 0;
		default:
			//FIX: exception
			return 0;
		}
	}
}

bool
RTPQueue::isWaiting(const RTPSource &src) const
{
	recvlock.enterMutex();
	bool w;

	if ( src == dummysource )
		w = (recvfirst != NULL);
	else
		w = (src.first != NULL);

	recvlock.leaveMutex();
	return w;
}

bool
RTPQueue::isSending(void) const
{
	if(sendfirst)
		return true;

	return false;
}

uint32
RTPQueue::getCurrentTimestamp(rtp_payload_t pt) const
{
	// translate from current time to timestamp
	timeval now;
	gettimeofday(&now,NULL);

	uint32 result = now.tv_usec - initial_time.tv_usec;
	result *= (current_rate/1000);
	result /= 1000;
	result += (now.tv_sec - initial_time.tv_sec) * current_rate;

	//result -= initial_timestamp;
	return result;
}

microtimeout_t
RTPQueue::getTimeout(void)
{
	struct timeval send, now;
	uint32 rate;
	uint32 rem;

	for(;;)
	{
		// if there is no packet to send, use the default scheduling
		// timeout
		if( !sendfirst )
			return timeout;

		uint32 stamp = sendfirst->getTimestamp();
		stamp -= initial_timestamp;
		rate = getRate(sendfirst->getPayloadType());

		// now we want to get in <code>send</code> when the
		// packet is scheduled to be sent.

		// translate timestamp to timeval
		send.tv_sec = stamp / rate;
		rem = stamp % rate;
		send.tv_usec = 1000000ul * rem / rate;

		// add timevals. Overflow holds the inital time
		// plus the time accumulated through successive
		// overflows of timestamp. See below.
		send.tv_sec += overflow_time.tv_sec;
		send.tv_usec += overflow_time.tv_usec;
		if(send.tv_usec >= 1000000l)
		{
			++send.tv_sec;
			send.tv_usec -= 1000000ul;
		}

		gettimeofday(&now, NULL);

		// Problem: when timestamp overflows, time goes back.
		// We MUST ensure that _send_ is not too lower than
		// _now_, otherwise, we MUST keep how many time was
		// lost because of overflow. We assume that _send_
		// 5000 seconds lower than now suggests timestamp
		// overflow.  (Remember than the 32 bits of the
		// timestamp field are 47722 seconds under a sampling
		// clock of 90000 hz.)  This is not a perfect
		// solution. Disorderedly timestamped packets coming
		// after an overflowed one will be wrongly
		// corrected. Nevertheless, this may only corrupt a
		// handful of those packets every more than 13 hours
		// (if timestamp started from 0).
		if ( now.tv_sec - send.tv_sec > 5000){
			uint32 nsec = (~static_cast<uint32>(0)) / rate;
			uint32 nusec = (~static_cast<uint32>(0)) % rate *
				1000000ul / rate;
			do {
				send.tv_sec += nsec;
				send.tv_usec += nusec;
				if( send.tv_usec >= 1000000l ) {
					++send.tv_sec;
					send.tv_usec -= 1000000ul;
				}

				overflow_time.tv_sec += nsec;
				overflow_time.tv_usec += nusec;
				if( overflow_time.tv_usec >= 1000000l ) {
					++overflow_time.tv_sec;
					overflow_time.tv_usec -= 1000000ul;
				}
			} while ( now.tv_sec - send.tv_sec > 5000 );
		}

		// This tries to solve the aforementioned problem
		// about disordered packets coming after an overflowed
		// one. Now we apply the reverse idea.
		if ( send.tv_usec - now.tv_usec > 20000 ){
			uint32 nsec = (~static_cast<uint32>(0)) / rate;
			uint32 nusec = (~static_cast<uint32>(0)) % rate *
				1000000ul / rate;
			send.tv_sec -= nsec;
			send.tv_usec -= nusec;
			if( send.tv_usec < 0ul ) {
				--send.tv_sec;
				send.tv_usec += 1000000ul;
			}
		}

		// A: This sets a maximum timeout of 1 hour.
		if ( send.tv_sec - now.tv_sec > 3600 ){
			return 3600000000ul;
		}
		int32 diff = (send.tv_sec - now.tv_sec) * 1000000ul;
		diff += (send.tv_usec - now.tv_usec);

		// B: wait <code>diff</code> usecs more before sending
		if ( diff >= 0 ){
			return static_cast<microtimeout_t>(diff);
		}

		// C: the packet must be sent right now
		if ( (diff < 0) &&
		     static_cast<microtimeout_t>(-diff) <= expired ){
			return 0;
		}

		// D: the packet has expired -> delete
		setCancel(cancelDeferred);
		sendlock.enterMutex();
		OutgoingRTPPkt* packet = sendfirst;
		sendfirst = sendfirst->next;
		expireSend(packet);             // new virtual to notify
		delete packet;
		if ( sendfirst )
			sendfirst->prev = NULL;
		else
			sendlast = NULL;
		sendlock.leaveMutex();
		setCancel(cancelImmediate);
	}
	I( false );
	return 0;
}

uint16
RTPQueue::getFirstSequence(const RTPSource &src)
{
	recvlock.enterMutex();

	// get the first packet
	IncomingRTPPkt* packet;
	if ( src == dummysource )
		packet = recvfirst;
 	else
		packet = src.first;

	// get the sequence number of the first packet
	uint16 seq;
	if ( packet )
		seq = packet->getSeqNum();
	else
		seq = 0l;

	recvlock.leaveMutex();
	return seq;
}

uint32
RTPQueue::getFirstTimestamp(const RTPSource &src)
{
	recvlock.enterMutex();

	// get the first packet
	IncomingRTPPkt* packet;
	if ( src == dummysource )
		packet = recvfirst;
 	else
		packet = src.first;

	// get the timestamp of the first packet
	uint32 ts;
	if ( packet )
		ts = packet->getTimestamp();
	else
		ts = 0l;

	recvlock.leaveMutex();
	return ts;
}

void
RTPQueue::setGlobalKitchenDuration(microtimeout_t t)
{


}

void
RTPQueue::setGlobalKitchenSize(uint32 s)
{


}

void
RTPQueue::putPacket(uint32 stamp, rtp_payload_t payload, const unsigned char *data, size_t datalen, bool mark)
{
	if ( !data || !datalen )
		return;

	OutgoingRTPPkt* packet;
	if ( sendcc )
		packet = new OutgoingRTPPkt(sendsources,15,data,datalen);
	else
		packet = new OutgoingRTPPkt(data,datalen);

	packet->setPayloadType(payload);
	packet->setSeqNum(sendseq++);
	packet->setTimestamp(stamp + initial_timestamp);
	packet->setSSRC(localsrc->getID());
	packet->setMarker(mark);

	// insert the packet into the "tail" of the sending queue
	sendlock.enterMutex();
	packet->prev = sendlast;
	if (sendlast)
		sendlast->next = packet;
	else
		sendfirst = packet;
	sendlast = packet;
	sendlock.leaveMutex();
}

size_t
RTPQueue::recvPacket(void)
{
	unsigned char *buffer = new unsigned char[RECVBUFFER_SIZE];
	int32 rtn = readData(buffer, RECVBUFFER_SIZE);
	// get time of arrival
	struct timeval recvtime;
	gettimeofday(&recvtime,NULL);

	if ( rtn < 0 ){
		delete buffer;
		return rtn;
	}

	// build a packet. It will link itself to its source
	IncomingRTPPkt* packet =
		new IncomingRTPPkt(*this,buffer,rtn,recvtime);

	// header validity check
	if ( !packet->isHeaderValid() ) {
		delete packet;
		return 0;
	}

	// plug-in for profile-specific validation and processing
	if ( !gotPacket(packet) )
	{
		delete packet;
		return 0;
	}

	insertRecvPacket(packet);
	return rtn;
}

void
RTPQueue::insertRecvPacket(IncomingRTPPkt* packet)
{
	RTPSource &src = packet->getSource();
	unsigned short seq = packet->getSeqNum();
	setCancel(cancelDeferred);
	recvlock.enterMutex();
	IncomingRTPPkt* list = src.last;
	if ( list && (seq < list->getSeqNum()) ) {
		// a disordered packet, so look for its place
		while ( list && (seq < list->getSeqNum()) ){
			// the packet is duplicated
			if ( seq == list->getSeqNum() ) {
				recvlock.leaveMutex();
				VDL(("Duplicated disordered packet: seqnum %d, SSRC:",seq,src.getID()));
				delete packet;
				setCancel(cancelImmediate);
				return;
			}
			list = list->srcprev;
		}
		if ( !list ) {
			// we have scanned the whole (and non empty)
			// list, so this must be the older (first)
			// packet from this source.

			// insert into the source specific queue
			src.first->srcprev = packet;
			packet->srcnext = src.first;
			// insert into the global queue
			if ( src.first->prev ){
				src.first->prev->next = packet;
				packet->prev = src.first->prev;
			}
			src.first->prev = packet;
			packet->next = src.first;
			src.first = packet;
		} else {
			// (we are in the middle of the source list)
			// insert into the source specific queue
			list->srcnext->srcprev = packet;
			packet->srcnext = list->srcnext;
			// -- insert into the global queue, with the
			// minimum priority compared to packets from
			// other sources
			list->srcnext->prev->next = packet;
			packet->prev = list->srcnext->prev;
			list->srcnext->prev = packet;
			packet->next = list->srcnext;
			// ------
			list->srcnext = packet;
			packet->srcprev = list;

			  // insert into the global queue (giving
			  // priority compared to packets from other sources)
			  //list->next->prev = packet;
			  //packet->next = list->next;
			  //list->next = packet;
			  //packet->prev = list;
		}
	} else {
		// An ordered packet
		if ( !list ) {
			// the only packet in the source specific queue
			src.last = src.first = packet;
			// the last packet in the global queue
			if ( recvlast ) {
				recvlast->next = packet;
				packet->prev = recvlast;
			}
			recvlast = packet;
			if ( !recvfirst )
				recvfirst = packet;
		} else {
			// there are already more packets from this source.
			// this ignores duplicate packets
			if ( list && (seq == list->getSeqNum()) ) {
				VDL(("Duplicated packet: seqnum %d, SSRC:",
				   seq,src.getID()));
				recvlock.leaveMutex();
				delete packet;
				setCancel(cancelImmediate);
				return;
			}
			// the last packet in the source specific queue
			src.last->srcnext = packet;
			packet->srcprev = src.last;
			src.last = packet;
			// the last packet in the global queue
			recvlast->next = packet;
			packet->prev = recvlast;
			recvlast = packet;
		}
	}
	// account the insertion of this packet into the queue
	src.recordInsertion(*packet);
	recvlock.leaveMutex();
	setCancel(cancelImmediate);
}

size_t
RTPQueue::sendPacket(void)
{
	setCancel(cancelDeferred);
	sendlock.enterMutex();
	OutgoingRTPPkt* packet = sendfirst;

	if ( !packet ){
		sendlock.leaveMutex();
		return 0;
	}

	int32 rtn = packet->getPayloadSize();
	if ( rtn )
		rtn = writeData(packet->getRawPacket(),
				packet->getRawPacketSize());

	// unlink the sent packet from the queue and destroy it. Also
	// record the sending.
	if ( rtn > -1 )
	{
		sendfirst = sendfirst->next;
		if ( sendfirst ) {
			sendfirst->prev = NULL;
		} else {
			sendlast = NULL;
		}
		// for general accounting and RTCP SR
		sendcount++;
		octetcount += packet->getPayloadSize();
		delete packet;
	}
	sendlock.leaveMutex();
	setCancel(cancelDeferred);
	return rtn;
}

// FIX: try to merge and organize
IncomingRTPPkt*
RTPQueue::getWaiting(uint32 stamp, const RTPSource& src)
{
	IncomingRTPPkt *result;
	recvlock.enterMutex();

	if ( src != dummysource ) {
		// we will modify the queue of this source
		RTPSource srcm = const_cast<RTPSource&>(src);
		int nold = 0;
		IncomingRTPPkt* p = srcm.first;
		if ( !p ) {
			result = NULL;
			recvlock.leaveMutex();
			return result;
		}
		while ( p && (p->getTimestamp() < stamp) ) {
			nold++;
			p = p->srcnext;
		}
		// to know whether the global queue gets empty
		bool noempty = false;
		for ( int i = 0; i < nold; i++) {
			IncomingRTPPkt * packet = srcm.first;
			srcm.first = srcm.first->srcnext;
			// unlink from the global queue
			noempty = false;
			if ( packet->prev ){
				noempty = true;
				packet->prev->next = packet->next;
			} if ( packet->next ) {
				noempty = true;
				packet->next->prev = packet->prev;
			}
			// now, delete it
			expireRecv(packet);  // notify packet discard
			delete packet;
		}
		// return the packet, if found
		if ( !srcm.first ) {
			// threre are no more packets from this source
			srcm.last = NULL;
			if ( !noempty )
				recvfirst = recvlast = NULL;
			result = NULL;
		} else if ( srcm.first->getTimestamp() > stamp ) {
			// threre are only newer packets from this source
			srcm.first->srcprev = NULL;
			result = NULL;
		} else {
			// (src.first->getTimestamp == stamp) is true
			result = srcm.first;
			// unlink the selected packet from the global queue
			if ( srcm.first->prev )
				srcm.first->prev->next = srcm.first->next;
			else
				recvfirst = srcm.first->next;
			if ( srcm.first->next )
				srcm.first->next->prev = srcm.first->prev;
			else
				recvlast = srcm.first->prev;
			// unlink the selected packet from the source queue
			srcm.first = srcm.first->srcnext;
			if ( srcm.first )
				srcm.first->prev = NULL;
			else
				srcm.last = NULL;
			// result.first->prev = result.first->next = result.first->srcprev = result.first->srcnext = NULL;
		}
	} else {
		// first, delete all older packets. The while loop
		// down here counts how many older packets are there;
		// then the for loop deletes them and advances p till
		// the first non older packet.
		int nold = 0;
		IncomingRTPPkt* p = recvfirst;
		while ( p && (p->getTimestamp() < stamp) ){
			nold++;
			p = p->next;
		}
		for (int i = 0; i < nold; i++) {
			IncomingRTPPkt* packet = recvfirst;
			recvfirst = recvfirst->next;
			// unlink the packet from the queue of its source
			RTPSource &src = packet->getSource();
			src.first = packet->srcnext;
			if ( packet->srcnext )
				packet->srcnext->srcprev = NULL;
			else
				src.last = NULL;
			// now, delete it
			expireRecv(packet);  // notify packet discard
			delete packet;
		}

		// return the packet, if found
		if ( !recvfirst ) {
			// there are no more packets in the queue
			recvlast = NULL;
			result = NULL;
		} else if ( recvfirst->getTimestamp() > stamp ) {
			// there are only newer packets in the queue
			p->prev = NULL;
			result = NULL;
		} else {
			// (recvfirst->getTimestamp() == stamp) is true
			result = recvfirst;
			// unlink the selected packet from the global queue
			recvfirst = recvfirst->next;
			if ( recvfirst )
				recvfirst->prev = NULL;
			else
				recvlast = NULL;
			// unlink the selected packet from the queue
			// of its source
			RTPSource &src = result->getSource();
			src.first = result->srcnext;
			if ( src.first )
				src.first->srcprev = NULL;
			else
				src.last = NULL;
			// result->prev = result->next = result->srcprev = result->srcnext = NULL;
		}
	}
	recvlock.leaveMutex();
	return result;
}

size_t
RTPQueue::setPartial(uint32 stamp, unsigned char *data, size_t offset, size_t max)
{
	sendlock.enterMutex();
	OutgoingRTPPkt* packet = sendfirst;
	while ( packet )
	{
		uint32 pstamp = packet->getTimestamp();
		if ( pstamp > stamp )
			packet = NULL;
		if ( pstamp >= stamp )
			break;

		packet = packet->next;
	}
	if ( !packet )
	{
		sendlock.leaveMutex();
		return 0;
	}

	if ( offset >= packet->getPayloadSize() )
		return 0;

	if ( max > packet->getPayloadSize() - offset )
		max = packet->getPayloadSize() - offset;

	memcpy(const_cast<unsigned char*>(packet->getPayload() + offset),
	       data, max);
	sendlock.leaveMutex();
	return max;
}

size_t
RTPQueue::getPartial(uint32 stamp, unsigned char *data, size_t offset, size_t max)
{
	IncomingRTPPkt* packet = getWaiting(stamp);

	if ( !packet )
		return 0;

	if ( offset >= packet->getPayloadSize() )
		return 0;

	if ( max > packet->getPayloadSize() - offset )
		max = packet->getPayloadSize() - offset;

	memcpy(data, packet->getPayload() + offset, max);
	return max;
}

size_t
RTPQueue::getPacket(uint32 stamp, unsigned char *data, size_t max,
		    const RTPSource &src)
{
	if ( !data )
		return 0;

	IncomingRTPPkt* packet;
	unsigned count = 0;
	complete = true;

	if ( NULL != (packet = getWaiting(stamp,src)) )
	{
		size_t len = packet->getPayloadSize();
		if ( max < len )
			len = max;

		memcpy(data, packet->getPayload(), len);
		marked = packet->getHeader()->marker;

		// get the source specific expected next sequence number
		RTPSource &src = packet->getSource();

		if ( src.getExpectedSeqNum() != packet->getSeqNum() )
			complete = false;

		src.setExpectedSeqNum(packet->getSeqNum() + 1 );

		delete packet;
		count += len;
		data += len;
		max -= len;
		// FIX:
		//if ( max < 1 )
		//	break;
	}
	return count;
}

const RTPData &
RTPQueue::getCookedPacket(const RTPSource &src)
{
	RTPData *data = NULL;
	recvlock.enterMutex();

	IncomingRTPPkt* packet;
	if ( src == dummysource )
		packet = recvfirst;
	else
		packet = src.first;

	if ( packet ) {
		RTPSource &src = packet->source;
		if ( src.getCurrentKitchenSize() >= src.getKitchenSize() ) {
			unsigned char p[8192];
			VDL(("Get Cooked"));
			IncomingRTPPkt* pkt = getWaiting(getFirstTimestamp());
			marked = pkt->getHeader()->marker;
			RTPSource &src = pkt->getSource();
			if ( src.getExpectedSeqNum() != packet->getSeqNum() )
				complete = false;
			data = new RTPData(*pkt);
		}
	}
	recvlock.leaveMutex();
	return *data;
}

rtp_payload_t
RTPQueue::getPayloadType(const RTPSource &src) const
{
	recvlock.enterMutex();

	// get first packet
	IncomingRTPPkt* packet;
	if ( src == dummysource )
		packet = recvfirst;
	else
		packet = src.first;

	// get payload type of the packet
	rtp_payload_t pt;
	if ( packet )
		pt = packet->getPayloadType();
	else
		pt = RTP_PAYLOAD_INVALID;

	recvlock.leaveMutex();
	return pt;
}

rtp_payload_t
RTPQueue::getPayloadType(uint32 stamp, const RTPSource &src)
{
	rtp_payload_t result;

	recvlock.enterMutex();
  	IncomingRTPPkt* packet;

	if ( src == dummysource ) {
		packet = recvfirst;
		while ( packet && (packet->getTimestamp() < stamp) )
			packet = packet->next;
	} else {
		packet = src.first;
		while ( packet && (packet->getTimestamp() < stamp) )
			packet = packet->srcnext;
	}

	// get the payload type of the packet, if found
	if ( !packet || (packet && (packet->getTimestamp() > stamp)) ) {
		result = RTP_PAYLOAD_INVALID;
	} else {
		// (packet->getTimestamp() == stamp) is true
		result = packet->getPayloadType();
	} 
	recvlock.leaveMutex();
	return result;
}

void
RTPQueue::run(void)
{
	microtimeout_t wait = 0;

	while ( active ) 
	{
		if ( !wait ){
			wait = getTimeout(); 
		}
		RTCPService(wait);
		RTPService(wait);
	}

	bye(NULL);
	sleep(~0);
}

void
RTPQueue::RTPService(microtimeout_t &wait)
{
	if ( !wait ) {
		if ( sendPacket() < 0 )
			wait = timeout;			
		timerTick();
	} else {
		if ( isPendingData(wait) )
			if ( recvPacket() < 0 )
				// FIX: Exception
				return ;
		wait = 0;
	}
}

#ifdef  CCXX_NAMESPACES
};
#endif

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */

