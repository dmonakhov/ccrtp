// Copyright (C) 1999-2001 Open Source Telecom Corporation.
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
// QueueRTCPManager class implementation
//
#include "private.h"

#ifdef  __NAMESPACES__
namespace ost {
#endif

// Beware: certain widespread and broken "C++ compiler" does not allow
// to initialize static members in the class declaration.
#if	__BYTE_ORDER == __BIG_ENDIAN
const uint16 QueueRTCPManager::RTCP_VALID_MASK = (0xc000 | 0x2000  | 0xfe);
const uint16 QueueRTCPManager::RTCP_VALID_VALUE = ((CCRTP_VERSION << 14) | RTCP_TYPE_SR);
#else
const uint16 QueueRTCPManager::RTCP_VALID_MASK = (0x00c0 | 0x0020 | 0xfe00);
const uint16 QueueRTCPManager::RTCP_VALID_VALUE = ((CCRTP_VERSION << 6) | (RTCP_TYPE_SR << 8));
#endif
const uint16 QueueRTCPManager::TIMEOUT_MULTIPLIER = 5;
const double QueueRTCPManager::RECONSIDERATION_COMPENSATION = 2.718281828 - 1.5;

QueueRTCPManager::QueueRTCPManager(int pri):
	RTPQueue(pri), 
	pathMTU(1500),
	rtcpsend_buffer(new unsigned char[pathMTU]),	
	rtcprecv_buffer(new unsigned char[pathMTU]),
	rtcp_active(false),
	controlbw(0), sendcontrolbw(0.25),recvcontrolbw(1-sendcontrolbw),
	ctrlsendcount(0), 
	lower_headers_size( networkHeaderSize() + transportHeaderSize() ),
	rtcp_pmembers(1), rtcp_we_sent(false),
	rtcp_avg_size(sizeof(RTCPFixedHeader) + sizeof(uint32) +
			     sizeof(SenderInfo)), 
	rtcp_initial(true),
	last_sendcount(0), prev_nvalid_sources(0),
	CNAME_len(0), 
	nprevalid_srcs(0), nvalid_srcs(1), // ourselves
	npredeleted_srcs(0), ndeleted_srcs(0),
	rtcp_min_interval(5000000)  // 5 s
{
	// guess CNAME, in the form of user@host_fqn
	findCNAME();

	// Fill in fixed fields that will never change
	RTCPPacket* pkt = reinterpret_cast<RTCPPacket*>(rtcpsend_buffer);
	pkt->fh.version = CCRTP_VERSION;
	// (handleSSRCCollision will have to take this into account)
	pkt->info.SR.ssrc = localsrc->getID();

	// initialize RTCP timing
	rtcp_tp.tv_sec = rtcp_tc.tv_sec = rtcp_tn.tv_sec = 0;
	rtcp_tp.tv_usec = rtcp_tc.tv_usec = rtcp_tn.tv_usec = 0;
	// force an initial check for incoming RTCP packets
	gettimeofday(&rtcp_next_check,NULL);
	rtcp_check_interval.tv_sec = 0;
	rtcp_check_interval.tv_usec = 250000;
	timersub(&rtcp_next_check,&rtcp_check_interval,&rtcp_last_check);
	rtcp_active = true;
}

QueueRTCPManager::~QueueRTCPManager()
{
	endQueueRTCPManager();
}

void
QueueRTCPManager::endQueueRTCPManager()
{
	rtcp_active = false;
	controlbw = sendcontrolbw = 0;
	try {
		delete [] rtcpsend_buffer;
	} catch (...) {}
	try {
		delete [] rtcprecv_buffer;
	} catch (...) {}
}

void
QueueRTCPManager::findCNAME()
{
	utsname name;
	// uname is POSIX, unlike gethostname
	int ret = uname(&name);
	
	if ( !ret ) {
		// uname not always provides the FQDN.
		struct hostent *host = gethostbyname(name.nodename);
		if ( host ) {
			const size_t HOSTFQN_LEN = 256;
			// LOGNAME environment var has two advantages:
			// 1) avoids problems of getlogin(3) and cuserid(3)
			// 2) unlike getpwuid, takes into account user 
			//    customization of env.
			const size_t USERNAME_LEN = 256;
			// Try both LOGNAME and USER env. var.
			char *user = getenv("LOGNAME");
			if ( !strcmp(user,"") )
				user = getenv("USER");

			const size_t CNAME_LEN = 
				HOSTFQN_LEN + USERNAME_LEN + 1;
			// build username@host_fqn
			char cname[CNAME_LEN];
			strncpy(cname,user,USERNAME_LEN);
			strncat(cname,"@",1);
			strncat(cname,host->h_name,HOSTFQN_LEN);
			localsrc->setSDESItem(RTCP_SDES_ITEM_CNAME,cname);
		}else {
			// TODO: exception			
		}
	} else {
		// TODO: exception
	}
}

void
QueueRTCPManager::RTCPService(microtimeout_t &wait)
{
	if ( !rtcp_active )
		return;

	microtimeout_t max_wait = rtcp_check_interval.tv_sec * 1000000 +
		rtcp_check_interval.tv_usec;
	
	// make sure the scheduling timeout is <= the check interval
	// for RTCP packets
	wait = (wait > max_wait)? max_wait : wait ;
	
	// see if there are incoming rtcp packets
	gettimeofday(&rtcp_tc,NULL);
	if ( timercmp(&rtcp_tc,&rtcp_next_check,>=) ) {
		recvControl();
		// If this do loops more than once, then we have not
		// been in time. So it skips until the next future
		// instant.
		do {
			timeval tmp = rtcp_next_check;
			timeradd(&rtcp_last_check,&rtcp_check_interval,
				 &rtcp_next_check);
			rtcp_last_check = tmp;
		} while ( timercmp(&rtcp_tc,&rtcp_next_check,>=));

	}
	
	gettimeofday(&rtcp_tc,NULL);
	if ( timercmp(&rtcp_tc,&rtcp_tn,>=) ) {
		if ( TimerReconsideration() ) {
			// update to last received RTCP packets
			recvControl();
			rtcp_last_check = rtcp_tc;
			sendControl();
			if (rtcp_initial)
				rtcp_initial = false;
			TimeOutSSRCs();
			rtcp_tp = rtcp_tc;
			// we have updated tp and sent a report, so we
			// have to recalculate the sending interval
			timeval T = computeRTCPInterval();
			timeradd(&rtcp_tc,&T,&rtcp_tn);

			// record current number of members for the
			// next check.
			rtcp_pmembers = membersCount();
		}
	} 
}

bool
QueueRTCPManager::TimerReconsideration()
{
	bool result = false;
	// compute again the interval to confirm it under current
	// circumstances
	timeval T = computeRTCPInterval();
	timeradd(&rtcp_tp,&T,&rtcp_tn);
	gettimeofday(&rtcp_tc,NULL);
	if ( timercmp(&rtcp_tc,&rtcp_tn,>=) ) {
		rtcp_tp = rtcp_tc;
		result = true;
	}
	return result;
}

void 
QueueRTCPManager::TimeOutSSRCs()
{
	// setCancel(THREAD_CANCEL_DEFERRED);

	// setCancel(THREAD_CANCEL_IMMEDIATE);
}

void
QueueRTCPManager::recvControl() 
{
	size_t len = 0;
	bool ispending = false;
	
	do {
	if ( isPendingControl(0) ) {
		len = readControl(rtcprecv_buffer,pathMTU);
		ispending = true;
	} else
		ispending = false;

	if ( !len )
		return;

	// process a <code>len<code> octets long RTCP compound packet
	// Check validity of the header fields of the compound packet 
	if ( !RTCPHeaderCheck(len) )
		return;

	// First part: The first RTCP packet in a compound packet must
	// be always a report.  
	RTCPPacket *pkt = reinterpret_cast<RTCPPacket *>(rtcprecv_buffer);

	// TODO: treat padding

	uint32 pointer = 0;
	uint16 i = 0;
	if ( pkt->fh.type == RTCP_TYPE_SR ){

		// get the sender info of the SR
		memcpy(getOrCreateSource(pkt->info.SR.ssrc).sender_info,
		       &(pkt->info.SR.sinfo),		       
		       sizeof(SenderInfo));
		// FIX: MembershipControl::updateSourceSenderInfo() 

		// get the report blocks 
		while ( i < pkt->fh.block_count ) {
			// this general RTCP manager ignores reports
			// concerning other sources than the local one
			if ( pkt->info.SR.blocks[i].ssrc ==localsrc->getID() ) {
				memcpy(getOrCreateSource(pkt->info.SR.blocks[i].ssrc).receiver_info,
				       &(pkt->info.SR.blocks[i].rinfo),
				       sizeof(ReceiverInfo));
			} else {
				// TODO: virtual for other RRs
			}
			i++;
		}
		
		// Advance to the next packet in the compound
		/*pointer += sizeof(RTCPFixedHeader) +
			sizeof(uint32) + sizeof(SenderInfo) +
			pkt->fh.block_count * sizeof(RRBlock);*/
		pointer += (pkt->fh.length << 2);

		pkt = reinterpret_cast<RTCPPacket *>(rtcprecv_buffer +pointer);

	} else if ( pkt->fh.type == RTCP_TYPE_RR ) {
		// no special initialization is required for RR reports,
		// all reports will be processed in the next do-while
	} else {
		// FIX: exception
	}

	bool valid = false;
	do {
		// get the report blocks 
		i = 0;
		while ( i < pkt->fh.block_count ) {
			// this general RTCP manager ignores reports
			// concerning other sources than the local one
			if ( pkt->info.RR.blocks[i].ssrc ==localsrc->getID() ) {
				memcpy(getOrCreateSource(pkt->info.RR.blocks[i].ssrc).receiver_info,
				       &(pkt->info.RR.blocks[i].rinfo),
				       sizeof(ReceiverInfo));
			} else {
				// TODO: virtual for other RRs
			}
			i++;
		}
		
		// Get the next packet in the compound
		/*pointer += sizeof(RTCPFixedHeader) + sizeof(uint32) +  
		  pkt->fh.block_count * sizeof(RRBlock);*/
		pointer += (pkt->fh.length << 2);

		pkt = reinterpret_cast<RTCPPacket *>(rtcprecv_buffer +pointer);
	} while (pkt->fh.type == RTCP_TYPE_RR );
	
	if ( !valid )
		return;

	// Fourth part: SDES, APP and BYE
	// process first everything but the BYE packets
	bool cname_found = false;
 	while ( (pkt->fh.type == RTCP_TYPE_SDES ||
		 pkt->fh.type == RTCP_TYPE_APP) ) {
		I ( cname_found || !pkt->fh.padding );
		bool cname = getSDES_APP(*pkt,pointer,len);
		cname_found = cname_found? cname_found : cname;
		// Get the next packet in the compound
		pkt = reinterpret_cast<RTCPPacket *>(rtcprecv_buffer +pointer);
	}
	
	if ( !valid )
		return;

	// FIX: Exception if !cname_found

	// process BYE packets
	while ( pointer < len ) {
		if ( pkt->fh.type != RTCP_TYPE_BYE ) {
			// FIX: exception non-BYE out of place
			break;
		}
		I( sizeof(RTCPFixedHeader) + pkt->fh.block_count * sizeof(uint32) 
		   < (len - pointer));
		getBYE(*pkt,pointer,len);
	}
	
	// FIX: Exception if pointer != len
	// Call plug-in parser for profile extensions at the end of the SR

	// Everything went right, update the RTCP average size
	updateAvgRTCPSize(len);

	} while (ispending);	
	return ;//1000000;
}

void
QueueRTCPManager::updateAvgRTCPSize(size_t len)
{
	uint32 newlen = len; 
	newlen += lower_headers_size;
	rtcp_avg_size = ( (15 * rtcp_avg_size) >> 4 ) + ( newlen >> 4);
}

bool
QueueRTCPManager::getBYE(RTCPPacket &pkt, uint32 &pointer, size_t len)
{
	int i = 0;
	char *reason = NULL;
	uint16 endpointer = pointer + pkt.fh.block_count * sizeof(uint32);

	if ( (sizeof(RTCPFixedHeader) + pkt.fh.block_count * sizeof(uint32))
	     < (ntohs(pkt.fh.length) << 2) ) {
		char *reason = new char[rtcprecv_buffer[endpointer] + 1 ];
		memcpy(reason,rtcprecv_buffer + endpointer + 1,
		       rtcprecv_buffer[endpointer]);
		reason[pointer] = '\0';
	} 

	while ( i < pkt.fh.block_count ){
		RTPSource &src = getSourceBySSRC(pkt.info.BYE.ssrc);
		i++;
		//pointer += sizeof(uint32);
		if(src.getGoodbye())
			gotGoodbye(src, reason);
		removeSource(pkt.info.BYE.ssrc);   // FIX:BYESource
		// REVERSE RECONSIDERATION
		ReverseReconsideration();
	}

	delete [] reason;
	pointer += (ntohs(pkt.fh.length) << 2);
	return true;
}

void
QueueRTCPManager::ReverseReconsideration()
{
	if ( membersCount() < rtcp_pmembers ) {
		timeval inc;

		// reconsider rtcp_tn (time for next RTCP packet)
		microtimeout_t t = (rtcp_tn.tv_sec - rtcp_tc.tv_sec) *
			1000000 + (rtcp_tn.tv_usec - rtcp_tc.tv_usec);
		t *= membersCount();
		t /= rtcp_pmembers;
		inc.tv_usec = t % 1000000;
		inc.tv_sec = t / 1000000;
		timeradd(&rtcp_tc,&inc,&rtcp_tn);

		// reconsider tp (time for previous RTCP packet)
		t = (rtcp_tc.tv_sec - rtcp_tp.tv_sec) * 1000000 + 
			(rtcp_tc.tv_usec - rtcp_tp.tv_usec);
		t *= membersCount();
		t /= rtcp_pmembers;
		inc.tv_usec = t % 1000000;
		inc.tv_sec = t / 1000000;
		timeradd(&rtcp_tc,&inc,&rtcp_tp);
	}
	rtcp_pmembers = membersCount();
}

bool
QueueRTCPManager::getSDES_APP(RTCPPacket &pkt, uint32 &pointer, size_t len)
{
	// Take into account that length fields in SDES items are
	// 8-bit long, so no ntoh[s|l] is required
	bool cname_found = false;

	if ( pkt.fh.type == RTCP_TYPE_APP ) {
		// FIX: FILL, call a plug-in
		pointer += ntohs((pkt.fh.length << 2)) * sizeof(uint32);
	} else if ( pkt.fh.type == RTCP_TYPE_SDES ) {
		// process chunks
		for ( int i = 0; 
		      (i < pkt.fh.block_count) && (pointer < len); i++) {
			pointer += sizeof(RTCPFixedHeader);
			SDESChunk *chunk = (SDESChunk*)(rtcprecv_buffer + pointer);
			pointer += sizeof(uint32);
			RTPSource &src = getSourceBySSRC(chunk->ssrc,true);
			bool end = false;
			while ( pointer < len && !end ) {
				SDESItem* item = &(chunk->item);
				if ( item->type > RTCP_SDES_ITEM_END && item->type <= RTCP_SDES_ITEM_H323_CADDR) {
					char *x = new char[item->len + 1];
					pointer += 2*sizeof(uint8) + len;
					memcpy(x,(const void *)(rtcprecv_buffer + pointer),item->len);
					x[item->len] = '\0';
					src.setSDESItem((sdes_item_type_t)item->type,x);	
					if ( item->type == RTCP_SDES_ITEM_CNAME) {
						cname_found = true;
					}
					
				} else if ( item->type == RTCP_SDES_ITEM_END) {
					end = true;
					pointer++;
					pointer += (pointer & 0x03);
				} else if ( item->type == RTCP_SDES_ITEM_PRIV ){
					uint8 plength = rtcprecv_buffer[pointer + 2];
					char *x = new char[plength + 1];
					pointer += 3*sizeof(uint8) + plength + item->len;						
					memcpy(x,(const void *)(rtcprecv_buffer + pointer),item->len);
					x[plength] = '\0';
					// FIX: do something with x
					x = new char[item->len + 1];
					memcpy(x,(const void *)(rtcprecv_buffer + pointer),item->len);
					x[item->len] = '\0';
					// FIX: do something with x
					delete [] x;
				} else {
					I( false );
				}
			}
			if(src.getHello())
				gotHello(src);
		}
	}

	return cname_found;
}

bool
QueueRTCPManager::RTCPHeaderCheck(size_t len) 
{
	// Note that the first packet in the compount --in order to
	// detect possibly misaddressed RTP packets-- is more
	// thoroughly checked than the following. This mask checks
	// version, padding and type.
	if ( (*(reinterpret_cast<uint16*>(rtcprecv_buffer)) & RTCP_VALID_MASK)
	     != RTCP_VALID_VALUE )
		return false;

	// this checks that every packet in the compound is tagged
	// with version == CCRTP_VERSION, and the length of the compound
	// packet matches the addition of the packets lenghts
	uint32 pointer = 0;
	RTCPPacket* pkt;
	do {
		pkt = reinterpret_cast<RTCPPacket*>
			(rtcprecv_buffer + pointer);
		pointer += ntohs((pkt->fh.length << 2));
	} while ( (pkt->fh.version == CCRTP_VERSION && pointer < len));
	if ( pointer != len )
		return false;

	return true;
}

timeval
QueueRTCPManager::computeRTCPInterval()
{	
	// TODO: change it to be more flexible
	float bw = controlbw;
	uint32 players = 0;
	if ( sendersCount() > 0 &&
	     ( sendersCount() < (membersCount() * sendcontrolbw) )) {
		// reserve "sendcontrolbw" fraction of the total RTCP
		// bandwith for senders.
		if (rtcp_we_sent) {
			// we take the side of active senders
			bw *= sendcontrolbw;
			players = sendersCount();
		} else {
			// we take the side of passive receivers
			bw *= recvcontrolbw;
			players = membersCount() - sendersCount();
		}
	}

	microtimeout_t min_interval = rtcp_min_interval;
	// be a bit quicker at first
	if ( rtcp_initial )
		min_interval /= 2;
	microtimeout_t interval = static_cast<microtimeout_t>
		((players * rtcp_avg_size / bw) * 1000000);
	if ( interval < rtcp_min_interval )
		interval = rtcp_min_interval;
	
	interval *= static_cast<microtimeout_t>(0.5 + 
						(rand() / (RAND_MAX + 1.0)));
	timeval result;
	result.tv_sec = interval / 1000000;
	result.tv_usec = interval % 1000000;
	return result;
}

void 
QueueRTCPManager::Bye(const char * const reason)
{
	// for this method, see section 6.3.7 in RFC XXXX
	// never send a BYE packet if never sent an RTP or RTCP packet
	// before
	if ( !(RTPSendCount() | RTCPSendCount()) )
		return;

	if ( membersCount() > 50) {
		// Usurp the scheduler role and apply a back-off
		// algorithm to avoid BYE floods.
		gettimeofday(&rtcp_tc,NULL);
		rtcp_tp = rtcp_tc;
		setMembersCount(1);
		setPrevMembersCount(1);
		rtcp_initial = true;
		rtcp_we_sent = false;
		rtcp_avg_size = sizeof(RTCPFixedHeader) + sizeof(uint32) +
			strlen(reason) + (4 - (strlen(reason) & 0x03));
		gettimeofday(&rtcp_tc,NULL);
		timeval T = computeRTCPInterval();
		timeradd(&rtcp_tp,&T,&rtcp_tn);
		while ( timercmp(&rtcp_tc,&rtcp_tn,<) ) {
			getOnlyBye();
			if ( TimerReconsideration() )
				break;
			gettimeofday(&rtcp_tc,NULL);
		}
	}

	sendBYE(reason);
	VDL(("Bye sent"));
} 

void
QueueRTCPManager::getOnlyBye()
{
	// This method is kind of simplified recvControl
	timeval wait;
	timersub(&rtcp_tn,&rtcp_tc,&wait);
	microtimeout_t timer = wait.tv_usec/1000 + wait.tv_sec * 1000;
	// wait up to rtcp_tn
	if ( !isPendingControl(timer) ) 
		return;

	size_t len = 0;
	do {
		len = readControl(rtcprecv_buffer,pathMTU);
		
		if ( !len )
			return;
		// Process a <code>len<code> octets long RTCP compound packet
		// Check validity of the header fields of the compound packet 
		if ( !RTCPHeaderCheck(len) )
			return;

		// TODO: treat padding

		uint32 pointer = 0;
		RTCPPacket* pkt;
		do {
			pkt = reinterpret_cast<RTCPPacket*>
				(rtcprecv_buffer + pointer);

			if (pkt->fh.type == RTCP_TYPE_BYE )
				;
			    
			pointer += ntohs((pkt->fh.length << 2));
		} while ( pointer < len );
	} while ( isPendingControl(0) );
}

size_t
QueueRTCPManager::sendControl(void)
{
	rtcp_initial = false;
	// Keep in mind: always include a report (in SR or RR) and at
	// least a SDES with the local CNAME

	// (A) SR or RR, depending on whether we sent
	// pkt will point to the several packets of the compound
	RTCPPacket *pkt = (RTCPPacket *)rtcpsend_buffer;
	// Fixed header of the first report
	pkt->fh.padding = false;

	// the fields block_count and length will be filled in later
	// now decide whether to send a SR or a SR
	if ( last_sendcount != RTPSendCount() ) {
		// we have sent rtp packets since last RTCP => send SR
		last_sendcount = RTPSendCount();
		pkt->fh.type = RTCP_TYPE_SR;

		timeval now;
		gettimeofday(&now,NULL);
		// NTP MSB and MSB
		pkt->info.SR.sinfo.NTP_msb = htonl(now.tv_sec + NTP_EPOCH_OFFSET);
		pkt->info.SR.sinfo.NTP_lsb = htonl((uint32)(((double)(now.tv_usec)*(uint32)(~0))/1000000.0));
		// RTP timestamp
		uint32 tstamp = now.tv_usec - initial_time.tv_usec;
		tstamp *= (current_rate/1000);
		tstamp /= 1000;
		tstamp += (now.tv_sec - initial_time.tv_sec) * current_rate;
		tstamp += initial_timestamp;
		pkt->info.SR.sinfo.RTPtimestamp = htonl(tstamp);
		// sender's packet and octet count
		pkt->info.SR.sinfo.packet_count = htonl(RTPSendCount());
		pkt->info.SR.sinfo.octet_count = htonl(RTPOctetCount());
	} else {
		// RR
		pkt->fh.type = RTCP_TYPE_RR;
	}
	// length of the RTCP packet, will be increasing till the end
	// of this routine. Both Report block and Sender info
	// (including the SSRC) are six 32-bit words long.
	// (sizeof(RRBlock) == sizeof (SendReport) == 6*sizeof(uint32)
	uint16 len = sizeof(RTCPFixedHeader) + sizeof(RRBlock);
	
	// (B) put report blocks
	// we have to leave room for at least a CNAME SDES item
	uint16 available = pathMTU - (2*sizeof(uint8) + CNAME_len + 6*sizeof(uint32));
	// report blocks inside the current SR/RR packet
	uint16 blocks = 0;
	// if we have to go to a new RR packet
	bool another = false;
	do {
		// pack as many report blocks as we can
		while ( (len < available) && (blocks < 31) ) {
			if ( packReportBlock(len) )
				blocks++;
			else
				break;
		}
		pkt->fh.block_count = blocks;
		// the length field specifies 32-bit words
		pkt->fh.length = htons(len >> 2);
		I( (len & 0x03) == 0 );

		I(blocks == 31 || len >=available);
		if ( blocks == 31 ) 
			another = tryAnotherRR(pkt,len,blocks);
	} while ( len < available && another );

	// (C) SDES (CNAME)
	// each SDES chunk must be 32-bit multiple long
	// fill the padding with 0s
	packSDES(len);


	// virtual for APP
	// BYE?
	
	size_t count = writeControl(rtcpsend_buffer,len);

	// Everything went right, update the RTCP average size
	updateAvgRTCPSize(len);

	return count;
}

void
QueueRTCPManager::handleSSRCCollision()
{
	// TODO: find another SSRC
	
	// take a look at the QueueRTCPManager constructor for why
	RTCPPacket* pkt = reinterpret_cast<RTCPPacket*>(rtcpsend_buffer);
	pkt->info.SR.ssrc = localsrc->getID();
}

// TODO: Check and update this
size_t
QueueRTCPManager::sendBYE(const char * const reason)
{
	// make room for the fixed header plus the SSRC identifier
	uint16 headerlen = sizeof(RTCPFixedHeader) + sizeof(uint32);
	uint16 padlen = 0;
	uint16 len = headerlen;
	if ( reason != NULL ) {
		padlen = 4 - ((1 + strlen(reason)) & 0x03);
		len += 1 + strlen(reason) + padlen;
	}
	// the length field counts 32-bit words
	I( len & 0x03 == 0 );
	// build the fixed header
	RTCPPacket *pkt = reinterpret_cast<RTCPPacket*>(rtcpsend_buffer);
	pkt->fh.version = CCRTP_VERSION;
	pkt->fh.padding = (padlen > 0);
	pkt->fh.block_count = 1;
	pkt->fh.type = RTCP_TYPE_BYE;
	pkt->fh.length = len >> 2;
	// add the SSRC identifier
	pkt->info.BYE.ssrc = getLocalInfo().getID();
	// add the optional reason
	if ( reason != NULL ){
		pkt->info.BYE.length = sizeof(reason);
		memcpy(rtcpsend_buffer + sizeof(BYEPacket),
		       reason,sizeof(reason));
		if ( padlen )
			memset(rtcpsend_buffer + sizeof(BYEPacket) 
			       + strlen(reason),
			       0,padlen);
	}
	size_t result = writeControl(rtcpsend_buffer, len);
	return result;
}

// FIX: check this
void
QueueRTCPManager::packSDES(uint16 &len)
{
	uint16 prevlen = len;

	RTCPPacket *pkt = (RTCPPacket *)(rtcpsend_buffer + len);
	pkt->fh.version = CCRTP_VERSION;
	pkt->fh.padding = 0;
	pkt->fh.block_count = 1;
	pkt->fh.type = RTCP_TYPE_SDES;
	pkt->info.SDES.ssrc = localsrc->getID();
	
	// FIX
	pkt->info.SDES.item.type =  RTCP_SDES_ITEM_CNAME;
	pkt->info.SDES.item.len = CNAME_len;
	len += sizeof(RTCPFixedHeader) + sizeof(uint32) + 2*sizeof(uint8);

	memcpy((rtcpsend_buffer + len),localsrc->getCNAME(),CNAME_len);
	len += CNAME_len;	

	pkt->fh.length = len - prevlen;

	// FIX: remote items
}

bool 
QueueRTCPManager::packReportBlock(uint16 &len)
{
	// FIX: fill
	if ( len < pathMTU ){
		len += sizeof(RRBlock);
		return true;
	} else
		return false;
}

bool 
QueueRTCPManager::tryAnotherRR(RTCPPacket *&pkt, uint16 &len, uint16 &blocks)
{
	bool result = false;
	// we need room for a new RR packet and a CNAME SDES
	uint16 available = pathMTU - 
		(2*sizeof(uint8) + 2*sizeof(uint32) + 
		 CNAME_len + sizeof(RRBlock));
	
	if ( len < available ) {
		// there may be more reports
		uint16 newlen = len + sizeof(RTCPFixedHeader) + sizeof(uint32);
		if ( packReportBlock(newlen) ) {
			result = true;
			// Header for this packet packet
			pkt = (RTCPPacket *)(rtcpsend_buffer + len);
			pkt->fh.version = CCRTP_VERSION;
			pkt->fh.padding = 0;
			blocks = 1;
			pkt->fh.type = RTCP_TYPE_RR;
			pkt->info.RR.ssrc = localsrc->getID();
			// we have appended a new Header and a report block
			len = newlen;
		} 
	} 
	return result;
}

#ifdef	__NAMESPACES__
};
#endif

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */
