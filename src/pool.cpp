// Copyright (C) 2000, 2001 Federico Montesino Pouzols <fedemp@altern.org>
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
// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however    
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.    
//
// This exception applies only to the code released under the name GNU
// ccRTP.  If you copy code from other releases into a copy of GNU
// ccRTP, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
//
// If you write modifications of your own for GNU ccRTP, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.
//

#include "private.h"
#include <ccrtp/pool.h>

#include <algorithm>

#ifdef  CCXX_NAMESPACES
namespace ost {
using std::list;
#endif

RTPSessionPool::RTPSessionPool()
{
#ifndef WIN32
	highestSocket = 0;
	setPoolTimeout(0,3000);
	FD_ZERO(&recvSocketSet);
#endif
}

bool
RTPSessionPool::addSession(RTPSessionBase& session)
{
#ifndef WIN32
	bool result = false;
	poolLock.writeLock();
	// insert in list. 
	SOCKET s = getDataRecvSocket(session);
	if ( sessionList.end() == 
	     find(sessionList.begin(),sessionList.end(),&session) ) {
		result = true;
		sessionList.push_back(&session);
		if ( s > highestSocket + 1 )
			highestSocket = s + 1;
		FD_SET(s,&recvSocketSet);	
	} else {
		result = false;
	}
	poolLock.unlock();
	return result;
#else
	return false;
#endif
}

bool
RTPSessionPool::removeSession(RTPSessionBase& session)
{
#ifndef WIN32
	bool result = false;
	poolLock.writeLock();
	// remove from list. 
	SOCKET s = getDataRecvSocket(session);
	PoolIterator i;
	if ( sessionList.end() != 
	     (i = find(sessionList.begin(),sessionList.end(),&session)) ) { 
		sessionList.erase(i);
		result = true;
		FD_CLR(s,&recvSocketSet);
	} else {
		result = false;
	}
	poolLock.unlock();
	return result;
#else
	return false;
#endif
}

size_t
RTPSessionPool::getPoolLength() const
{
#ifndef WIN32
	size_t result;
	poolLock.readLock();
	result = sessionList.size();
	poolLock.unlock();
	return result;
#else
	return 0;
#endif
}

void
SingleRTPSessionPool::run()
{
#ifndef WIN32
	SOCKET so;
	timeval timeout = getPoolTimeout();

	while ( isActive() ) {
		PoolIterator i = sessionList.begin();
		while ( i != sessionList.end() ) {
			controlReceptionService(**i);
			controlTransmissionService(**i);
			i++;
		}

		int n = select(highestSocket,&recvSocketSet,NULL,NULL,
			       &timeout);
		
		i = sessionList.begin();
		while ( (i != sessionList.end()) ) {
			so = getDataRecvSocket(**i);
			if ( FD_ISSET(so,&recvSocketSet) && (n-- > 0) ) {
				takeInDataPacket(**i);
			}
			dispatchDataPacket(**i);
			i++;
		}
	}
#endif // ndef WIN32
}

#if defined(_MSC_VER) && _MSC_VER >= 1300
SingleThreadRTPSession<DualRTPUDPIPv4Channel,DualRTPUDPIPv4Channel,AVPQueue>::SingleThreadRTPSession<DualRTPUDPIPv4Channel,DualRTPUDPIPv4Channel,AVPQueue>(
		const InetHostAddress& ia, 
			       tpport_t dataPort, 
			       tpport_t controlPort,
			       int pri,
			       uint32 memberssize,
			       RTPApplication& app):
		Thread(pri),
		TRTPSessionBase<RTPDataChannel,RTCPChannel,ServiceQueue>
	(ia,dataPort,controlPort,memberssize,app)
	{ }

SingleThreadRTPSession<DualRTPUDPIPv4Channel,DualRTPUDPIPv4Channel,AVPQueue>::SingleThreadRTPSession<DualRTPUDPIPv4Channel,DualRTPUDPIPv4Channel,AVPQueue>(
	const InetMcastAddress& ia,
			       tpport_t dataPort, 
			       tpport_t controlPort, 
			       int pri,
			       uint32 memberssize,
			       RTPApplication& app,
			       uint32 iface): 
		Thread(pri),
		TRTPSessionBase<RTPDataChannel,RTCPChannel,ServiceQueue>
	(ia,dataPort,controlPort,memberssize,app,iface)
	{ }

void SingleThreadRTPSession<DualRTPUDPIPv4Channel,DualRTPUDPIPv4Channel,AVPQueue>::startRunning()
{
	enableStack();
	Thread::start();
}

bool SingleThreadRTPSession<DualRTPUDPIPv4Channel,DualRTPUDPIPv4Channel,AVPQueue>::isPendingData(microtimeout_t timeout)
{
	return TRTPSessionBase<RTPDataChannel,RTCPChannel,ServiceQueue>::isPendingData(timeout);
}

void SingleThreadRTPSession<DualRTPUDPIPv4Channel,DualRTPUDPIPv4Channel,AVPQueue>::timerTick(void)
{
}

void SingleThreadRTPSession<DualRTPUDPIPv4Channel,DualRTPUDPIPv4Channel,AVPQueue>::run(void)
{
		microtimeout_t timeout = 0;
		while ( ServiceQueue::isActive() ) {
			if ( timeout < 1000 ){ // !(timeout/1000)
				timeout = getSchedulingTimeout();
			}
			setCancel(cancelDeferred);
			controlReceptionService();
			controlTransmissionService();
			setCancel(cancelImmediate);
			microtimeout_t maxWait = 
				timeval2microtimeout(getRTCPCheckInterval());
			// make sure the scheduling timeout is
			// <= the check interval for RTCP
			// packets
			timeout = (timeout > maxWait)? maxWait : timeout;
			if ( timeout < 1000 ) { // !(timeout/1000)
				setCancel(cancelDeferred);
				dispatchDataPacket();
				setCancel(cancelImmediate);
				timerTick();
			} else {
				if ( isPendingData(timeout/1000) ) {
					setCancel(cancelDeferred);
					takeInDataPacket();
					setCancel(cancelImmediate);
				}
				timeout = 0;
			}
		}
		dispatchBYE("GNU ccRTP stack finishing.");
		sleep(~0);
}

#endif

#ifdef  CCXX_NAMESPACES
}
#endif

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
