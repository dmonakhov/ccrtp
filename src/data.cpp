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
// RTPData class implementation
//
#include "private.h"

RTPData::dataCounter::dataCounter(const unsigned char *data, size_t size, rtp_payload_t pt): 
	count(1), data(data), size(size), pt(pt)
{
}

RTPData::dataCounter::~dataCounter()
{
	try {
		delete [] data;
	} catch (...) {}
}

RTPData::RTPData(IncomingRTPPkt& packet) :
	datablock( new dataCounter(packet.getRawPacket(),
		packet.getRawPacketSize(),packet.getPayloadType()) ),
	src(&(packet.getSource()))
{

}

RTPData::RTPData(const RTPData &origin): 
	datablock(origin.datablock),
	src(origin.src)
{
	++datablock->count;
	
}

RTPData::~RTPData()
{
	if ( --datablock->count == 0 )
		try {
			delete datablock;
		} catch (...) { }
}

RTPData &
RTPData::operator=(const RTPData &source)
{
	// autoassignment
	if ( datablock == source.datablock )
		return *this;
	
	// remove the last "soft link"
	if ( --datablock->count == 0 )
		delete datablock;

	datablock = source.datablock;
	++datablock->count;

	return *this;
}

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */

