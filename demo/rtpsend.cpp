// rtpsend
// Send RTP packets using ccRTP.
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

#include "rtp.h"
#include <cstdlib>

#ifdef  CCXX_NAMESPACES
using namespace ost;
#endif

/**
 * @brief This class sends an RTP Packet
 **/
class Sender: RTPSocket {
public:
	Sender(const unsigned char* data, const InetHostAddress& ia, 
	       tpport_t port, uint32 tstamp, uint16 count):
		RTPSocket(ia)
	{
		uint32 timestamp = tstamp? tstamp : 0;
		
		setTimeout(0);
		setExpired(1000000);
		
		if ( Connect(ia,port) < 0 ) {
			cerr << "Could not connect" << endl;
			exit(0);
		}
		
		for ( int i = 0; i < count ; i++ ) {
			putPacket(i*1600,RTP_PAYLOAD_PCMU,
				  data,strlen((char *)data) + 1);
			ccxx_sleep(200);
		}
		// Allow the service thread to run
		ccxx_sleep(10);
	}
};

int
main(int argc, char *argv[])
{
	if (argc != 6) { 
		cerr << "Syntax: " << "data host port timestamp count" << endl;
		exit(1);
	}

	Sender sender((unsigned char *)argv[1], InetHostAddress(argv[2]),
		      atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
	
	cout << "I have sent " << argv[5] 
	     << " RTP packets containing \"" << argv[1]
	     << "\", with timestamp " << argv[4]
	     << " to " << argv[2] << ":" << argv[3]
	     << endl;
}

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */




