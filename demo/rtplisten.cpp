// rtplisten
// Listen for RTP packets.
// Copyright (C) 2001,2002  Federico Montesino <fedemp@altern.org>
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
#include <cc++/rtp/rtp.h>

#ifdef  CCXX_NAMESPACES
using namespace ost;
using namespace std;
#endif

class Listener: RTPSession {
public:
	Listener(InetHostAddress& ia, tpport_t port):
		RTPSocket(ia,port)
	{
		cout << "My SSRC identifier is: " 
		     << hex << (int)getLocalSSRC() << endl;

		defaultApplication().setSDESItem(SDESItemTypeTOOL,
						 "rtplisten demo app.");
		setExpireTimeout(1000000);

		setPayloadFormat(StaticPayloadFormat(sptPCMU));
		startRunning();
		for (;;) {
			const AppDataUnit* adu;
			while ( (adu = getData(getFirstTimestamp())) ) {
				cerr << "I got an app. data unit with "
				     << adu->getSize()
				     << " payload octets." 
				     << " From " << 
					adu->getSource().getNetworkAddress()
				     << ":" 
				     << adu->getSource().getDataTransportPort()
				     << endl;
				delete adu;
			}
			Thread::sleep(7);
		}
	}

	// redefined from QueueRTCPManager
	void
	onGotSR(SyncSource& source, SendReport& SR, uint8 blocks)
	{
		RTPSession::onGotSR(source,SR,blocks);
		cout << "I got an SR RTCP report from sync. source "
		     << hex << (int)source.getID() << ". From " 
		     << source.getNetworkAddress() << ":" 
		     << source.getControlTransportPort() << endl;
	}

	// redefined from QueueRTCPManager
	void
	onGotRR(SyncSource& source, RecvReport& RR, uint8 blocks)
	{
		RTPSession::onGotRR(source,RR,blocks);
		cout << "I got an RR RTCP report from sync. source "
		     << hex << (int)source.getID() << ". From " 
		     << source.getNetworkAddress() << ":" 
		     << source.getControlTransportPort() << endl;
	}

	// redefined from QueueRTCPManager
	bool
	onGotSDESChunk(SyncSource& source, SDESChunk& chunk, size_t len)
	{ 
		bool result = RTPSession::onGotSDESChunk(source,chunk,len);
		cout << "I got a SDES chunk from sync. source "
		     << source.getID() << ". From " 
		     << source.getNetworkAddress() << ":" 
		     << source.getControlTransportPort() << endl;
		return result;
	}
};

int
main(int argc, char *argv[])
{
	cout << "rtplisten" << endl;

	if (argc != 3) { 
		cerr << "Syntax: " << " ip port" << endl;
		exit(1);
	} else {
		cout << "Press Ctrl-C to finish." << endl;
	}

	InetHostAddress ia(argv[1]);
	Listener foo(ia,atoi(argv[2]));
	return 0;
}

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
