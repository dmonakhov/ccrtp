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
 * @file rtpext.h 
 * @brief Declarations for low level filtering and extensions.
 *
 * This header includes miscellaneous declarations needed to do low
 * level packet filtering or to extend the scheduled queue.
 **/

#ifndef  CCXX_RTPEXT_H
#   define   CCXX_RTPEXT_H

#ifdef  CCXX_NAMESPACES
namespace ost {
#endif

/**
 * @class RTPPacket rtpext.h cc++/rtpext.h 
 *
 * @short A base class for both IncomingRTPPkt and OutgoingRTPPkt.
 *
 * Provides common low level header structures and related methods.
 *
 * @author David Sugar <dyfet@ostel.com>
 **/
class CCXX_CLASS_EXPORT RTPPacket
{
protected:
#pragma pack(1)
	/**
	 * @struct RTPFixedHeader private.h "private.h"
	 *
	 * @short RTP fixed header as it is send through the
	 * network
	 * 
	 * A low-level representation for generic RTP packet header as
	 * defined in RFC 1889. A packet consists of the fixed RTP
	 * header, a possibly empty list of contributing sources and
	 * the payload. Header contents are kept in network (big
	 * endian) order.  
	 */
	struct RTPFixedHeader
	{
#if	__BYTE_ORDER == __BIG_ENDIAN
		///< For big endian boxes
		unsigned char version:2;       ///< Version, currently 2
		unsigned char padding:1;       ///< Padding bit
		unsigned char extension:1;     ///< Extension bit
		unsigned char cc:4;            ///< CSRC count 
		unsigned char marker:1;        ///< Marker bit
		unsigned char payload:7;       ///< Payload type
#else
		///< For little endian boxes
		unsigned char cc:4;            ///< CSRC count 
		unsigned char extension:1;     ///< Extension bit
		unsigned char padding:1;       ///< Padding bit
		unsigned char version:2;       ///< Version, currently 2
		unsigned char payload:7;       ///< Payload type
		unsigned char marker:1;        ///< Marker bit
#endif
		uint16 sequence;       ///< sequence number
		uint32 timestamp;       ///< timestamp
		uint32 sources[1];      ///< contributing sources
	};
	
	/**
	 * @struct RTPHeaderExt private.h "private.h"
	 *
	 * Fixed component of the variable-length header extension,
	 * appended to the fixed header, after the CSRC list, 
	 * when X == 1.
	 * */
	typedef struct
	{
		uint16 undefined; ///< to be defined 
		uint16 length;    ///< number of 32-bit words in the extension
	}       RTPHeaderExt;
#pragma pack()

	// size of the header, including contributing sources and extensions
	uint32 hdrsize;
	// note: payload (not full packet) size.
	uint32 payload_size;
	// total length, including header, extension, payload and padding
	uint32 total;
	// packet in memory
	unsigned char* buffer;
	// whether the object was contructed with duplicated = true
	bool duplicated;

public:
	/**
	 * Constructor, construct a packet object given the memory
	 * zone its content is stored.  
	 *
	 * @param block whole packet
	 * @param len total length (header + payload + padding) of the 
	 *        packet 
	 * @param duplicate whether to memcopy the packet. At present,
	 *        this feature is not used.
	 * @note used in IncomingRTPPkt. 
	 * */
	RTPPacket(const unsigned char* const block, size_t len, 
		  bool duplicate = false);

	/**
	 * Constructor, construct a packet object without specifying
	 * its real content yet.
	 *
	 * @param hdrlen length of the header (including CSRC and extension)
	 * @param len payload length. 
	 * @note used in OutgoingRTPPkt. 
	 */
	RTPPacket(size_t hdrlen, size_t plen);

	/**
	 * Destructor, free the buffer provided in the constructor.
	 */
	//	virtual
	~RTPPacket()
	{ endPacket(); };
	
	/**
	 * Return low level structure for the header of the packet.
	 *
	 * @return RTPFixedHeader pointer to the header of the packet
	 */
	inline const RTPFixedHeader*
	getHeader(void) const
	{ return reinterpret_cast<const RTPFixedHeader*>(buffer); };
	
	/**
	 * Obtain the length of the header, including contributing
	 * sources and header extension, if present.
	 *
	 * @return number of octets 
	 */
	inline uint32
	getHeaderSize(void) const
	{ return hdrsize; };

	/**
	 * Get a pointer to RTPHeaderExt pointing after the RTP header
	 * (fixed part plus contributing sources). No check for
	 * for the X bit is done. 
	 * 
	 * @return header extension if present, garbage if not.
	 */
	inline const RTPHeaderExt*
	getHeaderExt() const
	{ uint32 fixsize = sizeof(RTPFixedHeader) + 
		  (getHeader()->cc << 2); 
	  return (reinterpret_cast<RTPHeaderExt*>(buffer + fixsize));
	} 

	/**
	 *
	 * @return pointer to the payload section of the packet
	 */
	inline const unsigned char* const
	getPayload(void) const
	{ return buffer + getHeaderSize(); };

	/**
	 * @return lenght of the payload section, in octets
	 * */
	inline uint32
	getPayloadSize() const
	{ return payload_size; };

	/**
	 * @return value of the PT header field 
	 */
	inline rtp_payload_t 
	getPayloadType() const
	{ return static_cast<rtp_payload_t>(getHeader()->payload); };

	/**
	 * @return value of the sequence number header field
	 */
	inline uint16
	getSeqNum() const
	{ return ntohs(getHeader()->sequence); };
	
	/**
	 * Ask whether the packet contains padding bytes at the end
	 * @return true if the header padding bit is 1
	 */
	inline bool
	isPadded() const
	{ return getHeader()->padding; };

	/**
	 * Get the number of octets padding the end of the payload
	 * section.
	 * @param padding length in octets
	 * */
	inline uint8
	getPaddingSize() const
	{ return buffer[total - 1]; };

	/**
	 * Ask whether the packet is marked (for isntance, is a new
	 * talk spurt in some audio profiles) 
	 * @return true is the header marker bit is 1 
	 * */
	inline bool
	isMarked() const
	{ return getHeader()->marker; };

	/**
	 * Ask whether the packet contains header extensions
	 * @return true if the header extension bit is 1
	 * */
	inline bool
	isExtended() const
	{ return getHeader()->extension; };

	/**
	 * Get the number of contributing sources specified in the
	 * packet header.
	 * */
	inline uint16
	getCSRCsCount() const
	{ return getHeader()->cc; };

	/**
	 * Get the 32-bit identifiers of the contributing sources for
	 * the packet as an array, of length getCSRCsCount()
	 * */
	inline const uint32*
	getCSRCs() const
	{ return static_cast<const uint32*>(&(getHeader()->sources[1])); };

	/**
	 * Get the raw packet as it will be sent through the network
	 *
	 * @return memory zone where the raw packet structure is
	 *         stored in
	 * */
	inline const unsigned char* const
	getRawPacket() const
	{ return buffer; };

	/**
	 * Get the raw packet length, including header, extension,
	 * payload and padding.
	 *
	 * @return size of the raw packet structure
	 * */
	inline uint32
	getRawPacketSize() const
	{ return total; };

protected:
	/**
	 * Obtain the absolute timestamp carried in the packet header.
	 *
	 * @return 32-bit timestamp in host order 
	 */
	inline uint32
	getRawTimestamp(void) const
	{ return ntohl(getHeader()->timestamp); };

	inline void 
	setbuffer(const void* src, size_t len, size_t pos)
	{ memcpy(buffer + pos,src,len); }

	/**
	 * Free memory allocated for the packet 
	 * */
	void 
	endPacket();
};

/**
 * @class OutgoingRTPPkt rtpext.h cc++/rtpext.h
 *
 * @short A representation for RTP packets being sent.
 *
 * This class is intented to construct packet objects just before they
 * are inserted into the sending queue, so that they will be processed
 * in a understandable and format independent manner inside the stack.
 *
 * @author Federico Montesino Pouzols <p5087@quintero.fie.us.es> 
 */
class CCXX_CLASS_EXPORT OutgoingRTPPkt: public RTPPacket
{
public:
	/**
	 * Construct a new packet to be sent containing several
	 * contributing source identifiers, header extension and
	 * payload. A new copy in memory with all this components
	 * together and the fixed header is done.
	 *
	 * @param csrcs array of countributing source 32-bit identifiers
	 * @param numcsrc number of CSRC identifiers in the array
	 * @param hdrext whole header extension
	 * @param hdrextlen size of whole header extension, in octets
	 * @param data payload
	 * @param datalen payload length, in octets
	 *
	 * @note for efficiency purposes, although this constructor
	 *       is valid for all packets, two simpler others are
	 *       provided.
	 * */
	OutgoingRTPPkt::OutgoingRTPPkt(
	       const uint32* const csrcs, uint16 numcsrc, 
	       const unsigned char* const hdrext, uint32 hdrextlen,
	       const unsigned char* const data, uint32 datalen);

	/**
	 * Construct a new packet to be sent containing several
	 * contributing source identifiers, but no header extension. A
	 * new copy in memory with all this components and the fixed
	 * header together is done.
	 *
	 * @param csrcs array of countributing source 32-bit identifiers
	 * @param numcsrc number of CSRC identifiers in the array
	 * @param data payload
	 * @param datalen payload length, in octets
	 * */
	 OutgoingRTPPkt::OutgoingRTPPkt(
         	const uint32* const csrcs, uint16 numcsrc, 
		const unsigned char* const data, uint32 datalen);

	/*
	 * Construct a new packet to be sent containing no
	 * contributing source identifiers nor header extension. A new
	 * copy in memory is done prepending the fixed header.
	 *
	 * @param csrcs array of countributing source 32-bit identifiers
	 * @param numcsrc number of CSRC identifiers in the array
	 * @param data payload
	 * @param datalen payload length, in octets
	 * */
	OutgoingRTPPkt::OutgoingRTPPkt(
	       const unsigned char* const data, uint32 datalen);

	/**
	 * Destructor.   
	 */
	~OutgoingRTPPkt();
	
	/**
	 * @param pt packet payload type
	 */
	inline void
	setPayloadType(rtp_payload_t pt)
	{ const_cast<RTPFixedHeader*>(getHeader())->payload = pt; };

	/**
	 * @param packet sequence number in host order
	 */
	inline void
	setSeqNum(uint16 seq)
	{ const_cast<RTPFixedHeader*>(getHeader())->sequence = htons(seq); };

	/**
	 * @param packet timestamp in host order
	 */
	inline void
	setTimestamp(uint32 ts)
	{ const_cast<RTPFixedHeader*>(getHeader())->timestamp = 
		  htonl(ts); };

	/**
	 * Set synchronization source numeric identifier.
	 *
	 * @return 32-bits Synchronization SouRCe numeric
	 *         identifier in network order
	 */
	inline void 
	setSSRC(uint32 ssrc) const
	{ const_cast<RTPFixedHeader*>(getHeader())->sources[0] = ssrc; };

	/**
	 * @param mark value for the market bit
	 */
	inline void
	setMarker(bool mark)
	{ const_cast<RTPFixedHeader*>(getHeader())->marker = mark; };

	/**
	 * @return sampling instant of the first octet in the packet,
	 * in network order
	 */
	inline uint32
	getTimestamp() const
	{ return getRawTimestamp(); };

	/**
	 * 
	 */
	inline bool 
	operator==(const OutgoingRTPPkt &p) const
	{ return ( this->getSeqNum() == p.getSeqNum() ); }

	/**
	 *
	 */
	inline bool
	operator!=(const OutgoingRTPPkt &p) const
	{ return !( *this==p ); };

private:
	/**
	 * Copy constructor from objects of its same kind, declared
	 * private to avoid its use.  
	 */
	OutgoingRTPPkt(const OutgoingRTPPkt &o);
	
	/**
	 * Assignment operator from objects of its same kind, declared
	 * private to avoid its use.  
	 */
	OutgoingRTPPkt&
	operator=(const OutgoingRTPPkt &o);

	// prev/next in the sending list
	OutgoingRTPPkt *next, *prev;       

	friend class RTPQueue;
};	

/**
 * @class IncomingRTPPkt rtpext.h cc++/rtpext.h
 *
 * @short A representation for RTP packets received from other
 * participants.
 *
 * This class is intented to construct packet objects just after every
 * packet is received by the scheduled queue, so that they will be
 * processed in an understandable and format independent manner inside
 * the stack.
 *
 * @author Federico Montesino Pouzols <p5087@quintero.fie.us.es> 
 */
class CCXX_CLASS_EXPORT IncomingRTPPkt : public RTPPacket
{
public:
	/**
	 * Build an RTP packet, given the queue it is going to be
	 * inserted, its content, length and reception time. This
	 * constructor links the packet to its source and records the
	 * reception of the packet in the correponding source
	 * object. However, this constructor does not assume that the
	 * packet will be inserted in the queue. Header check, whose
	 * result can be consulted via isHeaderValid(), is also 
	 * performed.
	 *
	 * @param queue queue the packet would be inserted
	 * @param block pointer to the buffer the whole packet is stored in
	 * @param len length of the whole packet, expressed in octets
	 * @param recvtime time the packet has been received at 
	 *
	 * @note If check fails, the packet is not properly
	 *       constructed. checking isHeaderValid() is recommended 
         *       before using a new RTPPacket object.
	 * */
	IncomingRTPPkt(RTPQueue &queue, const unsigned char* block, 
		       size_t len, struct timeval recvtime);

	/**
	 * Destructor.  
	 */
	~IncomingRTPPkt();

	/**
	 * Get validity of this packet
	 * @return whether the header check performed at construction
	 *         time ended successfully.
	 * */
	inline bool
	isHeaderValid()
	{ return valid; }

	/**
	 * 
	 */
	inline bool 
	operator==(const IncomingRTPPkt &p) const
	{ return ( (this->getSeqNum() == p.getSeqNum()) && 
		   (this->getSSRC() == p.getSSRC()) ); }
	
	/**
	 *
	 */
	inline bool
	operator!=(const IncomingRTPPkt &p) const
	{ return !( *this == p ); };

	/**
	 * Get synchronization source numeric identifier.
	 *
	 * @return 32-bits Synchronization SouRCe numeric
	 *         identifier in network order
	 */
	inline uint32 
	getSSRC() const
	{ return static_cast<uint32>(getHeader()->sources[0]); };
	
	/**
	 * Get an object that provides information about the source of
	 * this packet.  
	 * @return object representing the source of the
	 *         packet
	 */
	inline RTPSource& 
	getSource() const
	{ return source; };

	/**
	 * Get timestamp of this packet. The timestamp of incoming
	 * packets is filtered so that the timestamp this method
	 * provides for the first packet received from every source
	 * starts from 0.
	 *
	 * @return 32 bit timestamp starting from 0 for each source.
	 */
	inline uint32
	getTimestamp() const
	{ return cached_timestamp; };

	/**
	 * Set the time this packet was received at.
	 *
	 * @param t time of reception.
	 * @note this has almost nothing to do with the 32-bit timestamp 
	 *       contained in the packet header.
	 */
	inline void 
	setRecvTimestamp(const timeval &t)
	{ reception_timestamp = t; }

	/**
	 * Get the time this packet was received at.
	 *
	 * @param t structure where to get the time of reception.
	 * @note this has almost nothing to do with the 32-bit timestamp 
	 *       contained in the packet header.
	 */
	inline timeval
	getRecvTimestamp() const
	{ return reception_timestamp; }

	/**
	 * Get the first 16 bits (in network order) of the header of
	 * the RTP header extension. Its meaning is undefined at this
	 * level.
	 *
	 * @return 0 if the packet has no header extension, otherwise
	 *         the first 16 bits of the header extension, in
	 *         network order. 
	 * 
	 * @note 0 could be a valid value for the first 16 bits, in
	 *         that case RTPPacket::isExtended() should be use. 
	 */
	inline uint16
	getExtUndefined() const
	{ return (isExtended()? getHeaderExt()->undefined : 0); };

	/**
	 * Get the length (in octets) of the data contained in the
	 * header extension. Note that this length does not include
	 * the four octets at the beginning of the header extension.
	 *
	 * @return 0 if the packet has no header extension, otherwise
	 *         the length.  
	 *
	 * @note 0 is a valid value for this field, so
	 *       RTPPacket::isExtended() should be use.  
	 */
	inline uint32
	getExtSize() const
	{ return (isExtended()? getHeaderExt()->length : 0); };

private:
	/**
	 * Copy constructor from objects of its same kind, declared
	 * private to avoid its use.  
	 */
	IncomingRTPPkt(const IncomingRTPPkt &ip);

	/**
	 * Assignment operator from objects of its same kind, declared
	 * private to avoid its use.  
	 */
	IncomingRTPPkt&
	operator=(const IncomingRTPPkt &ip);

	// prev/next in the general list
	IncomingRTPPkt *next, *prev;       
	// prev/next in the source specific list
	IncomingRTPPkt *srcnext, *srcprev; 
	// source of the packet
	RTPSource &source;
	// time this packet was received at
	struct timeval reception_timestamp;
	// header validity, checked at construction time
	bool valid;
	// timestamp of the packet in host order and after
	// substracting the initial timestamp for its source (it is an
	// increment from the initial timestamp).
	uint32 cached_timestamp;

	// masks for RTP header validation: type not matching SR nor RR 
	static const uint16 RTP_INVALID_MASK = (0x7e);
	static const uint16 RTP_INVALID_VALUE = (0x48);

	friend class RTPQueue;
	friend class RTPSource;
};	

#pragma pack(1)
/**
 *
 *
 */
typedef struct 
{
#if	__BYTE_ORDER == __BIG_ENDIAN
	///< For big endian boxes
	unsigned char version:2;       ///< Version, currently 2
	unsigned char padding:1;       ///< Padding bit
	unsigned char block_count:5;   ///< Number of RR, SR, or SDES chunks
#else
	///< For little endian boxes
 	unsigned char block_count:5;   ///< Number of RR, SR, or SDES chunks 
	unsigned char padding:1;       ///< Padding bit
	unsigned char version:2;       ///< Version, currently 2
#endif
	uint8 type;              ///< type of RTCP packet
	uint16 length;           ///< number of 32-bit words in the packet
}       RTCPFixedHeader;
#pragma pack()

#ifdef	CCXX_NAMESPACES
};
#endif

#endif //CCXX_RTPEXT_H

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
