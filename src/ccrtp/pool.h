// Copyright (C) 2001, 2002 Federico Montesino Pouzols <fedemp@altern.org>
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
 * @file pool.h
 * @short Pools of RTP sessions.
 **/

#ifndef CCXX_RTP_POOL_H
#define CCXX_RTP_POOL_H

#include <list>
#include <ccrtp/rtp.h>

#ifdef  CCXX_NAMESPACES
namespace ost {
using std::list;
#endif

typedef TRTPSessionBase<> RTPSessionBase;

class RTPSessionBaseHandler
{
public:
	size_t
	takeInDataPacket(RTPSessionBase& s)
	{ return s.takeInDataPacket(); }
	
	size_t
	dispatchDataPacket(RTPSessionBase& s)
	{ return s.dispatchDataPacket(); }
	
	void
	controlReceptionService(RTPSessionBase& s)
	{ s.controlReceptionService(); }
	
	void
	controlTransmissionService(RTPSessionBase& s)
	{ s.controlTransmissionService(); }
	
	inline SOCKET getDataRecvSocket(RTPSessionBase& s) const
	{ return s.getDataRecvSocket(); }
	
	inline SOCKET getControlRecvSocket(RTPSessionBase& s) const
	{ return s.getControlRecvSocket(); }
};

/**
 * This class is a base class for classes that define a group of RTP
 * sessions that will be served by one or more execution
 * threads. Derived classes are responsible for serving each RTP
 * session with a thread at least.
 *
 * In order to use the RTP session "pool" you just have to build
 * RTPSessionBase objects for each RTP session (instead of RTPSession
 * objects). Then, add the RTPSessionBase objects to an RTP session
 * "pool" and call startRunning() method of the session pool.
 *
 * @author Federico Montesino Pouzols <fedemp@altern.org>
 **/
class CCXX_CLASS_EXPORT RTPSessionPool: public RTPSessionBaseHandler
{
public:
	RTPSessionPool();

	inline virtual ~RTPSessionPool()
	{ }

	bool
	addSession(RTPSessionBase& session);

	bool 
	removeSession(RTPSessionBase& session);

	size_t
	getPoolLength() const;

	virtual void startRunning() = 0;

	inline bool isActive()
	{ return poolActive; }

protected:
	inline void setActive()
	{ poolActive = true; }
	
	inline timeval getPoolTimeout()
	{ return poolTimeout; }

	inline void setPoolTimeout(int sec, int usec)
	{ poolTimeout.tv_sec = sec; poolTimeout.tv_usec = usec; }

	inline void setPoolTimeout(struct timeval to)
	{ poolTimeout = to; }

	std::list<RTPSessionBase*> sessionList;
	typedef std::list<RTPSessionBase*>::iterator PoolIterator;

#ifndef WIN32
	fd_set recvSocketSet;
	SOCKET highestSocket;  // highest socket number + 1
#endif

private:
	mutable ThreadLock poolLock;
	timeval poolTimeout;
	mutable bool poolActive;
};


class CCXX_CLASS_EXPORT SingleRTPSessionPool : 
		public RTPSessionPool,
		public Thread
{
public:
	/**
	 * @param pri optional thread priority value.
	 **/
	SingleRTPSessionPool(int pri = 0) : 
		RTPSessionPool(),
		Thread(pri)
	{ }
	
	~SingleRTPSessionPool()
	{ }

	void startRunning()
	{ setActive(); Thread::start(); }

protected:
	/**
	 * Runnable method for the thread. This thread serves all the
	 * added RTP sessions.
	 */
	void run();
};

#ifdef  CCXX_NAMESPACES
};
#endif

#endif //CCXX_RTP_POOL_H

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
