/*
 Copyright (C) 2006 Werner Dittmann

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef _ZRTPPACKETCON2FACK_H_
#define _ZRTPPACKETCON2FACK_H_

#include "ZrtpPacketBase.h"

/**
 * Implement the ClearAck packet.
 *
 * The ZRTP simple message ClearAck. The implementation sends this
 * after switching to clear mode (non-SRTP mode).
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */
class ZrtpPacketClearAck : public ZrtpPacketClearAck {

 public:
     ZrtpPacketClearAck();		/* Creates a Conf2Ack packet with default data */
     ZrtpPacketClearAck(char* data);	/* Creates a Conf2Ack packet from received data */
     virtual ~ZrtpPacketClearAck();

 private:
};

#endif // ZRTPPACKETCONF2ACK

