// Copyright (C) 1999-2002 Open Source Telecom Corporation.
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

#include <cc++/digest.h>
#include "private.h"
#include <ccrtp/queuebase.h>
#include <ccrtp/ioqueue.h>

#ifdef  CCXX_NAMESPACES
namespace ost {
#endif

static uint32 MD5BasedRandom32();

static uint32 MD5BasedRandom32()
{
	// This is the input to the MD5 algorithm.
	union {
		uint8 array[1];
		struct {
			timeval time;
			uint32 address;
			uint8 cname[10];
		} data;
	} message;

	// the output from the MD5 algorithm will be put here.
	union {
		uint32 buf32[4];
		uint8 buf8[16];
	} digest;

	gettimeofday(&(message.data.time),NULL);
	message.array[0] =
		static_cast<uint8>(message.data.time.tv_sec * 
				   message.data.time.tv_usec);
	message.data.address = (uint32)&message;
	memcpy(message.data.cname,
	       defaultApplication().getSDESItem(SDESItemTypeCNAME).c_str(),10);

	// compute MD5.
	MD5Digest md5;
	md5.putDigest(reinterpret_cast<unsigned char*>(message.array),
		      sizeof(message));
	md5.getDigest(reinterpret_cast<unsigned char*>(digest.buf8));
	
	// Get result as xor of the four 32-bit words from the MD5 algorithm.
	uint32 result = 0;
	for ( int i = 0; i < 4; i ++ )
		result ^= digest.buf32[i];
	return result;		
}

uint32 random32()
{
	// If /dev/urandom fails, default to the MD5 based algorithm
	// given in the RTP specification.
	uint32 number;
	bool success = true;
	int fd = open("/dev/urandom",O_RDONLY);
        if (fd == -1) {
		success = false;
        } else {
		if ( read(fd,&number,sizeof(number)) != sizeof(number) ) {
			success = false;
		}
	}
        close(fd);
	if ( !success )
		number = MD5BasedRandom32();
        return number;
}

uint16 random16()
{
	uint32 r32 = random32();
	uint16 r16 = r32 & (r32 >> 16);
	return r16;
}

RTPQueueBase::RTPQueueBase(uint32 *ssrc)
{
	if ( NULL == ssrc )
		setLocalSSRC(random32());
	else
		setLocalSSRC(*ssrc);
	// assume a default rate and payload type.
	setPayloadFormat(StaticPayloadFormat(sptPCMU));
	// queue/session creation time
	gettimeofday(&initialTime,NULL);
}

const uint32 RTPDataQueue::defaultSessionBw = 64000;

RTPDataQueue::RTPDataQueue(uint32 size)	: 
	IncomingDataQueue(size), OutgoingDataQueue()
{
	initQueue();
} 

RTPDataQueue::RTPDataQueue(uint32* ssrc, uint32 size):
	RTPQueueBase(ssrc),
	IncomingDataQueue(size), OutgoingDataQueue(), timeclock()
{
	initQueue();
}

// Initialize everything
void 
RTPDataQueue::initQueue()
{
	dataServiceActive = false;
	typeOfService = tosBestEffort; // assume a best effort network
	sessionBw = 0;
}

void 
RTPDataQueue::endQueue(void)
{
	// stop executing the data service.
	dataServiceActive = false;

	// purge both sending and receiving queues.
	try {
		purgeOutgoingQueue();
		purgeIncomingQueue();
	} catch (...) { }
}

uint32
RTPDataQueue::getCurrentTimestamp() const
{
	// translate from current time to timestamp
	timeval now;
	gettimeofday(&now,NULL);

	int32 result = now.tv_usec - getInitialTime().tv_usec;
	result *= (getCurrentRTPClockRate()/1000);
	result /= 1000;
	result += (now.tv_sec - getInitialTime().tv_sec) * 
		getCurrentRTPClockRate();

	//result -= initialTimestamp;
	return result;
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

