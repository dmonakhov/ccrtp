/*
  Copyright (C) 2006 Werner Dittmann

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by the
  Free Software Foundation; either version 2.1 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef _ZRTPQUEUE_H_
#define _ZRTPQUEUE_H_

#include <ccrtp/cqueue.h>
#include "ZrtpCallback.h"
#include "ZRtp.h"

/**
 * The bridge between the ZRTP implementation and GNU ccRTP.
 *
 * The ZRPT implementation is fairly independent from the underlying
 * RTP/SRTP implementation. This class implements specific
 * functions and interfaces that ZRTP uses to call functions of the
 * hosting RTP/SRTP environment. In this case the host is GNU ccRTP.
 *
 * <p/>
 *
 * As required by the ZRTP implementation this class implements
 * the ZrtpCallback interface.
 *
 * <p/>
 *
 * The <code>initialize</code> method stores the timeout provider and
 * reuses it for every instance. To do so the bridge inherits from
 * Minisip's <e>StateMachine<e/> but does use the timeout specific
 * parts only. The destructor frees the StateMachine to maintain the
 * timout provider's reference counter.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */
#ifdef  CCXX_NAMESPACES
namespace ost {
#endif

class ZrtpQueue : public AVPQueue, public ZrtpCallback {

 public:
    int32_t initialize(const char *zidFilename);

    void start();
    void stop();

    /**
     * This function is used by the service thread to process
     * the next incoming packet and place it in the receive list.
     *
     * This class overloads the function of IncomingDataQueue
     * implementation.
     *
     * @return number of payload bytes received.  <0 if error.
     */
    virtual size_t
    takeInDataPacket();

    /**
     * Handle timeout event forwarded by the TimeoutProvider.
     *
     * Just call the ZRTP engine for further processing.
     */
    void handleTimeout(const std::string &c) {
        if (zrtpEngine != NULL) {
            zrtpEngine->processTimeout();
        }
    };

    /*
    * Refer to ZrtpCallback.h
    */
    int32_t sendDataRTP(const unsigned char* data, int32_t length);

    int32_t sendDataSRTP(const unsigned char* dataHeader, int32_t lengthHeader,
                         char *dataContent, int32_t lengthContent);

    int32_t activateTimer(int32_t time);

    int32_t cancelTimer();

    void sendInfo(MessageSeverity severity, char* msg) {
        fprintf(stderr, "Severity: %d - %s\n", severity, msg);
    }

    /**
     * Switch on the security for the defined part.
     *
     * Create an CryproContext with the negotiated ZRTP data and
     * register it with the respective part (sender or receiver) thus
     * replacing the current active context (usually an empty
     * context). This effectively enables SRTP.
     *
     * @param secrets
     *    The secret keys and salt negotiated by ZRTP
     * @param part
     *    An enum that defines sender, receiver, or both.
     */
    void srtpSecretsReady(SrtpSecret_t* secrets, EnableSecurity part);

    /**
     * Switch off the security for the defined part.
     *
     * Create an empty CryproContext and register it with the
     * repective part (sender or receiver) thus replacing the current
     * active context. This effectively disables SRTP.
     *
     * @param part
     *    An enum that defines sender, receiver, or both.
     */
    void srtpSecretsOff(EnableSecurity part);

    /**
     * This method shall handle GoClear requests.
     *
     * According to the ZRTP specification the user must be informed about
     * this message because the ZRTP implementation switches off security
     * if it could authenticate the GoClear packet.
     *
     */
    void handleGoClear() {
        fprintf(stderr, "Need to process a GoClear message!");
    }

    /*
     * End of ZrtpCallback functions.
     */

    protected:
        ZrtpQueue(uint32 size = RTPDataQueue::defaultMembersHashSize,
                  RTPApplication& app = defaultApplication());

        /**
         * Local SSRC is given instead of computed by the queue.
         */
        ZrtpQueue(uint32 ssrc, uint32 size =
                    RTPDataQueue::defaultMembersHashSize,
                    RTPApplication& app = defaultApplication());

        virtual ~ZrtpQueue();

    private:
        ZRtp *zrtpEngine;
        SrtpSecret_t secret;
        bool enableZrtp;

        int32_t secureParts;

        uint32_t receiverSsrc;
        uint32_t receiverSecure;
        uint16_t receiverSeqNo;

        uint32_t senderSsrc;
        uint32_t senderSecure;

    /**
     * This flag is true if we saw the special <em>0xdeadbeef</em> marker
     * SSRC. The Zfone implementation uses this in its ZRTP packets. Other
     * ZRTP implementation may not require such a marker SSRC.
     * (maybe even Zfone could live without it but ...)
     */
        int8_t zfoneDeadBeef;
};

#ifdef  CCXX_NAMESPACES
}
#endif

#endif
