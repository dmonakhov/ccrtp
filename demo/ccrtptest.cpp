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

#ifdef  CCXX_NAMESPACES
using namespace ost;
using namespace std;
#endif

const InetHostAddress NA = "localhost";
const uint16 PORT = 34566;
unsigned char data[65535];

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
		cerr << "send run start" << endl;
		doTest();
		cerr << "send run end" << endl;
	}

	int doTest()
	{
		// should be valid
		//RTPSession tx();
		RTPSession tx(InetHostAddress("localhost"));
		tx.setSchedulingTimeout(10000);
		tx.setExpireTimeout(1000000);
		
		tx.startRunning();
		
		tx.setPayloadFormat(StaticPayloadFormat(sptPCMU));
		if ( !tx.addDestination(NA,PORT-1) ) {
		return 1;
		}
		
		// 50 packets per second (packet duration of 20ms)
		uint32 period = 20;
		uint16 inc = tx.getCurrentRTPClockRate()/50;
		TimerPort::setTimer(period);
		for ( int i = 0; i < 100; i++ ) {
			tx.putData(i*inc,data,100);
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
	run()
	{
		cerr << "recv run start" << endl;
		doTest();
		cerr << "recv run stop" << endl;
	}

	int
	doTest()
	{		
		RTPSession rx(NA,PORT);

		rx.setSchedulingTimeout(10000);
		rx.setExpireTimeout(1000000);
		
		rx.startRunning();
		rx.setPayloadFormat(StaticPayloadFormat(sptPCMU));
		// arbitrary number of loops
		for ( int i = 0; i < 1000 ; i++ ) {
			const AppDataUnit* adu;
			while ( (adu = rx.getData(rx.getFirstTimestamp())) ) {

				delete adu;
			}
			Thread::sleep(7);
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
		const uint32 NSESSIONS = 50;
		RTPSession rx(NA,PORT);
		RTPSession **tx = new RTPSession* [NSESSIONS];
		for ( uint32 i = 0; i < NSESSIONS; i++ ) {
			tx[i] = new RTPSession(InetHostAddress("localhost"));
		}
		for ( uint32 i = 0; i  < NSESSIONS; i++) {
			tx[i]->setSchedulingTimeout(10000);
			tx[i]->setExpireTimeout(1000000);
			tx[i]->setPayloadFormat(StaticPayloadFormat(sptPCMU));
			if ( !tx[i]->addDestination(NA,PORT) ) {
				cerr << "dest";
				return 1;
			}
  			if ( !tx[i]->addDestination(NA,PORT) ) {
  				cerr << "dest";
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
		for ( uint32 i = 0; i < 100; i++ ) {
			for ( uint32 s = 0; s  < NSESSIONS; s++) {
			// 50 packets per second (packet duration of 20ms)
				uint16 inc = 
					tx[s]->getCurrentRTPClockRate()/50;
				tx[s]->putData(i*inc,data,100);
			}
			Thread::sleep(TimerPort::getTimer());
			TimerPort::incTimer(period);
		}

		Thread::sleep(5000);
		for ( uint32 i = 0; i < NSESSIONS; i++ ) {
			delete tx[i];
		}
		cerr << "it begin" << endl;
		RTPSession::SyncSourcesIterator it;
		for (it = rx.begin() ; it != rx.end(); it++) {
			const SyncSource &s = *it;
			cerr << s.getID() << "-" ;
			if ( s.isSender() ) 
				cout << "(sender)-";
			cout << s.getNetworkAddress() << ":" <<
				s.getControlTransportPort() << "/" <<
				s.getDataTransportPort() << endl;
			Participant *p = s.getParticipant();
			cerr << p->getSDESItem(SDESItemTypeCNAME) << endl;
		}
		RTPApplication &app = defaultApplication();
		RTPApplication::ParticipantsIterator ai;
		for ( ai = app.begin(); ai != app.end(); ai++ ) {
			const Participant &p = *ai;
			cerr << "app it: " << p.getSDESItem(SDESItemTypeCNAME);
			//cerr << p.getPRIVPrefix();
		}

		cerr << "it end" << endl;

		delete tx;
		return 0;
	}
};

// class TestPacketHeaders { }
// header extension

// class TestRTCPTransmission { }

// class TestMiscellaneous { }

int main(int argc, char *argv[])
{
	int result = 0;
	bool send = false;
	bool recv = false;

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
		//tx = new SendPacketTransmissionTest();
		//tx->start();
		//rx = new RecvPacketTransmissionTest();
		//rx->start();

		MiscTest m;
		m.start();
		
		//tx->join();
		//rx->join();
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
