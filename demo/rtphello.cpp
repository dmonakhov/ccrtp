// rtphello
// A very simple program for testing and illustrating basic features of ccRTP.
// Copyright (C) 2001  Federico Montesino <p5087@quintero.fie.us.es>
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


// This is an introductory example file that illustrates basic usage
// of ccRTP. You will also see a bit on how to use other classes from
// CommonC++.

// I am a typical hello world program. I consist of a sender thread,
// that sends the salutation message on RTP packets; and a receiver
// thread, that prints the messages. This is a program with an unsual
// structure, since the sender has no clear periodicity, the receiver
// is very unreliable, and both are in the same program. Thus, it
// should not be seen as an example for typical applications but as a
// test of some functions of ccRTP.

// In order to use ccRTP, the RTP stack of CommonC++, just include...
#include <cc++/rtp.h>
#include <cstdio>
#include <cstdlib>

// base ports
const int RECEIVER_BASE = 33634;
const int TRANSMITTER_BASE = 32522;

// For this example, this is irrelevant. 
const int TIMESTAMP_RATE = 90000;

/**
 * @class ccRTP_Hello_Rx
 * Receiver of salutes.
 */
class ccRTP_Hello_Rx: public Thread
{

private:
	// socket to receive packets
	RTPSocket *socket;
	// loopback network address
	InetHostAddress local_ip;
	// identifier of this sender
	uint32 ssrc;
	
public:
	ccRTP_Hello_Rx(){
		// Before using ccRTP you should learn something about other
		// CommonC++ classes. We need InetHostAddress...

		// Construct loopback address
		local_ip = "127.0.0.1";

		// Is that correct?
		if( ! local_ip ){  
			// this is equivalent to `! local_ip.isInetAddress()'
			cerr << "Rx: IP address is not correct!" << endl;
			exit(1);
		}

		// create socket for RTP connection and get a random
		// SSRC identifier
		socket = new RTPSocket(local_ip,RECEIVER_BASE,0);
		ssrc = socket->getLocalInfo().getID();
	}
	
	~ccRTP_Hello_Rx(){
		cout << endl << "Destroying receiver with ID " << ssrc;
		Terminate();
		delete socket;
		cout << "... " << "destroyed.";
	}

	// This method does almost everything.
	void Run(void){    

		cout << "Hello, " << socket->getLocalInfo().getCNAME() 
		     << " ..." << endl;
		// redefined from Thread.
		// Set up connection
		socket->setTimeout(20000);
		socket->setExpired(3000000);
		//socket->UDPTransmit::setTypeOfService(SOCKET_IPTOS_LOWDELAY);
		if( socket->Connect(local_ip,TRANSMITTER_BASE) < 0 )
			cerr << "Rx (" << ssrc 
			     << "): could not connect to port." 
			     << TRANSMITTER_BASE;
		
		// Let's check the queues  (you should read the documentation
		// so that you know what the queues are for).
		cout << "Rx (" << ssrc 
		     << "): The queue is " 
		     << ( socket->RTPQueue::isActive() ? "" : "in") 
		     << "active." << endl;		

		cout << "Rx (" << ssrc
		     << "): " << local_ip.getHostname() 
		     <<	" is waiting for salutes in port "
		     << RECEIVER_BASE << "..." << endl;
		
		// This is the main loop, where packets are received.
		for( int i = 0 ; true ; i++ ){
			unsigned char buffer[100] = "[empty]";
			
			// Wait for an RTP packet. [Absolutely unreliable]
			while ( !socket->getPacket(socket->getFirstTimestamp(),
						   buffer,100) )
			ccxx_sleep(10);
			
			// Print content (likely a salute)
			time_t receiving_time = time(NULL);
			char tmstring[30];
			strftime(tmstring,30,"%X",localtime(&receiving_time));
			cout << "Rx (" << ssrc 
			     << "): [receiving at " << tmstring << "]: " 
			     <<	buffer << endl;
		}
	}
};

/**
 * @class ccRTP_Hello_Tx
 * Transmitter of salutes.
 */
class ccRTP_Hello_Tx: public Thread, public TimerPort
{

private:
	// socket to transmit
	RTPSocket *socket;
	// loopback network address
	InetHostAddress local_ip;
	// identifier of this sender
	uint32 ssrc;

public:
	ccRTP_Hello_Tx(){
		// Before using ccRTP you should learn something about other
		// CommonC++ classes. We need InetHostAddress...

		// Construct loopback address
		local_ip = "127.0.0.1";
		
		// Is that correct?
		if( ! local_ip ){  
		// this is equivalent to `! local_ip.isInetAddress()'
			cerr << "Tx: IP address is not correct!" << endl;
			exit(1);
		}
		
		socket = new RTPSocket(local_ip,TRANSMITTER_BASE,0);
		ssrc = socket->getLocalInfo().getID();
	}

	~ccRTP_Hello_Tx(){
		cout << endl << "Destroying transmitter with ID " << ssrc;
		Terminate();
		delete socket;
		cout << "... " << "destroyed.";
	}

	// This method does almost everything.
	void Run(void){    
		// redefined from Thread.
		cout << "Tx (" << ssrc << "): " << local_ip.getHostname() 
		     <<	" is going to salute perself through " 
		     << local_ip << "..." << endl;
		
		// Set up connection
		socket->setTimeout(20000);
		socket->setExpired(3000000);
		if( socket->Connect(local_ip,RECEIVER_BASE) < 0 )
			cerr << "Tx (" << ssrc 
			     << "): could not connect to port." 
			     << RECEIVER_BASE;
		
		// Let's check the queues  (you should read the documentation
		// so that you know what the queues are for).
		cout << "Tx (" << ssrc << "): The queue is "
		     << ( socket->RTPQueue::isActive()? "" : "in")
		     << "active." << endl;

		cout << "Tx (" << ssrc << "): Transmitting salutes to port "
		     << RECEIVER_BASE << "..." << endl;

		uint32 timestamp = 0;
		//socket->getCurrentTimestamp(RTP_PAYLOAD_MP2T);
		time_t initial_time = time(NULL);
		// This will be useful for periodic execution
		TimerPort::setTimer(1000);

		// This is the main loop, where packets are sent.
		for( int i = 0 ; true ;){

			const uint32 TIMESTAMP_RATE = 
				socket->getRate(RTP_PAYLOAD_MP2T);
			
			// send RTP packets, providing timestamp,
			// payload type and payload.  
			// construct salute.
			unsigned char salute[50];
			snprintf((char *)salute,50,
				 "Hello, brave gnu world! (no %u)",i);
			time_t sending_time = time(NULL);
			// get timestamp to send salute
			if ( i++ == 0 ){
				timestamp = socket->
					getCurrentTimestamp(RTP_PAYLOAD_MP2T);
				
			} else {
				// increment for 1 second
				timestamp += socket->getRate(RTP_PAYLOAD_MP2T);
			}	
			socket->putPacket(timestamp,
					  RTP_PAYLOAD_MP2T,
					  salute,
					  strlen((char *)salute)+1);
			// print info
			char tmstring[30];
			strftime(tmstring,30,"%X",
				 localtime(&sending_time));
			cout << "Tx (" << ssrc 
			     << "): sending salute " << "no " << i++ 
			     << ", at " << tmstring 
			     << "..." << endl;

			// Let's wait for the next cycle
			ccxx_sleep(TimerPort::getTimer());
			TimerPort::incTimer(1000);
		}
	}
};

void main(int argc, char *argv[]){

	// Construct the two main threads. they will not run yet.
	ccRTP_Hello_Rx *receiver = new ccRTP_Hello_Rx;
	ccRTP_Hello_Tx *transmitter = new ccRTP_Hello_Tx;
	
	cout << "This is rtphello, a very simple test program for ccRTP." << 
		endl << "Strike [Enter] when you are fed up with it." << endl;

	// Start execution of hello now.
	receiver->Start();
	transmitter->Start();

	cin.get();

	delete receiver;
	delete transmitter;

	cout << endl << "That's all" << endl;
	
	exit(0);
}

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
