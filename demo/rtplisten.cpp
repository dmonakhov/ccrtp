// rtplisten
// Listen for RTP packets.
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

class Listener: RTPSocket {
public:
	Listener(InetHostAddress& ia, tpport_t port):
		RTPSocket(ia,port)
	{
		Connect(ia,5004);
		for (;;) {
			//cerr << getFirstTimestamp();
			if ( getPacket(getFirstTimestamp(),buffer,SIZE) )
				cerr << "I got one" << endl;
			ccxx_sleep(5);
		}
	}
	
private:
	enum { SIZE = 8192 };
	unsigned char buffer[SIZE];
};

int
main(int argc, char *argv[])
{
	if (argc != 3) { 
		cerr << "Syntax: " << " ip port" << endl;
		exit(1);
	}

	InetHostAddress ia(argv[1]);
	Listener foo(ia,atoi(argv[2]));
}

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
