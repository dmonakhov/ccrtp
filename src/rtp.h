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

/** 
 * @file rtp.h 
 *
 * @short General purpose interface of ccRTP. 
 */

#ifndef	__CCXX_RTP_H__
#define	__CCXX_RTP_H__

#ifndef	__CCXX_SOCKET_H__
#include <cc++/socket.h>
#endif

#ifdef	__NAMESPACES__
namespace ost {
#endif

// RTP version
const uint8 RTP_VERSION = 2;

// Time interval expressed in microseconds
typedef uint32 microtimeout_t;

// Time interval expressed in nanoseconds 
typedef uint32 nanotimeout_t;

struct RTCPPacket;
struct SenderInfo;
struct ReceiverInfo;

/**
 * @enum rtp_payload_t rtp.h cc++/rtp.h
 * @short RTP static payload types.
 *
 * RTP Payload Types (usually referred to as PT) for standard audio
 * and video encodings. These codes were initially specified in RFC
 * 1890, ``RTP Profile for Audio and Video Conferences with Minimal
 * Control.''  Codes below 96 may be assigned statically, although
 * some of them are already reserverd. Codes in the range 96-127 are
 * assigned dinamically by means outside of the RTP profile or
 * protocol specification. 
 */
typedef enum
{
	RTP_PAYLOAD_PCMU = 0,   ///< ITU-T G.711. $\mu-$law audio (RFC 1890) 
	RTP_PAYLOAD_1016,       ///< CELP audio (FED-STD 1016) (RFC 1890) (Will soon be deprecated)
	RTP_PAYLOAD_G726,       ///< ITU-T G.726. ADPCM audio (RFC 1890)
	RTP_PAYLOAD_GSM,        ///< GSM audio (RFC 1890)
	RTP_PAYLOAD_G723,       ///< ITU-T G.723. MP-MLQ ACELP audio (RFC 1890)
	RTP_PAYLOAD_DVI4_8000,  ///< Modified IMA ADPCM audio 8Khz (RFC 1890)
	RTP_PAYLOAD_DVI4_16000, ///< Modified IMA ADPCM audio 16Khz (RFC 1890)
	RTP_PAYLOAD_LPC,        ///< LPC audio (RFC 1890)
	RTP_PAYLOAD_PCMA,       ///< ITU-T G.711 A-law audio (RFC 1890)
	RTP_PAYLOAD_G722,       ///< Audio (RFCs 1890, 3047)
	RTP_PAYLOAD_L16_DUAL,   ///< Linear uncompressed dual audio (RFC 1890)
	RTP_PAYLOAD_L16_MONO,   ///< Linear uncompressed mono audio (RFC 1890)
	RTP_PAYLOAD_QCELP,      ///< Audio at 8000 hz.
	RTP_PAYLOAD_MPA = 14,	///< MPEG Audio elem. stream (RFCs 1890, 2250)
	RTP_PAYLOAD_G728,       ///< ITU-T G.728. LD-CELP audio
	RTP_PAYLOAD_DVI4_11025, ///< DVI audio at 11025 hz (by Joseph Di Pol)
	RTP_PAYLOAD_DVI4_22050, ///< DVI audio at 22050 hz (by Joseph Di Pol)
	RTP_PAYLOAD_G729,       ///< ITU-T G.729. CS-ACELP audio
	RTP_PAYLOAD_CELB = 25,  ///< Sun's propietary audio. (RFCs 1890, 2029)
	RTP_PAYLOAD_JPEG,       ///< JPEG (ISO 10918) video (RFCs 1890, 2435)
	RTP_PAYLOAD_NV = 28,    ///< Ron Frederick's nv audio (RFC 1890)
	RTP_PAYLOAD_H261 = 31,  ///< ITU-T H.261 video (RFCs 1890, 2032) 
	RTP_PAYLOAD_MPV,	///< MPEG Video elem. stream (RFCs 1890, 2250)
	RTP_PAYLOAD_MP2T,	///< MPEG 2 Transport stream (RFCs 1890, 2250)
	RTP_PAYLOAD_H263,       ///< ITU-T H.263 video (RFCcs 2190, 2429)
	RTP_PAYLOAD_INVALID = 128,
	// the payload types defined down here are allocated dynamically.
	RTP_PAYLOAD_G726_40,
	RTP_PAYLOAD_G726_24,
	RTP_PAYLOAD_G726_16,
	RTP_PAYLOAD_G729D,
	RTP_PAYLOAD_G729E,
	RTP_PAYLOAD_GSM_EFR,
	RTP_PAYLOAD_L8,
	RTP_PAYLOAD_RED,
	RTP_PAYLOAD_VDVI,
	RTP_PAYLOAD_BT656,      ///< ITU BT.656-3 video (RFC 2431)
	RTP_PAYLOAD_H263_1998,  ///< 1998 version of H.263 video (RFC 2429)
	RTP_PAYLOAD_MP1S,       ///< MPEG Systems stream
	RTP_PAYLOAD_MP2P,       ///< MPEG 2 Program stream
	RTP_PAYLOAD_BMPEG,      ///< Bundled MPEG (RFC 2343)
	RTP_PAYLOAD_EMPTY       ///< "empty" payload     
}	rtp_payload_t;

/**
 * @enum rtcp_type_t rtp.h cc++/rtp.h
 * 
 * RTCP packet types. These codes were initially specified in RFC 1889.
 */
typedef enum
{
	RTCP_TYPE_SR = 200,             ///< Sender Report
	RTCP_TYPE_RR,                   ///< Receiver Report
	RTCP_TYPE_SDES,	                ///< Source DEScription
	RTCP_TYPE_BYE,                  ///< End of participation
	RTCP_TYPE_APP                   ///< APPlication specific
}       rtcp_type_t;

/** 
 * @enum sdes_item_type_t rtp.h cc++/rtp.h
 *
 * SDES items that may be carried in a Source DEScription RTCP packet.
 * CNAME is mandatory, the rest are optional and have different 
 * sending frequencies.
 */
typedef enum
{
	RTCP_SDES_ITEM_END = 0,         ///< END of SDES item list
	RTCP_SDES_ITEM_CNAME,           ///< Canonical end-point identifier
	RTCP_SDES_ITEM_NAME,            ///< Personal NAME of the user
	RTCP_SDES_ITEM_EMAIL,           ///< EMAIL address of the user
	RTCP_SDES_ITEM_PHONE,           ///< Phone number of the user
	RTCP_SDES_ITEM_LOC,             ///< Location where the user is
	RTCP_SDES_ITEM_TOOL,            ///< Application or tool
	RTCP_SDES_ITEM_NOTE,            ///< Comment usually reporting state
	RTCP_SDES_ITEM_PRIV,            ///< Private extension
	RTCP_SDES_ITEM_H323_CADDR,      ///< H323 callable address
}       sdes_item_type_t;

/** 
 * @enum type_of_service_t rtp.h cc++/rtp.h
 * @short Type of network service the application uses.
 *
 * If the application uses enhanced network service, for instance
 * Integrated Services or Differentiated Services, it <em>has not</em>
 * to ensure fair competition with TCP, provided that the requested
 * service is actually being delivered.  Whenever the application uses
 * best-effort service or the requested enhanced service is not
 * actually being delivered, it <em>has</em> to ensure fair competition
 * with TCP. By default, best-effot is assumed.
 *
 * @note Although not required, RTP packets are always sent on top of
 * UDP segments. No other underlying transport protocol is supported
 * at present.
 *
 * @todo implement fair competition with tcp
 */
typedef enum
{
	BEST_EFFORT_SERVICE,     ///< Best-effort network service
	ENHANCED_SERVICE         ///< Enhanced network service
}       type_of_service_t;

/**
 * @enum rtp_purge_t rtp.h cc++/rtp.h
 *
 * @short Flags to select wheter to purge the transmission, reception
 * or both queues.
 *
 * Intended to be used when calling RTPQueue::Purge 
 */
typedef	enum
{
	RTP_PURGE_SEND,        ///< Purge only packets in the sending queue
	RTP_PURGE_RECV,        ///< Purge only packets in the reception queue 
	RTP_PURGE_BOTH         ///< Purge all packets
}	rtp_purge_t;

typedef enum {
	CAST_MCAST,
	CAST_UCAST
}       rtp_cast_t;

class CCXX_CLASS_EXPORT IncomingRTPPkt;
class CCXX_CLASS_EXPORT OutgoingRTPPkt;
class CCXX_CLASS_EXPORT RTPQueue;
class CCXX_CLASS_EXPORT QueueRTCPManager;
class CCXX_CLASS_EXPORT RTPSource;

/** 
 * @class RTPData rtp.h cc++/rtp.h
 * @short Interface to data received over RTP packets.  
 *
 * A class of objects representing data transmitted over RTP packets.
 * Tipically, this object will apply to <em>received</em> data. Data
 * blocks received via RTP connections, along with its related
 * objects (for instance, its source), are accessed through the
 * methods of this class.
 *
 * @author Federico Montesino Pouzols <p5087@quintero.fie.us.es> 
 */
class CCXX_CLASS_EXPORT RTPData
{

public:
	/**
	 * @param origin the RTPData object being copied
	 */
	RTPData(const RTPData& origin);

	/**
	 * Assignment operator
	 * 
	 * @param source the RTPData object being assigned
	 * @return the result of the assignment
	 */
	RTPData&
	operator=(const RTPData& source);

	inline const unsigned char*
	getData() const
	{ return datablock->data; };

	inline size_t
	getSize() const
	{ return datablock->size; };

	inline rtp_payload_t
	getPayloadType() const
	{ return datablock->pt; }

protected:

	RTPData(IncomingRTPPkt& packet);

	~RTPData();

private:
	// dataCounter holds the reference counter
	struct dataCounter {
		uint16 count;
		const unsigned char* data;
		const size_t size;
		rtp_payload_t pt;
		dataCounter(const unsigned char* data, size_t size, rtp_payload_t pt);
		~dataCounter();
	};

	mutable dataCounter* datablock;



	// Who sent this data
	RTPSource* src;

	friend RTPQueue;
};

/** 
 * @class RTPSource rtp.h cc++/rtp.h
 * 
 * @short A class of objects representing sources of RTP and RTCP
 * packets. 
 * 
 * Each source for which any RTP or RTCP packet has been received is
 * represented through an RTPSource object, that provides access to
 * all known data about the source.
 *
 * @note Objects representing sources are dinamically created and
 * <em>destroyed</em> as participants enter, leave or seem to 
 * leave the current session. 
 * 
 * @author Federico Montesino Pouzols <p5087@quintero.fie.us.es> */
class CCXX_CLASS_EXPORT RTPSource
{

public:
	bool getHello();
	bool getGoodbye();

	uint32 getID() const
	{ return ssrc; };

	/**
	 * Get the transmission rate for this source.
	 *
	 * @todo implement with RTCP checking
	 */
	uint32 
	getRate() const;

	/**
	 * Specify how much time the incoming packets will be buffered
	 * for this source.  Note that the size is specified in
	 * temporal units, thus the internal queue will usually hold
	 * as many packets as necessary to fill the specified amount
	 * of time. The default size is the global kitchen size when
	 * the source is created.
	 *
	 * @param t amount of time the reception buffer fills. 0 means
	 *        there is no buffer/kitchen and, for this source,
	 *        RTPQueue::getCookedPacket behaves as
	 *        RTPQueue::getPacket does.
	 **/
	inline void
	setKitchenSize(microtimeout_t s)
	{ kitchensize = ((s / 1000) * getRate() / 1000); };
	 
	/**
	 * Get the required size of the kitchen for this source.
	 *
	 * @return global kitchen size, in microseconds
	 */
	inline microtimeout_t
	getKitchenDuration() const
	{ return (((kitchensize * 1000) / getRate())* 1000); };

	inline uint32
	getKitchenSize() const
	{ return kitchensize; };

	/**
	 * Get the current size of the kitchen (buffer) for this
	 * packet as the amount of time covered by the current
	 * buffered packet.
	 *
	 * @return amount of time covered by the source reception
	 *         buffer.
	 **/
	inline microtimeout_t
	getCurrentKitchenDuration()
	{ return (((currentkitchen * 1000) / getRate())* 1000); };

	inline microtimeout_t
	getCurrentKitchenSize()
	{ return currentkitchen;};

	const char* const
	getSDESItem(sdes_item_type_t type) const;
	
	inline const char* const 
	getCNAME() const
	{ return getSDESItem(RTCP_SDES_ITEM_CNAME); };

 	inline const char* const
	getNAME() const
	{ return getSDESItem(RTCP_SDES_ITEM_NAME); };

	inline const char* const
	getEMAIL() const
	{ return getSDESItem(RTCP_SDES_ITEM_EMAIL); };

	inline const char* const
	getPHONE() const
	{ return getSDESItem(RTCP_SDES_ITEM_PHONE); };

	inline const char* const 
	getLOC() const
	{ return getSDESItem(RTCP_SDES_ITEM_LOC); };

	inline const char* const 
	getTOOL() const
	{ return getSDESItem(RTCP_SDES_ITEM_TOOL); };

	inline const char* const 
	getNOTE() const
	{ return getSDESItem(RTCP_SDES_ITEM_NOTE); };

	inline const char* const 
	getPRIV() const
	{ return getSDESItem(RTCP_SDES_ITEM_PRIV); };

	inline const char* const 
	getH323_CADDR() const
	{ return getSDESItem(RTCP_SDES_ITEM_H323_CADDR); };

	/**
	 * Get if this Source is currently an active sender.
	 */
	inline bool
	isSender() const
	{ return active_sender; };

	bool isValid() const
	{ return valid; };

	inline bool
	operator==(const RTPSource &rhs) const
	{ return (this == &rhs); }

	inline bool
	operator!=(const RTPSource &rhs) const
	{ return !(*this == rhs); }

	/**
	 * @param ssrc SSRC identifier of the source
	 */
	RTPSource(uint32 ssrc); 

	/**
	 * Purges all incoming packets from this source.
	 */
	~RTPSource();

	/**
	 * @param origin the RTPSource object being copied
	 */
	RTPSource(const RTPSource& origin);

	RTPSource&
	operator=(const RTPSource &origin);

protected:
		
private:
	/**
	 * Set the current size of the kitchen (buffer) for this
	 * packet as the amount of octets accummulated in the incoming
	 * packet queue.
	 *
	 * @param s octets accumulated in the buffer.
	 **/
	inline void
	setCurrentKitchenSize(uint32 s)
	{ currentkitchen = s; };

	/**
	 *
	 * */
	void endSource();

	void
	setSDESItem(sdes_item_type_t item, const char* const value);

	/**
	 * Log the reception of a new packet from this source. Updates
	 * data such as the packet counter, the expected sequence
	 * number for the next packet and the time the last packet was
	 * received at.
	 *
	 * @param p packet just created and to be logged 
	 */
	void 
	recordReception(IncomingRTPPkt& p);

	/**
	 * Log the insertion of a packet from this source into the
	 * scheduled queue. Updates the size of this source's
	 * kitchen. All received packets should be registered with
	 * recordReception(), but only those actually inserted into
	 * the queue should be registered via this method.
	 *
	 * @param p packet inserted into the queue 
	 * */
	void 
	recordInsertion(IncomingRTPPkt& p);

	/**
	 * Log the extraction of a packet from this source from the
	 * scheduled queue. Updates the size of this source's
	 * kitchen. 
	 *
	 * @param p packet extracted from the queue 
	 * */
	void 
	recordExtraction(IncomingRTPPkt& p);

	/**
	 * Mark this source as an active sender.
	 */
	inline void
	setSender(bool active)
	{ active_sender = active; };

	/**
	 * Set the timestamp of the first packet received from this source. 
	 *
	 * @param ts timestamp of the first packet from this source
	 */
	inline void 
	setInitialTimestamp(uint32 ts)
	{ initial_timestamp = ts; }

	/**
	 * Get the timestamp of the first packet received from this
	 * source. This timestamp must be substracted to the timestamp
	 * of all packets from this source so that applications will
	 * not have to handle the initial timestamp.
	 *
	 * @return timestamp of the first packet from this source
	 */
	inline uint32 
	getInitialTimestamp()
	{return initial_timestamp; }

	/**
	 * Get the expected sequence number for the next packet to be
	 * received.
	 *
	 * @return the expected sequence number for the next packet 
	 */
	inline uint16
	getExpectedSeqNum()
	{ return expectedseqnum; }

	/**
	 * Set the expected sequence number for the next packet to be
	 * recived.
	 *
	 * @param n expected sequence number for the next packet 
	 */
	inline void
	setExpectedSeqNum(uint16 n)
	{ expectedseqnum = n; }

	// SSRC 32 bit identifier carried in RTP and RTCP packets (in
	// network order)
	uint32 ssrc;
	// timestamp of the first packet received from this source
	uint32 initial_timestamp; 
	// number of packets received from this source
	uint32 packet_count;
	// time the last packet was received at
	struct timeval last_time;
	// required kitchen for this source. Represented as octets,
	// given the current transmission rate
	uint32 kitchensize;
	// current kitchen for this source. Represented as octets,
	// given the current transmission rate
	uint32 currentkitchen;
	// the expected sequence number of the next packet to be received.
	uint16 expectedseqnum;  

	// this flag assures we only call one gotHello and one gotGoodbye
	// for this src.
	bool flag;

	// before becoming valid, multiple packets from this source, 
	// or an SDES RTCP containing its CNAME should be received.
	bool valid; 
	// A valid source not always is active
	bool active_sender;

	// Sources located before and after this one in the list
	// of sources.
	RTPSource* prev, * next;
	// first/last packets from this source in the queue 
	IncomingRTPPkt* first, * last;
	// Prev and next inside the collision list
	RTPSource* nextcollis;  
	// dummy content for undetermined descriptors
	static const char* const unknown;
	// sender info from the last sender report of this source  
	SenderInfo* sender_info;
	// dummy content for the last sender info from this source
	static const SenderInfo* dummySI;
	// last report block this source sent about the local source
	ReceiverInfo* receiver_info;
	// dummy content for the last report from this source
	static const ReceiverInfo* dummyRB;
	// Data extracted from the SDES items sent by this source
	char **sdes_items;
	
	friend class MembershipControl;
	friend RTPQueue;
	friend QueueRTCPManager;
	friend IncomingRTPPkt;
};

// TODO: implement this idea
class CCXX_CLASS_EXPORT Members
{
public:

	Members() :	
		members(static_cast<uint32>(-1)), // -1 to counteract dummysource
		active_senders(0)
	{ };
	/**
	 *
	 *
	 */
	inline void
	increaseMembersCount()
	{ members++; };

	/**
	 *
	 *
	 */
	inline void
	decreaseMembersCount()
	{ members--; };

	inline void
	setMembersCount(uint32 n)
	{ members = n; };

	inline void
	increaseSendersCount()
	{ active_senders++; };

	inline void
	decreaseSendersCount()
	{ active_senders--; };

	inline void
	setSendersCount(uint32 n)
	{ active_senders = n; };


	/**
	 *
	 * @return
	 */
	inline uint32 
	membersCount() const
	{ return members; };
	
	inline uint32
	sendersCount() const
	{ return active_senders; };


	// number of identified members
	uint32 members;
	// number of identified members that currently are active senders
	uint32 active_senders;
};

/** 
 * @class MembershipControl rtp.h cc++/rtp.h
 * @short Controls the group membership in the current session.  
 *
 * The use of RTCP is feasible in sessions with a few
 * participants. However, when there are thousands or millions of
 * participants, scalability problems impede the use of RTCP. In such
 * situations, group membership sampling (see RFC 2762) is recommended
 * instead of a membership table.
 *
 * For now, this class implements only a hash table of members, but
 * its design and relation with other classes is intented to support
 * group membership sampling in case scalability problems arise.
 *
 * @todo implement the reallocation mechanism (e.g., when the
 * number of ssrcs per collision list goes up to 2, make the
 * size approx. four times bigger (0.5 ssrcs per list
 * now. when the number of ssrcs per list goes down to 0.5,
 * decrease four times. Do not use 2 or 0.5, but `2 +
 * something' and `0.5 - somehting'). Always jumping between
 * prime numbers -> provide a table from 7 to many.  
 *
 * @author Federico Montesino Pouzols <p5087@quintero.fie.us.es> 
 */
class CCXX_CLASS_EXPORT MembershipControl : public Members
{
public:
	/**
	 * Get the description of a source by its <code>SSRC<code> identifier.
	 *
	 * @param ssrc SSRC identifier, int network order. 
	 * @return the RTPSource object identified by <code>ssrc<code>
	 * @retval a dummy empty object if there is no source with the 
	 *         given ssrc identifier
	 */
	inline const RTPSource&
	getSource(uint32 ssrc) const
	{ return const_cast<MembershipControl*>(this)->getSourceBySSRC(ssrc,false); }

	/**
	 * Get the description of a source by its <code>ssrc<code> identifier.
	 *
	 * @param ssrc SSRC identifier, int network order. 
	 * @return the RTPSource object identified by <code>ssrc<code>
	 * @retval a dummy empty object if <code>create == false<code> 
	 *         and there is no source with the given ssrc identifier
	 */
	inline const RTPSource&
	getOrCreateSource(uint32 ssrc)
	{ return getSourceBySSRC(ssrc,true); }

protected:
	/**
	 * 
	 * @param ssrc Synchronization SouRCe identifier
	 * @return the source object just created
	 */
	RTPSource&
	addNewSource(uint32 ssrc);

	/**
	 * @short The initial size is a hint to allocate the resources
	 * needed in order to keep the members' identifiers and
	 * associated information.
	 *
	 * Although ccRTP will reallocate resources when it becomes
	 * necessary, a good hint may save a lot of unpredictable time
	 * penalties.
	 *
	 * @param initial_size an estimation of how many participants
	 * the session will consist of.
	 *
	 */
	MembershipControl(uint32 initial_size = 7);

	/**
	 * Destructor. Purges all RTPSource structures created during
	 * the session, as well as the hast table and the list of
	 * sources.
	 * */
	virtual
	~MembershipControl();

	/**
	 * Purge all RTPSource structures, the hash table and the list
	 * of sources.
	 * */
	void
	endMembers();

	/**
	 * Get the description of a source by its <code>ssrc<code> identifier.
	 *
	 * @param ssrc SSRC identifier, int network order. 
	 * @param create whether to create a new source if not found
	 * @return the RTPSource object identified by <code>ssrc<code>
	 * @retval a dummy empty object if create == false and there is 
	 * no source with the given ssrc identifier
	 */
	RTPSource&
	getSourceBySSRC(uint32 ssrc, bool create = false);

	/**
	 * Remove the description of the source identified by
	 * <code>ssrc</code>
	 *
	 * @return whether the source has been actually removed or it
	 * did not exist.  
	 */
	bool
	removeSource(uint32 ssrc);

	const RTPSource&
	NullSource() const
	{ return dummysource; };

	// an empty RTPSource representing unidentified sources
	const static RTPSource dummysource;

private:

	MembershipControl(const MembershipControl &o);

	MembershipControl&
	operator=(const MembershipControl &o);

	// Hash table with sources of RTP and RTCP packets
	uint32 SOURCE_BUCKETS; 
	RTPSource** sources;
	// List of sources, ordered from older to newer
	RTPSource* first, * last;
};

/**
 * @class RTPQueue rtp.h cc++/rtp.h
 *
 * A thread serviced packet queue handler for building different kinds
 * of RTP protocol systems.  The queue manages both incoming and
 * outgoing RTP packets, as well as synchronization and
 * transmission/reception timers.  By making the queue handler a
 * seperate base class it becomes possible to define RTP classes for
 * RTP profiles and sessions of different types.
 *
 * Outgoing packets are sent via the RTPQueue::putPacket method.
 *
 * Incoming packets can be retrieved via: 
 * <ul>
 * <li>the RTPQueue::getPacket methods, that provide "raw" access to 
 *      the packets in the queue.
 * </li>
 * <li>the RTPQueue::getCookedPacket method, that provides "cooked"
 *      access.
 * </li>
 * </ul>
 *
 * @author David Sugar <dyfet@ostel.com>
 * @short RTP protocol queue handler.  
 */
class CCXX_CLASS_EXPORT RTPQueue : protected Thread, protected MembershipControl
{
public:
	/**
	 * Make Start public.
	 */
	inline void Start(void)
		{Thread::Start();};
	/**
	 * Get the application description.
	 *
	 * @return The RTPSource object describing the local application
	 */
	inline const RTPSource& 
	getLocalInfo() const
	{ return *localsrc; };

 	/**
 	 * Determine if packets are waiting in the reception queue.
 	 *
	 * @param src optional source selector.
 	 * @return true if packets are waiting.
 	 */
 	bool 
	isWaiting(const RTPSource &src = dummysource) const;

 	/**
 	 * Determine if cooked packets are waiting in the reception
 	 * queue.  Note that isCookedWaiting() == true implies
 	 * isWaiting() == true.
	 *
 	 * @return true if there are cooked packets waiting.
	 * @todo implement it
	 */
 	bool 
	isCookedWaiting(void) const;
 
 	/**
 	 * Determine if outgoing packets are waiting to send.
 	 *
 	 * @return true if there are packets waiting to be send.
 	 */
 	bool 
	isSending(void) const;

	/**
	 * This is used to create a data packet in the send queue.  
	 * Sometimes a "NULL" or empty packet will be used instead, and
	 * these are known as "silent" packets.  "Silent" packets are
	 * used simply to "push" the scheduler along more accurately
	 * by giving the appearence that a next packet is waiting to
	 * be sent and to provide a valid timestamp for that packet.
	 *
	 * @param stamp timestamp for expected send time of packet.
	 * @param payload format of this packet.
	 * @param data value or NULL if special "silent" packet.
	 * @param length may be 0 to indicate a default by payload type.
	 * @param mark mark field in the RTP header
	 */
	void 
	putPacket(uint32 stamp, rtp_payload_t payload, unsigned char* data = NULL, size_t len = 0, bool mark = false);

	/**
	 * Attempt to get a cooked packet. If available, thid method
	 * will retrieve the first cooked packet. A packet is said to be
	 * cooked if:
	 * <ol>
	 * <li>There is no disorder or lost between the last retrieved 
	 *     packet an the current packet.
	 * </li>
	 * <li>There is disorder or lost, but the queue has reached its 
	 *     maximun size.
	 * </li>
	 * </ol>
	 *
	 * Therefore, cooking means recomposing a corrupted stream
	 * -waiting for the disordered packet, regenerating it (if FEC
	 * is enabled) or asking for retransmission- whenever it is
	 * possible.
	 *
	 * If the queue has not yet reached its maximun size (that can
	 * be set with RTPQueue::setKitchenSize), ccRTP will try to
	 * cook the packets before allowing the application to pick
	 * them up. Note that if there are more packets than the
	 * maximun, they will not be thrown away.
	 *
	 * In order to accomplish this task, at the beginning no
	 * packet will be released until the queue reaches the kitchen
	 * size (this will provide a buffer to ccRTP where to cook the
	 * packets). Then, provided that there is no disorder, ccRTP
	 * will release every packet when asked (note that usual
	 * applications will try to retrieve packets, broadly
	 * speaking, at the same rate they are received, so the
	 * kitchen will remain the same (maximun) size if there is no
	 * lost).
	 *
	 * If the newest packet (the one with the minor timestamp) in
	 * the queue is disordered, ccRTP will not release it until
	 * the kitchen reaches at least it maximun size. When the
	 * maximun size is reached, if disorder could not be fixed,
	 * ccRTP will then release the packet. Thus, unrepairable
	 * disorder implies the queue will stop providing packets for a
	 * while and then provide a disordered packet.
	 *
	 * If a lost packet arrives after newer packets have been
	 * retrieved, it will be silently discarded.
	 *
	 * Summary: 
	 *
	 * If the next packet is ordered, it will be released
	 * wheter or not the kitchen is full.
	 *
	 * If the next packet is disordered, it will be released only
	 * when the kitchen has at least its maximun size.
	 *
	 * @param src optional source selector.
	 * @return the block of data contained in the first "cooked" packet.
	 * @see RTPQueue::setKitchenSize
	 * @see RTPQueue::isCookedWaiting
	 */
	const RTPData&
	getCookedPacket(const RTPSource &src = dummysource);

 	/**
 	 * Get timestamp of first packet waiting in the queue.
 	 *
	 * @param src optional source selector.
 	 * @return timestamp of first arrival packet.
 	 **/
 	uint32 
	getFirstTimestamp(const RTPSource &src = dummysource);
 
 	/**
 	 * Get the sequence id of the first packet waiting.
 	 *
 	 * @return sequence id of first packet.
	 * @param src optional source selector.
 	 **/
 	uint16 
	getFirstSequence(const RTPSource &src = dummysource);

	/**
	 * Retreive data from a specific timestamped packet if such a
	 * packet is currently available in the receive buffer.
	 *
	 * @param src optional source selector.
	 * @return data retrieved from the reception buffer.
	 **/
	const RTPData&
	getPacket(uint32 stamp, const RTPSource &src = dummysource);

	/**
	 * Retreive data from a specific timestamped packet if such a
	 * packet is currently available in the receive buffer.
	 *
	 * @param timestamp of packet desired.
	 * @param data buffer to copy into.
	 * @param maximum data size.
	 * @param src optional source selector
	 * @return number of packet data bytes retrieved.
	 * @retval 0 if there is no such packet.
	 **/
	size_t 
	getPacket(uint32 stamp, unsigned char* data, size_t max,
		  const RTPSource &src = dummysource);

	/**
	 * Get the payload type of a specific packet by timestamp.
	 *
	 * @param timestamp to find.
	 * @param src optional source selector
	 * @return payload of specified packet if found.
	 **/
	rtp_payload_t 
	getPayloadType(uint32 timestamp, const RTPSource &src = dummysource);

	/**
	 * Get the timestamp for a packet whose payload sampling instant
	 * corresponds to the current system time.
	 *
	 * The timestamp applications should provide for each packet
	 * represents the sampling instant of its payload and should
	 * not be a reading of the system clock. Nevertheless, the
	 * internal operation of the RTP stack relies on the accuracy
	 * of the provided timestamp, since several computations
	 * assume that there is a certain degree of correspondence
	 * between the timestamp and the system clock.
	 * 
	 * It is recommended that applications use this method in
	 * order to <em>periodically adjust the RTP timestamp</em>.
	 *
	 * In particular, it is advisable getting the timestamp
	 * corresponding to the first sampling instant or any instant
	 * after a period of inactivity through a call to this method.
	 *
	 * Applications should use the nominal sampling or
	 * any other value provided by the coder in order to compute
	 * the next timestamps with minimum computational requirement.
	 *
	 * For instance, an application using an RTP profile that
	 * specifies a fixed sampling rate of 8 Khz with eight bits
	 * per sample, continuously transmitting audio blocks 80
	 * octets long, would transmit 100 packets every
	 * second. Every packet would carry a timestamp 80 units
	 * greater than the previous one. So, the first timestamp
	 * would be obtained from this method, whereas the following
	 * ones would be computed adding 80 every time. Also the
	 * timestamp should be increased for every block whether
	 * it is put in the queue or dropped. 
	 *
	 * The aforementioned increment can be obtained from the
	 * RTPQueue::getTimestampIncrement() method rather than
	 * computing it by hand in the application.
	 *
	 * @note Frame based applications must follow a specific
	 * timestamping method, probably specified in a profile.
	 *
	 * @note You should take into account that by default ccRTP
	 * assumes that the application begins sampling at the queue
	 * creation time.  Moreover, the first sampling instant is
	 * assigned a "user visible" timestamp of 0, although the RTP
	 * stack will then add internally a ramdom offset unknown to
	 * the application.  That is to say, the application may count
	 * samples from 0 in order to get the timestamp for the next
	 * packet, provided that the first sampling instant is the
	 * same as the queue creation time.  Nevertheless, this
	 * simpler way of starting will not be as accurate as it would
	 * be if the application got at least the first timestamp
	 * through getCurrentTimestamp.  <em>We provide this option
	 * since ccRTP interface is evolving, but we admit that it is
	 * ugly, we could remove this option or even replace uint32
	 * timestamps with a restrictively regulated object;
	 * suggestions are gladly welcomed</em>
	 *
	 * @param pt payload type of the packets sent. Determines the
	 * timestamp increasing rate.  
	 */
	uint32 
	getCurrentTimestamp(rtp_payload_t pt) const;

	/**
	 * If your application uses a fixed sampling rate and a fixed
	 * payload format, given the size of the next packet to be
	 * sent, this method provides the timestamp increment for the
	 * next packet.
	 *
	 * @param packet_size the size of the next packet to be sent
	 * @return the timestamp increment for the next packet */
	uint32
	getTimestampIncrement(size_t packet_size) const;

	/**
	 * Specify the bandwidth of the current session.  
	 *
	 * @param bw bandwidth of the current session, in bits/s.
	 *
	 * @see QueueRTCPManager::setControlBandwidth
	 */
	void
	setSessionBandwidth(uint32 bw)
	{ sessionbw = bw; };

	/**
	 * Get the transmission rate for a payload type. Note that for
	 * some payload types, the transmission rate corresponds to
	 * the timestamp rate, but this is not true for all payload
	 * types.
	 *
	 * @return transmission rate corresponding to the specified
	 *         payload type. If no payload type is specified,
	 *         returns the timestamp rate corresponding to the
	 *         first available packet.
	 * @todo complete implementation
	 **/
	uint32 
	getRate(rtp_payload_t pt = RTP_PAYLOAD_EMPTY) const;

	/**
	 * Get the payload type of the first available packet
	 *
	 * @param optional source selector
	 * @return payload type of the first available packet
	 */
	rtp_payload_t
	getPayloadType(const RTPSource& src) const;

	/**
	 * Get the representation of a session participant by its
	 * Synchronization SouRCe identifier. 
	 *
	 * @param ssrc an SSRC identifier 
	 *
	 * @return a description of the source identified by ssrc
	 */
	inline const RTPSource&
	getSource(uint32 ssrc) const
	{ return MembershipControl::getSource(htonl(ssrc)); };

	/**
	 * Specify the kind of service the application expects to use.  
	 *
	 * @param tos type of service the application expects to use
	 *
	 * @note If enhanced service is specified but packet loss is
	 * high (the requested service does not appear to actually be
	 * delivered) ccRTP defaults to best-effort suitable
	 * behaviour: guarantee fair competition with TCP. 
	 * 
	 * @todo Implement fair competition with tcp
	 */
	inline void
	setTypeOfService(type_of_service_t tos)
	{ type_of_service = tos; }
 
	/**
	 * Set partial data for an already queued packet.  This is often
	 * used for multichannel data.
	 *
	 * @return number of packet data bytes set.
	 * @param timestamp of packet desired.
	 * @param data buffer to copy from.
	 * @param offset to copy from.
	 * @param maximum data size.
	 */
	size_t 
	setPartial(uint32 timestamp, unsigned char* data, size_t offset, size_t max);

	/**
	 * Get partial data from a packet.  This is often used to support
	 * oddball hardware that has unusual or non-standard codec
	 * buffering intervals.
	 *
	 * @return number of packet data bytes retrieved.
	 * @param timestamp of packet desired.
	 * @param data buffer to copy into.
	 * @param offset to copy from.
	 * @param maximum data size.
	 */
	size_t 
	getPartial(uint32 timestamp, unsigned char* data, size_t offset, size_t max);

	/**
	 * Get active connection state flag.
	 *
	 * @return true if connection "active".
	 */
	inline bool 
	isActive(void) const
	{ return active; };

	/**
	 * Set the default scheduling timeout to use when no data
	 * packets are waiting to be sent.
	 *
	 * @param timeout in milliseconds.
	 */
	inline void 
	setTimeout(microtimeout_t t)
	{ timeout = t;};

	/**
	 * Set the "expired" timer for expiring packets pending in
	 * the send queue which have gone unsent and are already
	 * "too late" to be sent now.
	 *
	 * @param timeout to expire unsent packets in milliseconds.
	 *
	 */
	inline void 
	setExpired(microtimeout_t t)
	{ expired = t;};

	/**
	 * Specify how much time the incoming packets will be buffered
	 * for each and every source.  Note that the size is specified
	 * in temporal units, thus the internal queue will usually
	 * hold as many packets as necessary to fill the specified
	 * amount of time. The default size is 0 us.
	 *
	 * If using only the "high level" method for retrieving
	 * packets, RTPQueue::getCookedPacket(), before providing
	 * disordered packets to the application, ccRTP will try to
	 * cook them. The bigger the specified time, the higher the
	 * probability of reordering and recomposing the stream in case
	 * of corruption is.
	 *
	 * @param t amount of time the reception buffer fills. 0 means
	 *        there is no buffer/kitchen and
	 *        RTPQueue::getCookedPacket behaves as
	 *        RTPQueue::getPacket does. 
	 *
	 * @see RTPSource::setKitchenSize()
	 * @see RTPQueue::getCookedPacket()
	 * @see RTPQueue::isCookedWaiting() 
	 **/
	void
	setGlobalKitchenDuration(microtimeout_t t);

	void
	setGlobalKitchenSize(uint32 s);

	/**
	 * Get the current global kitchen size. Note that the kitchen
	 * size can be modified individually for every source, so it
	 * is not guaranteed that this is the current kitchen for all
	 * sources.
	 * 
	 * @return global kitchen size, in microseconds
	 **/
	inline microtimeout_t
	getGlobalKitchenDuration() const
	{ return (((kitchensize * 1000) / getRate())*1000); };
	
	/**
	 * This method sets the maximun end to end delay allowed. If
	 * the processing delay plus the trip time for a packet is
	 * greater than the end to end delay, the packet is discarded,
	 * and the application cannot get it.
	 *
	 * This is a way of setting an upper bound to the end to end
	 * delay, computed as the elapsed time between the packet
	 * timestamping at the sender side, and the picking of the
	 * packet at the receiver side.
	 *
	 * @param t maximum end to end delay allowed. A value of 0
	 * implies there is no limit and is the default
	 */
	inline void
	setEndToEndDelay(microtimeout_t t)
		{ e2edelay = t; };

	/**
	 * Set maximum packet segment size before fragmenting sends.
	 *
	 * @param maximum packet size.
	 */
	inline void 
	setSegmentSize(size_t size)
		{segment = size;};

	/**
	 * Is last packet processed "complete" or are sequences
	 * missing?
	 *
	 * @return true if complete.
	 *
	 * @todo maybe, with the new RTPData, this is unnecessary. Could
	 * be useful for an application that is trying to figure out 
	 * wheter to start processing data or wait for misordered packets.
	 */
	inline bool 
	isComplete(void)
		{return complete;};

	/**
	 * Is last packet processed "marked" (used to signal end on
	 * multi-sequence sends).
	 *
	 * @return true if marked.
	 *
	 * @todo maybe, with the new RTPData, this is unnecessary.
	 */
	inline bool 
	isMarked(void)
		{return marked;};

 	/**
 	 * Set the packet timeclock for synchronizing timestamps.
 	 */
 	inline void 
	setTimeclock(void)
 		{timeclock.setTimer();};
 
 	/**
 	 * Get the packet timeclock for synchronizing timestamps.
 	 *
 	 * @return runtime in milliseconds since last set.
 	 */
 	inline timeout_t 
	getTimeclock(void)
 		{return timeclock.getElapsed();};
	
	/**
	 * Get the total number of packets sent so far
	 *
	 * @return total number of packets sent
	 */
	inline uint32
	RTPSendCount() const
	{ return sendcount;};

	/**
	 * Get the total number of octets (payload only) sent so far.
	 *
	 * @return total number of octets sent as payload in RTP
	 *         packets.
	 **/
	inline uint32
	RTPOctetCount() const
	{ return octetcount;};

protected:
	
	/**
	 * Constructor. This will generate a random application SSRC
	 * identifier.
	 *
	 * @param pri service thread base priority relative to its parent
	 * @param size an estimation of the number of participants in the 
	 *        session */
	RTPQueue(int pri, uint32 size = 7);

	/**
	 * Using this constructor you can start a session with the
	 * given ssrc, instead of the usual randomly generated
	 * one. This is necessary when you need to initiate several
	 * sessions having the same SSRC identifier, for instance, to
	 * implement layered encoding, in which case each layer is
	 * managed through a different session but all sessions share
	 * the same SSRC identifier. 
	 *
	 * @warning This doesn't seem to be a good solution
	 *
	 * @param ssrc Synchronization SouRCe identifier for this session
	 * @param pri service thread base priority relative to it's parent
	 * @param size an estimation of the number of participants in the 
	 *        session 
	 */
	RTPQueue(uint32 ssrc, int pri, uint32 size = 7);
	
	/**
	 * The queue destructor flush the queue and stop all services.
	 */
	virtual 
	~RTPQueue(); 

	/**
	 * A plugin point for a scheduler of RTCP packets. The default
	 * implementation in RTPQueue is to do nothing.
	 * 
	 * @param wait queue scheduling timeout 
	 */
	virtual inline void 
	RTCPService(microtimeout_t& wait)
	{ return; };

	/**
	 * A plugin point for posting of BYE messages.
	 */
	virtual void 
	Bye(const char* const reason)
	{ return; }

        /**
         * A plugin point for timer tick driven events.
         */
        virtual void timerTick(void)
	{ return; };

	/**
	 * This computes the timeout period for scheduling transmission
	 * of the next packet at the "head" of the send buffer.  If no
	 * packets are waiting, a default timeout is used.  This actually
	 * forms the "isPending()" timeout of the rtp receiver in the
	 * service thread.
	 *
	 * @return timeout until next packet is scheduled to send.
	 */
	microtimeout_t
	getTimeout(void);

	/**
	 * This function is used to check for and schedule against
	 * arriving packets based on the derived connection type.
	 *
	 * @return true if packet waiting for processing.
	 * @param number of microseconds to wait.
	 */
	virtual bool
	isPendingData(microtimeout_t timeout) = 0;

  	/**
 	 * This function is used to purge the queue(s).
	 * Depending on the flag value, it purges the packets waiting
	 * to be sent, waiting to be picked up from the reception
	 * queue, or both.  
	 */
	void Purge(rtp_purge_t flag);

	/**
	 * This function is used by the service thread to process
	 * the next outgoing packet pending in the sending queue.
	 *
	 * @return number of bytes sent.  0 if silent, <0 if error.
	 */
	size_t
	sendPacket(void);

	/**
	 * This function performs the physical I/O for writing a
	 * packet to the destination.  It is a virtual that is
	 * overriden in the derived class.
	 *
	 * @return number of bytes sent.
	 * @param packet to write.
	 * @param length of data to write.
	 */
	virtual size_t
	writeData(const unsigned char* const packet, size_t len) = 0;

	/**
	 * This function is used by the service thread to process
	 * the next incoming packet and place it in the receive list.
	 *
	 * @return number of payload bytes received.  <0 if error.
	 */
	size_t
	recvPacket(void);

	/**
	 * This function performs the physical I/O for reading a
	 * packet from the source.  It is a virtual that is
	 * overriden in the derived class.
	 *
	 * @return number of bytes read.
	 * @param packet read buffer.
	 * @param length of data to read.
	 */
	virtual size_t
	readData(unsigned char* buffer, size_t len) = 0;

	/**
	 * A virtual function to support parsing of arriving packets
	 * to determine if they should be kept in the queue and to
	 * dispatch events.
	 *
	 * @return true if packet is kept.
	 * @param packet returned.
	 */
	virtual bool
	gotPacket(IncomingRTPPkt* packet)
	{ return true; }

	/**
	 * A hook to filter packets being sent that have been expired.
	 *
	 * @param packet expired from the send queue.
	 */
	virtual void expireSend(OutgoingRTPPkt *packet)
		{return;};

	/**
	 * A hook to filter packets in the receive queue that are
	 * being expired.
	 *
	 * @param packet expired from the recv queue.
	 */
	virtual void expireRecv(IncomingRTPPkt *packet)
		{return ;};

	/**
	 * This is used to fetch a packet in the receive queue and to
	 * expire packets older than the current timestamp.
	 *
	 * @return packet buffer object for current timestamp if found.
	 * @param timestamp timestamp requested.
	 * @param src optional source selector
	 * @note if found, the packet is removed from the reception queue
	 */
	IncomingRTPPkt*
	getWaiting(uint32 timestamp, const RTPSource &src = dummysource);

	/**
	 * This method ends the queue and service threads.
	 */
	void
	endQueue();

	/**
	 * The queue removes itself when the service thread is detached
	 **/
	void Final()
	{ delete this; };

	// true if connection "active"
	volatile bool active;       
	// Object representing the local source
	RTPSource* localsrc;
	// when the queue is created
	struct timeval initial_time;  
	uint32 current_rate;
	// ramdonly generated offset for the timestamp of sent packets
	uint32 initial_timestamp;

private:
	friend IncomingRTPPkt;

	RTPQueue(const RTPQueue &o);

	RTPQueue&
	operator=(const RTPQueue &o);

	/**
	 * Global queue initialization.
	 *
	 * @param localssrc local 32-bit SSRC identifier
	 **/
	void
	initQueue(uint32 localssrc);

	/**
	 * Runnable method for the service thread.
	 */
	void 
	Run(void);
	

	/**
	 * Transmission and reception of RTP packets. 
	 * 
	 * @param wait scheduling timeout
	 */
	void 
	RTPService(microtimeout_t& wait);
	
	/**
	 * Insert a just received packet in the queue (both general
	 * and source specific queues). If the packet was already in
	 * the queue (same SSRC and sequence number), it is not
	 * inserted but deleted.
	 *
	 * @param pkt a packet just received and validated 
	 */
	void 
	insertRecvPacket(IncomingRTPPkt *packet);		

	type_of_service_t type_of_service;  
	uint32 sessionbw;
	rtp_cast_t sessioncast;

	// packet receive workspace size
	static const size_t RECVBUFFER_SIZE = 8192;	
	// number of packets sent from the beginning
	uint32 sendcount;
	// number of payload octets sent from the beginning
	uint32 octetcount;

	// transmission and reception queues
	OutgoingRTPPkt* sendfirst, * sendlast; 
	IncomingRTPPkt* recvfirst, * recvlast;
	mutable Mutex sendlock, recvlock;
	// the sequence number of the next packet to sent
	uint16 sendseq;    
	// contributing sources
	uint32 sendsources[16];
	// how many CSRCs to send.
	unsigned sendcc;       
	// maximum packet size before fragmenting sends.
	unsigned segment;      
	// bit M in RTP fixed header of the last packet
	bool marked;
	// whether there was not loss
	bool complete;
	
	TimerPort timeclock;

	// elapsed time accumulated through successive overflows of
	// the local timestamp field
	struct timeval overflow_time; 

	// transmission scheduling timeout for the service thread
	microtimeout_t timeout;
	// how old a packet can reach in the sending queue before deletetion
	microtimeout_t expired;
	
	// Maximun delay allowed between packet timestamping and
	// packet availability for the application.
	microtimeout_t e2edelay;  
	// Kitchen (reordering buffer) size in octets, given the
	// current transmission rate.
	uint32 kitchensize;

	// this is a global table holding the 
	// `payload_type <-> timestamp_rate' correpondence for 
	// the statically assigned payload types
	static uint32 payload_rate[96];
};

/**
 * @class QueueRTCPManager rtp.h cc++/rtp.h
 *
 * Extends the RTP queue with the management of RTCP functions:
 *
 * <ul>
 * <li> Provide feedback on the quality of the data distribution.
 * </li>
 * <li> Convey the CNAME for every RTP source.
 * </li>
 * <li> Control the sending rate of RTCP packets
 * </li>
 * <li> Convey minimal control information about the participants
 * </li>
 * </ul>
 *
 * @author Federico Montesino <p5087@quintero.fie.us.es>
 * @short Management of RTCP functions.  
 */
class CCXX_CLASS_EXPORT QueueRTCPManager : public RTPQueue
{
public:
	/**
	 * Specify the bandwith available for control (RTCP) packets.  
	 *
	 * @param bw fraction of the session bandwidth, between 0 and 1 
	 * 
	 * This method sets the global control bandwidth for both
	 * sender and receiver reports. As recommended in (RFC XXXX -
	 * RTP is currently in transition from Proposed to Draft
	 * Standard), 1/4 of the total control bandwidth is dedicated
	 * to senders, whereas 3/4 are dedicated to receivers. 
	 *
	 * @note If this method is not called, it is assumed that the
	 * control bandwidth is equal to 5% of the session bandwidth.
	 * 
	 * @see setSessionBandwidth
	 * @see setSendersControlFraction 
	 */
	virtual void
	setControlBandwidth(float fraction)
	{ controlbw = fraction; };

	/**
	 * Specify the fraction of the total control bandwith to be 
	 * dedicated to senders reports.  
	 *
	 * @param fraction fraction of bandwidth, must be between 0 an 1. 
	 * 
	 * This method sets the fraction of the global control
	 * bandwidth that will be dedicated to senders reports. Of
	 * course, <code>1 - fraction</code> will be dedicated to
	 * receivers reports.
	 * 
	 * @see setControlBandwidth
	 */
	virtual void
	setSendersControlFraction(float fraction)
	{ sendcontrolbw = fraction; recvcontrolbw = 1 - fraction;};

	/**
	 * Get the total number of RTCP packets sent until now
	 *
	 * @param total number of RTCP packets sent
	 */
	inline uint32
	RTCPSendCount() const
	{ return ctrlsendcount; }

	/**
	 * Minimal control information about the local participant
	 *
	 */
 	void inline
	setNAME(const char* const name)
	{ localsrc->setSDESItem(RTCP_SDES_ITEM_NAME,name); };

	void inline 
	setEMAIL(const char* const email)
	{ localsrc->setSDESItem(RTCP_SDES_ITEM_EMAIL,email); };

	void inline setPHONE(const char* const phone)
	{ localsrc->setSDESItem(RTCP_SDES_ITEM_PHONE,phone); };

	void inline 
	setLOC(const char* const loc) 
	{ localsrc->setSDESItem(RTCP_SDES_ITEM_LOC,loc); };

	void inline
	setTOOL(const char* const tool)
	{ localsrc->setSDESItem(RTCP_SDES_ITEM_TOOL,tool); };

	void inline 
	setNOTE(const char* const note)
	{ localsrc->setSDESItem(RTCP_SDES_ITEM_NOTE,note); };

	void inline 
	setPRIV(const char* const priv)
	{ localsrc->setSDESItem(RTCP_SDES_ITEM_PRIV,priv); };

	void inline 
	setH323_CADDR(const char* const h323ca)
	{ localsrc->setSDESItem(RTCP_SDES_ITEM_H323_CADDR,h323ca); };

protected:
	QueueRTCPManager(int pri);

	virtual 
	~QueueRTCPManager();

	void
	endQueueRTCPManager();

	/**
	 * A scheduler of RTCP packets. 
	 * 
	 * @param wait queue scheduling timeout
	 */
	void 
	RTCPService(microtimeout_t& wait);

	/**
	 * Try to post a BYE message. It will send a BYE packet as
	 * long as at least one RTP or RTCP packet has been sent
	 * before. If the number of members in the session is more
	 * than 50, the algorithm described in section 6.3.7 of RFC
	 * ???? is applied in order to avoid a flood of BYE messages.
	 */
	void 
	Bye(const char* const reason = NULL);

	/**
	 * A plugin point for sdes contact.
	 */
	virtual void
	gotHello(RTPSource &src)
	{ return; }
 
 	/**
 	 * A plugin point for goodbye message.
 	 */
 	virtual void 
	gotGoodbye(RTPSource &src, char *reason)
	{ return; }

	/**
	 *
	 */
	void
	handleSSRCCollision();

	/**
	 * Computes the interval for sending RTCP compound packets,
	 * based on the average size of RTCP packets sent and
	 * received, and the current estimated number of participants
	 * in the session.
	 *
	 * @note This currently follows the rules in section 6 of
	 *       draft-ietf-avt-rtp-new-10
	 * @todo make it more flexible as recommended in the draft
	 *
	 * @return interval for sending RTCP compound packets
	 */
	virtual timeval 
	computeRTCPInterval();

private:
	QueueRTCPManager(const QueueRTCPManager &o);

	QueueRTCPManager&
	operator=(const QueueRTCPManager &o);

	/**
	 *
	 */
	void 
	setSDESItem(sdes_item_type_t type, const char *const value);

	/**
	 * Find out the local CNAME as user@host and store it as part
	 * of the internal state of this class.  
	 */
	void
	findCNAME();

	/**
	 * 
	 * @param len length in octets of the last RTCP packet received
	 */
	void
	updateAvgRTCPSize(uint16 len);

	/**
	 * Apply reverse reconsideration adjustment to timing
	 * parameters when receiving BYE packets and not waiting to
	 * send a BYE. 
	 */
	void 
	ReverseReconsideration();

	/**
	 *
	 *
	 */
	bool
	TimerReconsideration();

	/**
	 * Purge sources that do not seem active any more.
	 *
	 * @note MUST be perform at least every RTCP transmission
	 *       interval
	 * @todo implement it 
	 * */
	void 
	TimeOutSSRCs();

	/**
	 *
	 * @return whether there was a CNAME SDES item
	 */
	bool
	getSDES_APP(RTCPPacket &pkt, uint16 &pointer, uint16 len);

	/**
	 * Process a BYE packet just received and identified.
	 *
	 * @param pkt previously identified RTCP BYE packet 
	 * @param pointer octet number in the RTCP reception buffer
	 *        where the packet is stored 
	 * @param len total length of the compount RTCP packet the BYE 
	 *        packet to process is contained
	 */
	bool
	getBYE(RTCPPacket &pkt, uint16 &pointer, uint16 len);

	/**
	 *
	 */
	void
	getOnlyBye();

	/**
	 * For certain control calculations in RTCP, the size of the
	 * underlying network and transport protocols is needed. This
	 * method provides the size of the network level header for
	 * the default case of IP (20 octets). In case other protocol
	 * with different header size is used, this method should be
	 * redefined in a new specialized class.
	 *
	 * @return size of the headers of the network level. IP (20) by
	 *        default. 
	 */
	inline virtual uint16
	networkHeaderSize()
	{ return 20; }

	/**
	 * For certain control calculations in RTCP, the size of the
	 * underlying network and transport protocols is needed. This
	 * method provides the size of the transport level header for
	 * the default case of UDP (8 octets). In case other protocol
	 * with different header size is used, this method should be
	 * redefined in a new specialized class.
	 *
	 * return size of the headers of the transport level. UDP (8)
	 *        by default 
	 */
	inline virtual uint16
	transportHeaderSize()
	{ return 8; }

	/**
	 * Posting of RTCP messages.
	 *
	 * @return size_t number of octets sent
	 */
	size_t
	sendControl(void);

	/**
	 * For picking up incoming RTCP packets if they are waiting. A
	 * timeout for the maximum interval since the last RTCP packet
	 * had been received is also returned. This is checked once a
	 * second.
	 * 
	 * @return number of microseconds since last RTCP arrival.  
	 * @todo update doc and clarify purpose
	 */
	void
	recvControl(void);

	/**
	 *
	 *
	 * @return whether there were data to pack a report block or there
	 * are no more data
	 */
	bool
	packReportBlock(uint16& len);

	/**
	 * Try to pack a new RR in the RTCP compound packet currently
	 * being built.  After having packed one or more SR or RR to
	 * send in an RTCP compound packet, checks if there are more
	 * reports to send and there is room enough in the current
	 * packet to include them. If so, the header for a new RR is
	 * appended together with the first report block, and true is
	 * returned.
	 * 
	 * @param fh Header of the current SR/RR
	 * @param len provisionary length of the RTCP compound packet
	 * @param blocks number of blocks for the next packet
	 * @return true if a new RR has been appended to the current 
	 *         RTCP compound packet. 
	 */
	bool
	tryAnotherRR(RTCPPacket*& pkt, uint16& len, uint16& blocks);

	/**
	 *
	 * @param len provisionary length of the RTCP compound packet 
	 *
	 * @return
	 */
	void 
	packSDES(uint16& len);

	size_t
	sendBYE(const char* const reason);

	/**
	 *
	 * @return
	 */
	virtual size_t 
	writeControl(const unsigned char* const buffer, size_t len) = 0;

	/**
	 *
	 * @return
	 */
	virtual size_t 
	readControl(unsigned char* buffer, size_t len) = 0;

	/**
	 *
	 * @param timeout
	 * @return 
	 */
	virtual bool 
	isPendingControl(microtimeout_t timeout) = 0;
	
	/**
	 *
	 * @param pos header position in the RTCP recepction buffer
	 * @param length length of the RTCP compound packet in 
	 *        the reception buffer
	 *
	 * @return
	 */
	bool
	RTCPHeaderCheck(size_t len);


	// path MTU. RTCP packets should not be greater than this
	uint16 pathMTU;

	// buffer to hold RTCP compound packets being sent. Allocated
	// at construction time
	unsigned char* rtcpsend_buffer;

	// buffer to hold RTCP compound packets being
	// received. Allocated at construction time
	unsigned char* rtcprecv_buffer;

	// whether the RTCP service is active
	bool rtcp_active;

	float controlbw, sendcontrolbw, recvcontrolbw;
	// number of RTCP packets sent since the beginning
	uint32 ctrlsendcount;

	// Network + transport headers size, typically size of IP +
	// UDP headers
	uint16 lower_headers_size;
	// state for rtcp timing. Its meaning is defined in RFC ????, 6.3
	timeval rtcp_tp, rtcp_tc, rtcp_tn;
	uint32 rtcp_pmembers;
	uint32 rtcp_bw;
	bool rtcp_we_sent;	
	uint16 rtcp_avg_size;
	bool rtcp_initial;
	// last time we checked if there were incoming RTCP packets
	timeval rtcp_last_check;
	// interval to check if there are incoming RTCP packets
	timeval rtcp_check_interval;
	// next time to check if there are incoming RTCP packets
	timeval rtcp_next_check;

	// value of sendcount at the time of the last RTCP packet transmission
	uint32 last_sendcount;

	uint32 prev_nvalid_sources;
	timeval rtcp_calculated_interval;
	
	// length of CNAME as user@host
	size_t CNAME_len;

   	// According to timer reconsideration and reverse reconsideration 
	// algorithms, sources do not become valid or deleted as soon as
	// they appear or send BYE. 
	// number of not yet valid, but noticed sources 
	uint32 nprevalid_srcs;
	// number of valid sources
	uint32 nvalid_srcs;
	// number of sources having sent a BYE, but not yet deleted
	uint32 npredeleted_srcs;
	// number of already deleted sources
	uint32 ndeleted_srcs;
	// minimum interval for transmission of RTCP packets
	microtimeout_t rtcp_min_interval;

	// Nember of seconds ellapsed from 1900 to 1970
	const static uint32 NTP_EPOCH_OFFSET = static_cast<uint32>(2208992400u);

	// masks for RTCP header validation
#if	__BYTE_ORDER == __BIG_ENDIAN
	static const uint16 RTCP_VALID_MASK = (0xc000 | 0x2000  | 0xfe);
	static const uint16 RTCP_VALID_VALUE = ((RTP_VERSION << 14) | 
						RTCP_TYPE_SR);
#else
	static const uint16 RTCP_VALID_MASK = (0x00c0 | 0x0020 | 0xfe00);
	static const uint16 RTCP_VALID_VALUE = ((RTP_VERSION << 6) | 
						(RTCP_TYPE_SR << 8));
#endif
	static const uint16 TIMEOUT_MULTIPLIER = 5;
	static const double RECONSIDERATION_COMPENSATION = 2.718281828 - 1.5;
};

/**
 * @class UDPIPv4Socket 
 *
 * Wrapper for UDPSocket that provides the physical I/O related
 * methods needed by the data or control connection of an RTP stack,
 * based on UDP and IPv4. It should be "straightforward" defining
 * wrappers like this for other underlying protocols, and then
 * instantiating the template T_RTPSocket for them.
 *
 * @author Federico Montesino <p5087@quintero.fie.us.es>
 * @short Socket for RTP stack based on UDP and IPv4.
 */
class CCXX_CLASS_EXPORT UDPIPv4Socket: protected UDPSocket 
{
public:
	/**
	 * Constructor.
	 * 
	 * @param bind network address this socket is to be bound 
	 * @param port transport port this socket is to be bound
	 */
	UDPIPv4Socket(const InetAddress& ia, tpport_t port) : 
		UDPSocket(ia, port) 
	{ };

	/**
	 * Destructor.
	 */
	~UDPIPv4Socket() 
	{ };

	/**
	 * Connect to a foreign socket.
	 *
	 * @param ia network address to connect to
	 * @param port transport port to connect to
	 */
	sockerror_t
	Connect(const InetAddress& ia, tpport_t port);
	
	/**
	 *
	 */
	inline bool 
	isPendingPacket(microtimeout_t timeout)
	{ return isPending(SOCKET_PENDING_INPUT, timeout/1000);};
	
	/**
	 *
	 */
        inline size_t 
	writePacket(const unsigned char* const buffer, size_t len)
	{return ::send(so, buffer, len, MSG_DONTWAIT);};

	/**
	 *
	 */
        inline size_t
	readPacket(unsigned char *buffer, size_t len)
	{return ::read(so, buffer, len);};

	/**
	 *
	 */
	inline sockerror_t 
	setMulticast(bool enable)
	{ return setMulticast(enable); };

	/**
	 * Join a multicast group. 
	 *
	 * @param ia multicast group address 
	 * @return error code from the socket operation
	 */
	inline sockerror_t
	joinGroup(const InetMcastAddress& ia)
	{ return Join(ia); };

	/**
	 * Leave a multicast group. 
	 * 
	 * @param ia multicast group address  
	 * @return error code from the socket operation
	 */
	inline sockerror_t
	leaveGroup(const InetMcastAddress& ia)
	{ return Drop(ia); }; 

	/**
	 * Set the value of the TTL field in the packets to send.
	 *
	 * @param ttl Time To Live
	 * @return error code from the socket operation
	 */
	inline sockerror_t
	setMcastTTL(uint8 ttl)
	{ setTimeToLive(ttl); };

	/**
	 * End socket, terminating the socket connection.
	 */
	void 
	endSocket()
	{ UDPSocket::endSocket(); };
};

/**
 * @class T_RTPSocket rtp.h cc++/rtp.h
 *
 * Generic RTP protocol stack for exchange of realtime data.  This
 * stack uses the concept of packet send and receive queues to schedule
 * and buffer outgoing packets and to arrange or reorder incoming packets
 * as they arrive.  A single service thread both schedules sending of
 * outgoing packets and receipt of incoming packets.
 *
 * @author David Sugar <dyfet@ostel.com>
 * @short RTP protocol stack based on Common C++.
 */
template <typename serviceQueue, typename dataSocket, typename controlSocket>
class __EXPORT T_RTPSocket  : public serviceQueue
{
public:
	/**
	 * @param bind network address this socket is to be bound
	 * @param port transport port this socket is to be bound
	 * @param pri service thread base priority relative to it's parent
	 * */
	T_RTPSocket(const InetAddress& ia, tpport_t port = 5004, int pri = 0) :
		serviceQueue(pri)
	{
		base = even_port(port);
		dso = new dataSocket(ia,even_port(port));
		cso = new controlSocket(ia,odd_port(port + 1));
	};
	
	/**
	 * @param bind multicast network address this socket is to be bound
	 * @param port transport port this socket is to be bound
	 * @param pri service thread base priority relative to it's parent
	 * */
	T_RTPSocket(const InetMcastAddress& ia, tpport_t port = 5004, int pri = 0)
	{
		base = even_port(port);
		dso = new dataSocket((InetAddress)ia,even_port(port));
		cso = new controlSocket((InetAddress)ia,odd_port(port + 1));
	};

	/**
	 * Stack destructor.
	 * */
	~T_RTPSocket()
	{ endSocket(); };

	/**
	 * Connect to a foreign host and start the service thread. If
	 * no port is specified then it is assumed to be the same as
	 * the locally bound port number.
	 * */
	inline sockerror_t 
	Connect(const InetHostAddress& ia, tpport_t port = 0)
	{ return connect(ia,port); };

	/**
	 * Connect to a multicast group and start the service
	 * thread. If no port is specified then it is assumed to be
	 * the same as the locally bound port number.
	 * */
	inline sockerror_t 
	Connect(const InetMcastAddress& ia, tpport_t port = 0)
	{ return connect(ia,port); };

	/**
	 * Join a multicast group. 
	 *
	 * @param ia address of the multicast group
	 * @return error code from the socket operation
	 */
	inline sockerror_t
	joinGroup(const InetMcastAddress& ia, tpport_t port = 0)
	{ 
		sockerror_t error  = dso->setMulticast(true);
		if ( error ) return error;
		error = dso->joinGroup(ia); 
		if ( error ) return error;
		error = cso->setMulticast(true);
		if ( error ) {
			dso->leaveGroup(ia);
			return error;
		}
		error = cso->joinGroup(ia);
		if ( error ) {
			dso->leaveGroup(ia);
			return error;
		}
		return Connect(ia,port);
	};

	/**
	 * Leave a multicast group. 
	 * 
	 * @param ia address of the multicast group
	 * @return error code from the socket operation
	 */
	inline sockerror_t
	leaveGroup(const InetMcastAddress& ia)
	{ 
		sockerror_t error = dso->setMulticast(true);
		if ( error ) return error;
		error = dso->leaveGroup(ia); 
		if ( error ) return error;
		error = cso->setMulticast(true);
		if ( error ) return errror;
		return cso->leaveGroup(ia);
	};

	/**
	 * Set the value of the TTL field in the sent packets.
	 *
	 * @param ttl Time To Live
	 * @return error code from the socket operation
	 */
	inline sockerror_t
	setMcastTTL(uint8 ttl)
	{ 
		sockerror_t error = dso->setMulticast(true);
		if ( error ) return error;
		error = dso->setTimeToLive(ttl); 
		if ( error ) return error;
		error = cso->setMulticast(true);		
		if ( error ) return error;		
		return cso->setTimeToLive(ttl);
	};
	
protected:
	/**
	 * @param timeout maximum timeout to wait, in microseconds
	 */
	inline bool 
	isPendingData(microtimeout_t timeout)
	{ return dso->isPendingPacket(timeout); };

	/**
	 * @param buffer memory region to read to
	 * @param len maximum number of octets to read
	 */
	inline size_t 
	readData(unsigned char* buffer, size_t len)
	{ return dso->readPacket(buffer, len); };

	/**
	 * @param buffer memory region to write from
	 * @param len number of octets to write
	 */
	inline size_t 
	writeData(const unsigned char* const buffer, size_t len)
	{ return dso->writePacket(buffer, len); };

	/**
	 * @param timeout maximum timeout to wait, in microseconds
	 * @return whether there are packets waiting to be picked
	 */
        inline bool 
	isPendingControl(microtimeout_t timeout)
	{ return cso->isPendingPacket(timeout); };

	/**
	 * @return number of octets actually read
	 * @param buffer
	 * @param len
	 */
        inline size_t 
	readControl(unsigned char *buffer, size_t len)
	{ return cso->readPacket(buffer,len); };

	/**
	 * @return number of octets actually written
	 * @param buffer
	 * @param len
	 */
        inline size_t
	writeControl(const unsigned char* const buffer, size_t len)
	{ return cso->writePacket(buffer,len); };

	inline void
	endSocket()
	{ dso->endSocket(); cso->endSocket(); };

private:
	/**
	 * Connect to a foriegn RTP socket and start the service
	 * thread.  If no port is specified then it is assumed to
	 * be the same as the locally bound port number.
	 *
	 * @return failure type.
	 * @param address of foreign socket.
	 * @param port number of foreign connection.  */
	sockerror_t 
	connect(const InetAddress& ia, tpport_t port = 0)
	{
		sockerror_t error;
		if ( !port )
			port = base;
		if ( active )
			active = false;
		// make both RTP (even) and RTCP (odd) connections
		error = dso->Connect(ia, even_port(port));
		if ( error )
			return error;
		error = cso->Connect(ia, odd_port(port + 1));
		if ( error )
			return error;
		// Start running the RTP queue service thread
		active = true;
		Start();  
		return SOCKET_SUCCESS;
	};

	/**
	 * Ensure a port number is odd. If it is an even number, return 
	 * the next lower (odd) port number.
	 *
	 * @param port number to filter
	 * @return filtered (odd) port number
	 */
	inline tpport_t
	odd_port(tpport_t port)
	{ return (port & 0x01)? (port) : (port - 1); };
	
	/**
	 * Ensure a port number is even. If it is an odd number, return
	 * the next lower (even) port number.
	 *
	 * @param port number to filter
	 * @return filtered (even) port number
	 */
	inline tpport_t 
	even_port(tpport_t port)
	{ return (port & 0x01)? (port - 1) : (port); };

	tpport_t base;
	dataSocket* dso;
	controlSocket* cso;
};

/**
 * @typedef RTPSocket rtp.h cc++/rtp.h
 *
 * @short RTP protocol stack with RTCP support, based on IPv4/UDP
 * sockets for both data and control.  
 */
typedef T_RTPSocket<QueueRTCPManager,UDPIPv4Socket,UDPIPv4Socket> RTPSocket;

/**
 * @class RTPDuplex rtp.h cc++/rtp.h 
 *
 * A peer associated RTP socket pair for physically connected peer
 * hosts.  This has no RTCP and assumes the receiver is connected
 * to a known transmitter, hence no "foreign" packets will arrive.
 *
 * @author David Sugar
 * @short RTP peer host over UDP.
 */
class CCXX_CLASS_EXPORT RTPDuplex : public RTPQueue, protected UDPReceive, public UDPTransmit
{

public:

	/**
	 * @param bind network address this socket is to be bound 
	 * @param local transport port this socket is to be bound
	 * @param remote peer transpor port
	 * @param pri service thread base priority relative to it's parent
	 */
	RTPDuplex(const InetAddress &bind, tpport_t local, tpport_t remote, int pri);

	/**
	 *
	 */
	virtual
	~RTPDuplex();

	/**
	 * @param host peer address 
	 * @param port peer port. If not specified, the same as the 
	 *        local is used
	 * @return socket status
	 */
	sockerror_t 
	Connect(const InetHostAddress &host, tpport_t port = 0);

protected:

	/**
	 * @param timeout how much time to wait for new data
	 * @return if there is some new data
	 */
	bool 
	isPendingData(microtimeout_t timeout)
	{ return isPendingReceive(timeout); }

	/**
	 * @param buffer pointer to data to be written
	 * @param len how many octets to write
	 * @return number of octets written
	 */
	size_t 
	writeData(const unsigned char *const buffer, size_t len)
	{ return Transmit((const char *)buffer, len); }

	/**
	 * @param buffer where to store the retrieved data 
	 * @param len how many octets to read
	 * @return number of octets read
	 */
	size_t 
	readData(unsigned char *buffer, size_t len)
	{ return Receive(buffer, len); }

	/**
	 * @return the associated peer information
	 */
	RTPSource &getPeer();

private:

	tpport_t base;
};

#ifdef	__NAMESPACES__
};
#endif

#endif  //__CCXX_RTP_H__

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
