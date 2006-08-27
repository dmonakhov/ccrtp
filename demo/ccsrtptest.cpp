// test ccRTP functionality
// Copyright (C) 2004 Federico Montesino Pouzols <fedemp@altern.org>
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <cstdlib>
#include <ccrtp/rtp.h>
#include <ccrtp/CryptoContext.h>

#ifdef  CCXX_NAMESPACES
using namespace ost;
using namespace std;
#endif


inline int
hex_char_to_nibble(uint8_t c) {
        switch(c) {
                case ('0'): return 0x0;
                case ('1'): return 0x1;
                case ('2'): return 0x2;
                case ('3'): return 0x3;
                case ('4'): return 0x4;
                case ('5'): return 0x5;
                case ('6'): return 0x6;
                case ('7'): return 0x7;
                case ('8'): return 0x8;
                case ('9'): return 0x9;
                case ('a'): return 0xa;
                case ('A'): return 0xa;
                case ('b'): return 0xb;
                case ('B'): return 0xb;
                case ('c'): return 0xc;
                case ('C'): return 0xc;
                case ('d'): return 0xd;
                case ('D'): return 0xd;
                case ('e'): return 0xe;
                case ('E'): return 0xe;
                case ('f'): return 0xf;
                case ('F'): return 0xf;
                default: return -1;   /* this flags an error */
        }
        /* NOTREACHED */
        return -1;  /* this keeps compilers from complaining */
}

/*
 * hex_string_to_octet_string converts a hexadecimal string
 * of length 2 * len to a raw octet string of length len
 */

int
hex_string_to_octet_string(char *raw, char *hex, int len) {
        uint8 x;
        int tmp;
        int hex_len;

        hex_len = 0;
        while (hex_len < len) {
                tmp = hex_char_to_nibble(hex[0]);
                if (tmp == -1)
                        return hex_len;
                x = (tmp << 4);
                hex_len++;
                tmp = hex_char_to_nibble(hex[1]);
                if (tmp == -1)
                        return hex_len;
                x |= (tmp & 0xff);
                hex_len++;
                *raw++ = x;
                hex += 2;
        }
        return hex_len;
}

class PacketsPattern
{
public:
	inline const InetHostAddress&
	getDestinationAddress() const
		{ return destinationAddress; }

	inline const tpport_t
	getDestinationPort() const
		{ return destinationPort; }

	uint32
	getPacketsNumber() const
		{ return packetsNumber; }

        uint32
        getSsrc() const
		{ return 0xdeadbeef; }

        const unsigned char*
	getPacketData(uint32 i)
		{ return data; }

	const size_t
	getPacketSize(uint32 i)
		{ return packetsSize; }

private:
	static const InetHostAddress destinationAddress;
	static const uint16 destinationPort = 10002;
	static const uint32 packetsNumber = 10;
	static const uint32 packetsSize = 12;
	static unsigned char data[];
};

const InetHostAddress PacketsPattern::destinationAddress =
InetHostAddress("localhost");

unsigned char PacketsPattern::data[] = {
        "0123456789\n"
};

PacketsPattern pattern;

static char* fixKey = "c2479f224b21c2008deea6ef0e5dbd4a761aef98e7ebf8eed405986c4687";

// static uint8* masterKey =  (uint8*)"masterKeymasterKeymasterKeymaster";
// static uint8* masterSalt = (uint8*)"NaClNaClNaClNa";

static uint8 binKeys[60];

class
Test
{
public:
	virtual int
	doTest() = 0;
};

class
SendPacketTransmissionTest : public Test, public Thread, public TimerPort
{
public:
	void
	run()
	{
		doTest();
	}

	int doTest()
	{
		// should be valid?
		//RTPSession tx();
		RTPSession tx(pattern.getSsrc(), InetHostAddress("localhost"));
		tx.setSchedulingTimeout(10000);
		tx.setExpireTimeout(1000000);

                CryptoContext* txCryptoCtx = new CryptoContext(pattern.getSsrc(),
						0,                                       // roc,
						0L,                                      // keydr << 48,
						SrtpEncryptionAESCM,                     // encryption algo
						SrtpAuthenticationSha1Hmac,              // authtication algo
                                                binKeys,                                  // Master Key
						128 / 8,                                 // Master Key length
                                                binKeys+16,                               // Master Salt
						112 / 8,                                 // Master Salt length
						128 / 8,                                 // encryption keyl
						160 / 8,                                 // authentication key len (SHA1))
						112 / 8,                                 // session salt len
						80 / 8);                                 // authentication tag len
                txCryptoCtx->deriveSrtpKeys(0);

		tx.setOutQueueCryptoContext(txCryptoCtx);

		tx.startRunning();

		tx.setPayloadFormat(StaticPayloadFormat(sptPCMU));
		if (!tx.addDestination(pattern.getDestinationAddress(),
				       pattern.getDestinationPort()) ) {
			return 1;
		}

		// 50 packets per second (packet duration of 20ms)
		uint32 period = 20;
		uint16 inc = tx.getCurrentRTPClockRate()/50;
		TimerPort::setTimer(period);
		for ( uint32 i = 0; i < pattern.getPacketsNumber(); i++ ) {
			tx.putData(i*inc,
				   pattern.getPacketData(i),
				   pattern.getPacketSize(i));
			cout << "Sent some data: " << i << endl;
			Thread::sleep(TimerPort::getTimer());
			TimerPort::incTimer(period);
		}
		return 0;
	}
};


class
RecvPacketTransmissionTest : public Test, public Thread
{
public:
	void
	run() {
	       doTest();
	}

	int
        doTest() {
		RTPSession rx(pattern.getSsrc(), pattern.getDestinationAddress(),
			      pattern.getDestinationPort());

		rx.setSchedulingTimeout(10000);
		rx.setExpireTimeout(1000000);

                CryptoContext* rxCryptoCtx = new CryptoContext(pattern.getSsrc(),
                                0,                                       // roc,
                                0L,                                      // keydr << 48,
                                SrtpEncryptionAESCM,                     // encryption algo
                                SrtpAuthenticationSha1Hmac,              // authtication algo
                                binKeys,                                 // Master Key
                                128 / 8,                                 // Master Key length
                                binKeys+16,                              // Master Salt
                                112 / 8,                                 // Master Salt length
                                128 / 8,                                 // encryption keyl
                                160 / 8,                                 // authentication key len (SHA1))
                                112 / 8,                                 // session salt len
                                80 / 8);                                 // authentication tag len
                rxCryptoCtx->deriveSrtpKeys(0);
                rx.setInQueueCryptoContext(rxCryptoCtx);

                rx.startRunning();
		rx.setPayloadFormat(StaticPayloadFormat(sptPCMU));
		// arbitrary number of loops
		for ( int i = 0; i < 500 ; i++ ) {
                        const AppDataUnit* adu;
			while ( (adu = rx.getData(rx.getFirstTimestamp())) ) {
				cerr << "got some data: " << adu->getData() << endl;
				delete adu;
			}
                        Thread::sleep(70);
		}
		return 0;
	}
};

class
MiscTest : public Test, public Thread, public TimerPort
{
	void
	run()
	{
		doTest();
	}

	int
	doTest()
	{
		const uint32 NSESSIONS = 10;
		RTPSession rx(pattern.getDestinationAddress(),pattern.getDestinationPort());
		RTPSession **tx = new RTPSession* [NSESSIONS];
		for ( uint32 i = 0; i < NSESSIONS; i++ ) {
			tx[i] = new RTPSession(InetHostAddress("localhost"));
		}
		for ( uint32 i = 0; i  < NSESSIONS; i++) {
			tx[i]->setSchedulingTimeout(10000);
			tx[i]->setExpireTimeout(1000000);
			tx[i]->setPayloadFormat(StaticPayloadFormat(sptPCMU));
			if ( !tx[i]->addDestination(pattern.getDestinationAddress(),
						    pattern.getDestinationPort()) ) {
				return 1;
			}
		}

		rx.setPayloadFormat(StaticPayloadFormat(sptPCMU));
		rx.setSchedulingTimeout(5000);
		rx.setExpireTimeout(10000000); // 10 seconds!
		rx.startRunning();

		for ( uint32 i = 0; i  < NSESSIONS; i++) {
			tx[i]->startRunning();
		}
		uint32 period = 20;
		TimerPort::setTimer(period);
		for ( uint32 i = 0; i < pattern.getPacketsNumber(); i++ ) {
			if ( i == 70 ) {
				RTPApplication &app = defaultApplication();
				app.setSDESItem(SDESItemTypeCNAME,"foo@bar");
			}
			for ( uint32 s = 0; s  < NSESSIONS; s++) {
				// 50 packets per second (packet duration of 20ms)
				uint16 inc =
					tx[s]->getCurrentRTPClockRate()/50;
				tx[s]->putData(i*inc,
					       pattern.getPacketData(i),
					       pattern.getPacketSize(i));
			}
			Thread::sleep(TimerPort::getTimer());
			TimerPort::incTimer(period);
		}

		Thread::sleep(5000);
		for ( uint32 i = 0; i < NSESSIONS; i++ ) {
			delete tx[i];
		}
		RTPSession::SyncSourcesIterator it;
		cout << "Sources of synchronization:" << endl;
		for (it = rx.begin() ; it != rx.end(); it++) {
			const SyncSource &s = *it;
			cout << s.getID();
			if ( s.isSender() )
				cout << " (sender) ";
			cout << s.getNetworkAddress() << ":" <<
				s.getControlTransportPort() << "/" <<
				s.getDataTransportPort();
			Participant *p = s.getParticipant();
			cout << " (" <<
				p->getSDESItem(SDESItemTypeCNAME)
			     << ") " << endl;
		}
		RTPApplication &app = defaultApplication();
		RTPApplication::ParticipantsIterator ai;
		cout << "Participants:" << endl;
		for ( ai = app.begin(); ai != app.end(); ai++ ) {
			const Participant &p = *ai;
			cout << p.getSDESItem(SDESItemTypeCNAME) << endl;
			//cout << p.getPRIVPrefix();
		}
		delete tx;
		return 0;
	}
};

// class TestPacketHeaders { }
// header extension

// class TestRTCPTransmission { }

// class TestMiscellaneous { }

// Things that should be tested:
// extreme values (0 - big) for putData
// segmentation (setMaxSendSegmentSize())
// performance: packets/second (depending on packet size and # of participants)
int main(int argc, char *argv[])
{
	int result = 0;
        bool send = false;
        bool recv = false;

        char c;
        char* inputKey = NULL;

        /* check args */
        while (1) {
                c = getopt(argc, argv, "k:rsaeld:");
                if (c == -1) {
                        break;
                }
                switch (c) {
                        case 'k':
                                inputKey = optarg;
                                break;
                        case 'r':
                                recv = true;
                                break;
                        case 's':
                                send = true;
                                break;
                        default:
                                cerr << "Wrong Arguments" << endl;
                }
        }

        if (inputKey == NULL) {
                inputKey = fixKey;
        }
        hex_string_to_octet_string((char*)binKeys, inputKey, 60);

        if (send || recv) {
                if (send) {
                        printf("Running as sender\n");
                }
                else {
                        printf("Running as receiver\n");
                }
        }
        else {
                printf("Miscellanous tests\n");
        }
	RecvPacketTransmissionTest *rx;
	SendPacketTransmissionTest *tx;

	// accept as parameter if must run as --send or --recv

	// run several tests in parallel threads
	if ( send ) {
		tx = new SendPacketTransmissionTest();
		tx->start();
		tx->join();
	} else 	if ( recv ) {
		rx = new RecvPacketTransmissionTest();
		rx->start();
		rx->join();
	} else {
		MiscTest m;
		m.start();
		m.join();
	}
	exit(result);
}

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
