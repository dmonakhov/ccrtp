// Copyright (C) 2000-2001 Open Source Telecom Corporation.
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
 
#ifndef __CCXX_PHONE_H__
#define __CCXX_PHONE_H__
 
#ifndef __CCXX_RTP_H__
#include <cc++/rtp.h>
#endif
 
#ifdef	__NAMESPACES__
namespace ost {
#endif

/**
 * Load /etc/phone.conf [thread] key value pairs.  Has internal defaults
 * if section or file is missing.
 *
 * @author David Sugar <dyfet@ostel.com>
 * @short Load keythreads priority and session count configuration.
 */
class KeyRTP : public Keydata
{
public:
        /**
         * Initialize keythread data.
         */
        KeyRTP();
	
	/**
	 * Get unicast address.
	 */
	inline InetAddress getInterface(void)
		{return InetAddress(getLast("interface"));};
	
	/**
	 * Get binding port number.
	 */
	inline tpport_t getPort(void)
		{return (tpport_t) atoi(getLast("port"));};

	/**
	 * Get stack timer.
	 */
	inline microtimeout_t getTimer(void)
		{return (microtimeout_t)atol(getLast("timer")) * 1000l;};

	/**
	 * Get packet expiration timer.
	 */
	inline microtimeout_t getExpire(void)
		{return (microtimeout_t)atol(getLast("expire")) * 1000l;};
};

/**
 * Load /etc/phone.conf [thread] key value pairs.  Has internal defaults
 * if section or file is missing.
 *
 * @author David Sugar <dyfet@ostel.com>
 * @short Load keythreads priority and session count configuration.
 */
class KeyThreads : public Keydata
{
public:
        /**
         * Initialize keythread data.
         */
        KeyThreads();
 
        /**
         * Get relative priority to run service threads at.
         *
         * @return audio thread priority (relative).
         */
        inline int priAudio(void)
                {return atoi(getLast("audio"));};

        /**
         * Get relative priority for the rtp stack.
         *
         * @return audio thread priority (relative).
         */
        inline int priRTP(void)
                {return atoi(getLast("rtp"));};

        /**
         * Get relative process priority.
         *
         * @return rtp stack thread priority (relative).
         */
        inline int getPriority(void)
                {return atoi(getLast("priority"));};

        /**
         * Get thread stack frame size.
         *
         * @return thread stack frame in k.
         */
        inline unsigned getStack(void)
                {return atoi(getLast("stack"));};
		

        /**
         * Get scheduler policy to use.
         *
         * @return scheduler policy.
         */
        inline const char *getPolicy(void)
                {return getLast("priority");};
};	

/**
 * This is the base session stack that will maintain all network audio
 * activity.
 *
 * @author David Sugar <dyfet@ostel.com>
 * @short RTP stack for network audio. 
 */
class RTPAudio : public RTPSocket
{
public:
	RTPAudio();
};

extern bool multicast;
extern bool daemon;
extern KeyThreads keythreads;
extern KeyRTP keyrtp;
extern RTPAudio *rtp;

#ifdef	__NAMESPACES__
};
#endif

#endif
