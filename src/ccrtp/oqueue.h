// Copyright (C) 2001,2002 Federico Montesino Pouzols <fedemp@altern.org>.
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
 * @file oqueue.h 
 *
 * @short Generic RTP output queues.
 **/

#ifndef	CCXX_RTP_OQUEUE_H_
#define CCXX_RTP_OQUEUE_H_

#include <ccrtp/queuebase.h>

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

/**
 * @defgroup oqueue Generic RTP output queues.
 * @{
 **/

/**
 * @class DestinationListHandler
 *
 * This class handles a list of destination addresses. Stores network
 * addresses as InetAddress objects.
 *
 * @author Federico Montesino Pouzols <fedemp@altern.org>
 **/
class CCXX_CLASS_EXPORT DestinationListHandler
{
protected:
	struct TransportAddress;

public:
	DestinationListHandler();

	~DestinationListHandler();

	/**
	 * Get whether there is only a destination in the list.
	 **/
	inline bool isSingleDestination() const
	{ return (destinationCounter == 1); }

	inline TransportAddress* getFirstDestination() const
	{ return firstDestination; }

	inline void lockDestinationList() const
	{ destinationLock.readLock(); }

	inline void unlockDestinationList() const
	{ destinationLock.unlock(); }

protected:
	inline void writeLockDestinationList() const
	{ destinationLock.writeLock(); }

	/**
	 * Locks the object before modifying it.
	 **/
	bool
	addDestinationToList(const InetAddress& ia, tpport_t data, 
			     tpport_t control);
	
	/**
	 * Locks the object before modifying it.
	 **/
	bool removeDestinationFromList(const InetAddress& ia,
				       tpport_t dataPort,
				       tpport_t controlPort);

	struct TransportAddress
	{ 
		TransportAddress(InetAddress na, tpport_t dtp, tpport_t ctp) :
			networkAddress(na), dataTransportPort(dtp), 
			controlTransportPort(ctp), next(NULL)
		{  }

		inline TransportAddress* getNext()
		{ return next; }

		inline void setNext(TransportAddress* nc)
		{ next = nc; }

		inline const InetAddress& getNetworkAddress() const
		{ return networkAddress; }

		inline tpport_t getDataTransportPort() const
		{ return dataTransportPort; }

		inline tpport_t getControlTransportPort() const
		{ return controlTransportPort; }

		InetAddress networkAddress;
		tpport_t dataTransportPort, controlTransportPort;
		TransportAddress* next;
	};

private:
	uint8 destinationCounter;
	TransportAddress* firstDestination, * lastDestination;
	mutable ThreadLock destinationLock;
};

/**
 * @class OutgoingDataQueue
 *
 * A generic outgoing RTP data queue supporting multiple destinations.
 *
 * @todo Add CSRC interface.
 * @author Federico Montesino Pouzols <fedemp@altern.org>
 **/
class CCXX_CLASS_EXPORT OutgoingDataQueue:
	public OutgoingDataQueueBase,
	protected DestinationListHandler
{
public:
	bool
	addDestination(const InetHostAddress& ia, 
		       tpport_t dataPort = DefaultRTPDataPort,
		       tpport_t controlPort = 0);

	bool
	addDestination(const InetMcastAddress& ia, 
		       tpport_t dataPort = DefaultRTPDataPort,
		       tpport_t controlPort = 0);

	bool
	forgetDestination(const InetHostAddress& ia, 
			  tpport_t dataPort = DefaultRTPDataPort,
			  tpport_t controlPort = 0);

	bool
	forgetDestination(const InetMcastAddress& ia, 
			  tpport_t dataPort = DefaultRTPDataPort,
			  tpport_t controlPort = 0);

 	/**
 	 * Determine if outgoing packets are waiting to send.
 	 *
 	 * @return true if there are packets waiting to be send.
 	 */
 	bool 
	isSending() const;

	/**
	 * This is used to create a data packet in the send queue.  
	 * Sometimes a "NULL" or empty packet will be used instead, and
	 * these are known as "silent" packets.  "Silent" packets are
	 * used simply to "push" the scheduler along more accurately
	 * by giving the appearence that a next packet is waiting to
	 * be sent and to provide a valid timestamp for that packet.
	 *
	 * @param stamp Timestamp for expected send time of packet.
	 * @param data Value or NULL if special "silent" packet.
	 * @param len May be 0 to indicate a default by payload type.
	 **/
	void
	putData(uint32 stamp, const unsigned char* data = NULL, size_t len = 0);

	/**
	 * Set marker bit for the packet in which the next data
	 * provided will be send. When transmitting audio, should be
	 * set for the first packet of a talk spurt. When transmitting
	 * video, should be set for the last packet for a video frame.
	 *
	 * @param mark Marker bit value for next packet.
	 **/
	void setMark(bool mark)
	{ sendInfo.marked = mark; }

	/**
	 * Get wheter the mark bit will be set in the next packet.
	 **/
	inline bool getMark() const
	{ return sendInfo.marked; }

	/**
	 * Set partial data for an already queued packet.  This is often
	 * used for multichannel data.
	 *
	 * @param timestamp Timestamp of packet.
	 * @param data Buffer to copy from.
	 * @param offset Offset to copy from.
	 * @param max Maximum data size.
	 * @return Number of packet data bytes set.
	 **/
	size_t 
	setPartial(uint32 timestamp, unsigned char* data, size_t offset, size_t max);

	inline microtimeout_t
	getDefaultSchedulingTimeout() const
	{ return defaultSchedulingTimeout; }

	/**
	 * Set the default scheduling timeout to use when no data
	 * packets are waiting to be sent.
	 *
	 * @param to timeout in milliseconds.
	 **/
	inline void 
	setSchedulingTimeout(microtimeout_t to)
	{ schedulingTimeout = to; }

	inline microtimeout_t
	getDefaultExpireTimeout() const
	{ return defaultExpireTimeout; }

	/**
	 * Set the "expired" timer for expiring packets pending in
	 * the send queue which have gone unsent and are already
	 * "too late" to be sent now.
	 *
	 * @param to timeout to expire unsent packets in milliseconds.
	 **/
	inline void 
	setExpireTimeout(microtimeout_t to)
	{ expireTimeout = to; }

	inline microtimeout_t getExpireTimeout() const
	{ return expireTimeout; }

	/**
	 * Get the total number of packets sent so far
	 *
	 * @return total number of packets sent
	 */
	inline uint32
	getSendPacketCount() const
	{ return sendInfo.packetCount; }

	/**
	 * Get the total number of octets (payload only) sent so far.
	 *
	 * @return total number of payload octets sent in RTP packets.
	 **/
	inline uint32
	getSendOctetCount() const
	{ return sendInfo.octetCount; }

protected:
	OutgoingDataQueue();

	virtual ~OutgoingDataQueue()
	{ }
	
	struct OutgoingRTPPktLink 
	{
		OutgoingRTPPktLink(OutgoingRTPPkt* pkt,
				   OutgoingRTPPktLink* p, 
				   OutgoingRTPPktLink* n) :
			packet(pkt), prev(p), next(n) { }

		~OutgoingRTPPktLink() { delete packet; }

		inline OutgoingRTPPkt* getPacket() { return packet; }

		inline void setPacket(OutgoingRTPPkt* pkt) { packet = pkt; }
		
		inline OutgoingRTPPktLink* getPrev() { return prev; }

		inline void setPrev(OutgoingRTPPktLink* p) { prev = p; }

		inline OutgoingRTPPktLink* getNext() { return next; }

		inline void setNext(OutgoingRTPPktLink* n) { next = n; }

		// the packet this link refers to.
		OutgoingRTPPkt* packet;
		// global outgoing packets queue.
		OutgoingRTPPktLink * prev, * next;
	};

	/**
	 * This computes the timeout period for scheduling transmission
	 * of the next packet at the "head" of the send buffer.  If no
	 * packets are waiting, a default timeout is used.  This actually
	 * forms the "isPending()" timeout of the rtp receiver in the
	 * service thread.
	 *
	 * @return timeout until next packet is scheduled to send.
	 **/
	microtimeout_t
	getSchedulingTimeout();

	/**
	 * This function is used by the service thread to process
	 * the next outgoing packet pending in the sending queue.
	 *
	 * @return number of bytes sent.  0 if silent, <0 if error.
	 **/
	size_t
	dispatchDataPacket();

	/**
	 */
	inline void 
	setInitialTimestamp(uint32 ts)
	{ initialTimestamp = ts; }

	/**
	 */
	inline uint32 
	getInitialTimestamp()
	{ return initialTimestamp; }

	void purgeOutgoingQueue();

        virtual void
        setControlPeer(const InetAddress &host, tpport_t port) = 0;

private:
        /**
	 * A hook to filter packets being sent that have been expired.
	 *
	 * @param - expired packet from the send queue.
	 **/
	inline virtual void onExpireSend(OutgoingRTPPkt&)
	{ }

	virtual void
        setDataPeer(const InetAddress &host, tpport_t port) = 0;

	/**
	 * This function performs the physical I/O for writing a
	 * packet to the destination.  It is a virtual that is
	 * overriden in the derived class.
	 *
	 * @param buffer Pointer to data to write.
	 * @param len Length of data to write.
	 * @return number of bytes sent.
	 **/
	virtual size_t
	sendData(const unsigned char* const buffer, size_t len) = 0;

	static const microtimeout_t defaultSchedulingTimeout;
	static const microtimeout_t defaultExpireTimeout;
	mutable ThreadLock sendLock;
	// outgoing data packets queue
	OutgoingRTPPktLink* sendFirst, * sendLast; 
	uint32 initialTimestamp;
	// transmission scheduling timeout for the service thread
	microtimeout_t schedulingTimeout;
	// how old a packet can reach in the sending queue before deletetion
	microtimeout_t expireTimeout;

	struct {
		// number of packets sent from the beginning
		uint32 packetCount;
		// number of payload octets sent from the beginning
		uint32 octetCount;
		// the sequence number of the next packet to sent
		uint16 sendSeq;
		// contributing sources
		uint32 sendSources[16];
		// how many CSRCs to send.
		uint16 sendCC;
		// This flags tells whether to set the bit M in the
		// RTP fixed header of the packet in which the next
		// provided data will be sent.
		bool marked;
		// whether there was not loss.
		bool complete;
		// ramdonly generated offset for the timestamp of sent packets
		uint32 initialTimestamp;
		// elapsed time accumulated through successive overflows of
		// the local timestamp field
		timeval overflowTime;
	} sendInfo;
};

/** @}*/ // oqueue

#ifdef  CCXX_NAMESPACES
};
#endif

#endif  //CCXX_RTP_OQUEUE_H_

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
