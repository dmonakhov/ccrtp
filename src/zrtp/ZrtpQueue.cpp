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

/*
 * Authors: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#include <string>
#include "ZrtpQueue.h"
#include "TimeoutProvider.h"
#include "ZIDFile.h"
#include "ZrtpStateClass.h"

static TimeoutProvider<std::string, ost::ZrtpQueue*>* staticTimeoutProvider = NULL;

#ifdef  CCXX_NAMESPACES
namespace ost {
#endif

int32_t
ZrtpQueue::initialize(const char *zidFilename)
{
    if (staticTimeoutProvider != NULL) {
        staticTimeoutProvider->stopThread();
        delete staticTimeoutProvider;
    }
    staticTimeoutProvider = new TimeoutProvider<std::string, ZrtpQueue*>();
    staticTimeoutProvider->start();

    std::string fname;
    if (zidFilename == NULL) {
        char *home = getenv("HOME");

        std::string baseDir = (home != NULL) ? (std::string(home) + std::string("/."))
                                             : std::string(".");
        fname = baseDir + std::string("GNUccRTP.zid");
        zidFilename = fname.c_str();
    }
    ZIDFile *zf = ZIDFile::getInstance();
    if (zf->open((char *)zidFilename) < 0) {
        enableZrtp = false;
        sendInfo(Error, "cannot open ZID file");
    }
    return 1;
}

ZrtpQueue::ZrtpQueue(uint32 size, RTPApplication& app) :
        AVPQueue(size,app), enableZrtp(true), zfoneDeadBeef(0)
{
    secureParts = 0;
    zrtpEngine = NULL;

    senderSecure = 0;
    receiverSecure = 0;

    receiverSsrc = 0;
}

ZrtpQueue::ZrtpQueue(uint32 ssrc, uint32 size, RTPApplication& app) :
        AVPQueue(ssrc,size,app), enableZrtp(true), zfoneDeadBeef(0)
{
    secureParts = 0;
    zrtpEngine = NULL;

    senderSecure = 0;
    receiverSecure = 0;

    receiverSsrc = 0;

}

ZrtpQueue::~ZrtpQueue() {
    stop();
}

void ZrtpQueue::start() {
    ZIDFile *zid = ZIDFile::getInstance();
    const uint8_t* ownZid = zid->getZid();

    if (zrtpEngine == NULL) {
        zrtpEngine = new ZRtp((uint8_t*)ownZid, (ZrtpCallback*)this);
        zrtpEngine->startZrtpEngine();
    }
}

void ZrtpQueue::stop() {
    if (staticTimeoutProvider != NULL) {
        staticTimeoutProvider->stopThread();
        delete staticTimeoutProvider;
        staticTimeoutProvider = NULL;
    }

    if (zrtpEngine != NULL) {
        zrtpEngine->stopZrtp();
        delete zrtpEngine;
        zrtpEngine = NULL;
    }
}

/*
 * The takeInDataPacket implementation for ZRTP.
 */
size_t
ZrtpQueue::takeInDataPacket(void)
{
    InetHostAddress network_address;
    tpport_t transport_port;

    uint32 nextSize = (uint32)getNextDataPacketSize();
    unsigned char* buffer = new unsigned char[nextSize];
    int32 rtn = (int32)recvData(buffer,nextSize,network_address,transport_port);
    if ( (rtn < 0) || ((uint32)rtn > getMaxRecvPacketSize()) ){
        delete buffer;
        return 0;
    }

    // get time of arrival
    struct timeval recvtime;
    gettimeofday(&recvtime,NULL);

    //  build a packet. It will link itself to its source
    IncomingRTPPkt* packet =
            new IncomingRTPPkt(buffer,rtn);

    // Generic header validity check.
    if ( !packet->isHeaderValid() ) {
        delete packet;
        return 0;
    }

    bool doZrtp = false;
    if (enableZrtp) {
        uint16 magic = packet->getHdrExtUndefined();
        if (magic != 0) {
            magic = ntohs(magic);
            if (magic == ZRTP_EXT_PACKET) {
                doZrtp = true;
                packet->checkZrtpChecksum(false);
                if (zrtpEngine != NULL) {
                    unsigned char* extHeader =
                            const_cast<unsigned char*>(packet->getHdrExtContent());
                    // this now points beyond the undefined and length field.
                    // We need them, thus adjust
                    extHeader -= 4;
                    if (zrtpEngine->handleGoClear(extHeader)) {
                        delete packet;
                        return 0;
                    }
                }
            }
        }

       /*
        * In case this is not a special Zfone packet and we
        * have not seen a packet with a valid SSRC then set
        * the found SSRC as the stream's SSRC. We need the real
        * SSRC for encryption/decryption. Thus we rely on the fact
        * that we receive at least one real RTP packet, i.e. not
        * a Zfone ZRTP packet from our peer before we do some
        * SRTP encryption/decryption. This is usually the case
        * because RTP clients send data very fast and ZRTP protocl
        * handling takes some more milliseconds. ZRTP
        * implementations that do not use special marker SSRC
        * are always ok.
        */
        if (packet->getSSRC() != 0xdeadbeef) {
            if (receiverSsrc == 0) {
                receiverSsrc = packet->getSSRC();
            }
        }
        else {
            zfoneDeadBeef = 1;
        }
    }
    CryptoContext* pcc = getInQueueCryptoContext(receiverSsrc);
    if (pcc != NULL) {
        packet->unprotect(pcc); // TODO - discard packet in case of error
    }
    if (doZrtp && zrtpEngine != NULL) {
        unsigned char* extHeader = const_cast<unsigned char*>(packet->getHdrExtContent());
        // this now points beyond the undefined and length field. We need them,
        // thus adjust
        extHeader -= 4;
        int ret = zrtpEngine->processExtensionHeader(extHeader, const_cast<unsigned char*>(packet->getPayload()));

        /*
         * the ZRTP engine returns OkDismiss in case of the Confirm packets.
         * They contain payload data that should not be given to the application
         */
        receiverSeqNo = packet->getSeqNum();
        if (ret == OkDismiss) {
            delete packet;
            return 0;
        }
        // if no more payload then it was a pure ZRTP packet, done with it.
        if (packet->getPayloadSize() <= 0) {
            delete packet;
            return 0;
        }
    }

    // virtual for profile-specific validation and processing.
    if (!onRTPPacketRecv(*packet) ) {
        delete packet;
        return 0;
    }

    bool source_created;
    SyncSourceLink* sourceLink =
            getSourceBySSRC(packet->getSSRC(),source_created);
    SyncSource* s = sourceLink->getSource();
    if ( source_created ) {
        // Set data transport address.
        setDataTransportPort(*s,transport_port);
        // Network address is assumed to be the same as the control one
        setNetworkAddress(*s,network_address);
        sourceLink->initStats();
        // First packet arrival time.
        sourceLink->setInitialDataTime(recvtime);
        sourceLink->setProbation(getMinValidPacketSequence());
        if ( sourceLink->getHello() )
            onNewSyncSource(*s);
    }
    else if ( 0 == s->getDataTransportPort() ) {
        // Test if RTCP packets had been received but this is the
        // first data packet from this source.
        setDataTransportPort(*s,transport_port);
    }

    // Before inserting in the queue,
    // 1) check for collisions and loops. If the packet cannot be
    //    assigned to a source, it will be rejected.
    // 2) check the source is a sufficiently well known source
    // TODO: also check CSRC identifiers.
    if (checkSSRCInIncomingRTPPkt(*sourceLink, source_created,
        network_address, transport_port) &&
        recordReception(*sourceLink,*packet,recvtime) ) {
        // now the packet link is linked in the queues
        IncomingRTPPktLink* packetLink =
                new IncomingRTPPktLink(packet,
                                       sourceLink,
                                       recvtime,
                                       packet->getTimestamp() -
                                               sourceLink->getInitialDataTimestamp(),
                                       NULL,NULL,NULL,NULL);
        insertRecvPacket(packetLink);
    } else {
        // must be discarded due to collision or loop or
        // invalid source
        delete packet;
    }

    // Start the ZRTP engine only after we got a first RTP packet. This is necessary
    // to have a real SSRC at hand.
    if (enableZrtp && zrtpEngine == NULL) {
        start();
    }

    // ccRTP keeps packets from the new source, but avoids
    // flip-flopping. This allows losing less packets and for
    // mobile telephony applications or other apps that may change
    // the source transport address during the session.
    return rtn;
}

/*
 * Here the callback methods required by the ZRTP implementation
 */
int32_t ZrtpQueue::sendDataRTP(const unsigned char *data, int32_t length) {

    uint8 dummyChecksum[] = {0, 0};
    uint32_t ts = time(NULL);
    OutgoingRTPPkt* packet = new OutgoingRTPPkt(NULL, 0, data, length, dummyChecksum, 2, 0, cContext);

    if (zfoneDeadBeef) {
        packet->setSSRC(0xdeadbeef);
    }
    else {
        packet->setSSRCNetwork(getLocalSSRCNetwork());
    }
    packet->setPayloadType(13);

    int seqNo = getSequenceNumber();
    packet->setSeqNum(seqNo++);
    setNextSeqNum(seqNo);

    packet->setTimestamp(ts);

    packet->enableZrtpChecksum();
    packet->computeZrtpChecksum();

    dispatchImmediate(packet);
    delete packet;

    return 1;
}

int32_t ZrtpQueue::sendDataSRTP(const unsigned char *dataHeader, int32_t lengthHeader,
                                            char *dataContent, int32_t lengthContent)
{
    time_t ts = time(NULL);
    // plus 2 is for ZRTP checksum
    uint8* tmpBuffer = new uint8[lengthContent + 2];

    memcpy(tmpBuffer, dataContent, lengthContent);
    OutgoingRTPPkt* packet = new OutgoingRTPPkt(NULL, 0, dataHeader, lengthHeader,
            tmpBuffer, lengthContent+2, 0, cContext);

    if (zfoneDeadBeef) {
        packet->setSSRC(0xdeadbeef);
    }
    else {
        packet->setSSRCNetwork(getLocalSSRCNetwork());
    }
    packet->setPayloadType(13);

    int seqNo = getSequenceNumber();
    packet->setSeqNum(seqNo++);
    setNextSeqNum(seqNo);

    packet->setTimestamp(ts);

    packet->enableZrtpChecksum();
    if (zfoneDeadBeef) {
        packet->protect(0xdeadbeef);
    }
    else {
        packet->protect(getLocalSSRC());
    }
    packet->computeZrtpChecksum();

    dispatchImmediate(packet);
    delete packet;
    return 1;
}

void ZrtpQueue::srtpSecretsReady(SrtpSecret_t* secrets, EnableSecurity part)
{
    CryptoContext* cryptoContext;
    char buffer[128];

    if (part == ForSender || part == (EnableSecurity)ForSender+ForReceiver) {
    // encrypting packets, intiator uses initiator keys, responder uses responders keys
        if (secrets->role == Initiator) {
            cryptoContext = new CryptoContext(
                    getLocalSSRC(),
                    0 /*roc*/,
                    0L,                                      // keyderivation << 48,
                    SrtpEncryptionAESCM,                     // encryption algo
                    SrtpAuthenticationSha1Hmac,              // authtication algo
                    (unsigned char*)secrets->keyInitiator,   // Master Key
                    secrets->initKeyLen / 8,                 // Master Key length
                    (unsigned char*)secrets->saltInitiator,  // Master Salt
                    secrets->initSaltLen / 8,                // Master Salt length
                    secrets->initKeyLen / 8,                 // encryption keyl
                    20,                                      // authentication key len
                    secrets->initSaltLen / 8,                // session salt len
                    1,
                    1,
                    secrets->srtpAuthTagLen / 8);            // authentication tag lenA
        }
        else {
            cryptoContext = new CryptoContext(
                    getLocalSSRC(),
                    0 /*roc*/,
                    0L,                                      // keyderivation << 48,
                    SrtpEncryptionAESCM,                     // encryption algo
                    SrtpAuthenticationSha1Hmac,              // authtication algo
                    (unsigned char*)secrets->keyResponder,   // Master Key
                    secrets->respKeyLen / 8,                 // Master Key length
                    (unsigned char*)secrets->saltResponder,  // Master Salt
                    secrets->respSaltLen / 8,                // Master Salt length
                    secrets->respKeyLen / 8,                 // encryption keyl
                    20,                                      // authentication key len
                    secrets->respSaltLen / 8,                // session salt len
                    1,
                    1,
                    secrets->srtpAuthTagLen / 8);            // authentication tag len
            }
            cryptoContext->deriveSrtpKeys(getSequenceNumber());
            cContext = cryptoContext;
            snprintf(buffer, 120, "SAS Value(S): %s\n", secrets->sas.c_str());
            sendInfo(Info, buffer);
    }
    if (part == ForReceiver || part == (EnableSecurity)ForSender+ForReceiver) {
    // decrypting packets, intiator uses responder keys, responder initiator keys
        if (secrets->role == Initiator) {
            cryptoContext = new CryptoContext(
                    receiverSsrc,
                    0 /*roc*/,
                    0L,                                      // keyderivation << 48,
                    SrtpEncryptionAESCM,                     // encryption algo
                    SrtpAuthenticationSha1Hmac,              // authtication algo
                    (unsigned char*)secrets->keyResponder,   // Master Key
                    secrets->respKeyLen / 8,                 // Master Key length
                    (unsigned char*)secrets->saltResponder,  // Master Salt
                    secrets->respSaltLen / 8,                // Master Salt length
                    secrets->respKeyLen / 8,                 // encryption keyl
                    20,                                      // authentication key len
                    secrets->respSaltLen / 8,                // session salt len
                    1,
                    1,
                    secrets->srtpAuthTagLen / 8);            // authentication tag len
        }
        else {
            cryptoContext = new CryptoContext(
                    receiverSsrc,
                    0 /*roc*/,
                    0L,                                      // keyderivation << 48,
                    SrtpEncryptionAESCM,                     // encryption algo
                    SrtpAuthenticationSha1Hmac,              // authtication algo
                    (unsigned char*)secrets->keyInitiator,   // Master Key
                    secrets->initKeyLen / 8,                 // Master Key length
                    (unsigned char*)secrets->saltInitiator,  // Master Salt
                    secrets->initSaltLen / 8,                // Master Salt length
                    secrets->initKeyLen / 8,                 // encryption keyl
                    20,                                      // authentication key len
                    secrets->initSaltLen / 8,                // session salt len
                    1,
                    1,
                    secrets->srtpAuthTagLen / 8);            // authentication tag len
        }
        cryptoContext->deriveSrtpKeys(receiverSeqNo);
        setInQueueCryptoContext(cryptoContext);
        snprintf(buffer, 120, "SAS Value(R): %s\n", secrets->sas.c_str());
        sendInfo(Info, buffer);
    }
}

void ZrtpQueue::srtpSecretsOff(EnableSecurity part)
{
    CryptoContext* cryptoContext;

    if (part == ForSender) {
        if (cContext != NULL) {
            CryptoContext* tmp = cContext;
            cContext = NULL;
            delete tmp;
        }
    }
    if (part == ForReceiver) {
        cryptoContext = new CryptoContext(receiverSsrc); // a dummy CC just needed for remove method
        removeInQueueCryptoContext(cryptoContext);
        delete cryptoContext;
    }
}


int32_t
ZrtpQueue::activateTimer(int32_t time)
{
    std::string s("ZRTP");
    staticTimeoutProvider->requestTimeout(time, this, s);
    return 1;
}

int32_t
ZrtpQueue::cancelTimer()
{
    std::string s("ZRTP");
    staticTimeoutProvider->cancelRequest(this, s);
    return 1;
}

#ifdef  CCXX_NAMESPACES
}
#endif
