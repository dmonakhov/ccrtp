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
// RTPPacket, OutgoingRTPPkt and IncomingRTPPkt classes implementation
//
#include "private.h"

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

// constructor commonly used for incoming packets
RTPPacket::RTPPacket(const unsigned char* const block, size_t len, 
		     bool duplicate):
	total(len),
	duplicated(duplicate)
{

	const RTPFixedHeader* const header = 
		reinterpret_cast<const RTPFixedHeader*>(block);
	hdrsize = sizeof(RTPFixedHeader) + (header->cc << 2);
	if ( header->extension ){
		RTPHeaderExt *ext = (RTPHeaderExt *)(block + hdrsize);
		hdrsize += sizeof(uint32) + ntohl(ext->length);
	}
	if ( header->padding ) 
		len -= block[len - 1];
	payload_size = len - hdrsize;
	
	if ( duplicate ) {
		buffer = new unsigned char[len];
		setbuffer(block,len,0);
	} else {
		buffer = const_cast<unsigned char*>(block);
	}
}
	
// constructor commonly used for outgoing packets
RTPPacket::RTPPacket(size_t hdrlen, size_t plen) :
	buffer(NULL),
	hdrsize(hdrlen),
	payload_size(plen),
	duplicated(false)
{
	uint8 padding = 0;
	total = hdrlen + payload_size;
	if ( total & 0x03 ) {
		padding = 4 - (total & 0x03);
		total += padding;
	}
	buffer = new unsigned char[total];
	if ( padding )
		memset(buffer + total - padding,0,padding);
	buffer[total - 1] = padding;

	if ( padding )
		const_cast<RTPFixedHeader*>(getHeader())->padding = 1;
	else
		const_cast<RTPFixedHeader*>(getHeader())->padding = 0;

	const_cast<RTPFixedHeader*>(getHeader())->version = CCRTP_VERSION;
}

void 
RTPPacket::endPacket()
{
	try {
		delete [] buffer;
	} catch (...) { };
}

OutgoingRTPPkt::OutgoingRTPPkt(
	const uint32* const csrcs, uint16 numcsrc, 
        const unsigned char* const hdrext, uint32 hdrextlen,
	const unsigned char* const data, uint32 datalen) :
	RTPPacket((sizeof(RTPFixedHeader) + sizeof(uint32) * numcsrc + hdrextlen),datalen),
	next(NULL), prev(NULL)
{
	uint32 pointer = sizeof(RTPFixedHeader);
	setbuffer(csrcs,numcsrc * sizeof(uint32),pointer);
	pointer += numcsrc * sizeof(uint32);
	setbuffer(hdrext,hdrextlen,pointer);
	pointer += hdrextlen;
	setbuffer(data,datalen,pointer);

	const_cast<RTPFixedHeader*>(getHeader())->cc = numcsrc;
	const_cast<RTPFixedHeader*>(getHeader())->extension = (hdrextlen > 0);
}

OutgoingRTPPkt::OutgoingRTPPkt(
	const uint32* const csrcs, uint16 numcsrc, 
	const unsigned char* data, uint32 datalen) :
	RTPPacket((sizeof(RTPFixedHeader) + sizeof(uint32) *numcsrc),datalen),
	next(NULL), prev(NULL)
{
	uint32 pointer = sizeof(RTPFixedHeader);
	setbuffer(csrcs,numcsrc * sizeof(uint32),pointer);
	pointer += numcsrc * sizeof(uint32);
	setbuffer(data,datalen,pointer);

	const_cast<RTPFixedHeader*>(getHeader())->cc = numcsrc;
	const_cast<RTPFixedHeader*>(getHeader())->extension = 0;
}

OutgoingRTPPkt::OutgoingRTPPkt(const unsigned char* data, uint32 datalen) :
	RTPPacket(sizeof(RTPFixedHeader),datalen),
	next(NULL), prev(NULL)
{
	setbuffer(data,datalen,sizeof(RTPFixedHeader));

	const_cast<RTPFixedHeader*>(getHeader())->cc = 0;
	const_cast<RTPFixedHeader*>(getHeader())->extension = 0;
}

OutgoingRTPPkt::~OutgoingRTPPkt()
{ 
}

IncomingRTPPkt::IncomingRTPPkt(RTPQueue &queue, const unsigned char* const block, size_t len, timeval recvtime):
	RTPPacket(block,len),
	next(NULL), prev(NULL), srcnext(NULL), srcprev(NULL),
	source(queue.getSourceBySSRC(this->getSSRC(),true)), 
	reception_timestamp(recvtime)
{
	// first, perform validity check
	// check protocol version
	if ( getHeader()->version != CCRTP_VERSION ) {
		valid = false;
		return;
	}	
	// check it is not a SR or an RR
	if ( (getPayloadType() & RTP_INVALID_MASK) == RTP_INVALID_VALUE ) {
		valid = false;
		return;
	}
	// check valid length: take into account header extension and padding
	if ( getHeaderSize() >= len ) {
		valid = false;
		return;
	}

	valid = true;
	// account the reception and validation of this new packet
	source.recordReception(*this);
	cached_timestamp = getRawTimestamp() - source.getInitialTimestamp();
}

IncomingRTPPkt::~IncomingRTPPkt()
{
};

#ifdef	CCXX_NAMESPACES
};
#endif

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
