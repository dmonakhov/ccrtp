# Copyright (C) 1999-2002 Open Source Telecom Corporation.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AUTOMAKE_OPTIONS = no-dependencies dist-shar
LT_VERSION=@LT_CCXX_VERSION@
AM_CXXFLAGS= @WARN_FLAGS@
# RELEASE = $(LT_VERSION) -release $(LT_RELEASE)
RELEASE = $(LT_VERSION) -release $(LT_RELEASE) @GNULIBS@
LIBS = @GNULIBS@

SUBDIRS = ccrtp ccrtp/crypto/openssl ccrtp/zrtp
lib_LTLIBRARIES = libccrtp1.la

if SRTP_SUPPORT
srtp_src = ccrtp/crypto/openssl/hmac256.cxx \
		ccrtp/crypto/openssl/hmac.cxx \
		ccrtp/crypto/openssl/sha256.cxx \
		ccrtp/crypto/openssl/AesSrtp.cxx \
		ccrtp/crypto/openssl/InitializeOpenSSL.cxx
endif

if ZRTP_SUPPORT
zrtp_src = ccrtp/zrtp/ZIDFile.cxx \
		ccrtp/zrtp/ZIDRecord.cxx \
		ccrtp/zrtp/ZRtp.cxx \
		ccrtp/zrtp/ZrtpPacketCommit.cxx \
		ccrtp/zrtp/ZrtpPacketConf2Ack.cxx \
		ccrtp/zrtp/ZrtpPacketConfirm.cxx \
		ccrtp/zrtp/ZrtpPacketDHPart.cxx \
		ccrtp/zrtp/ZrtpPacketGoClear.cxx \
		ccrtp/zrtp/ZrtpPacketClearAck.cxx \
		ccrtp/zrtp/ZrtpPacketError.cxx \
		ccrtp/zrtp/ZrtpPacketHelloAck.cxx \
		ccrtp/zrtp/ZrtpPacketHello.cxx \
		ccrtp/zrtp/ZrtpStateClass.cxx \
		ccrtp/zrtp/ZrtpTextData.cxx \
		ccrtp/zrtp/Base32.cxx \
		ccrtp/zrtp/ZrtpQueue.cpp \
		ccrtp/crypto/openssl/ZrtpDH.cxx
endif

libccrtp1_la_SOURCES = rtppkt.cpp rtcppkt.cpp source.cpp data.cpp incqueue.cpp \
	outqueue.cpp queue.cpp control.cpp members.cpp socket.cpp duplex.cpp pool.cpp \
	CryptoContext.cxx $(srtp_src) $(zrtp_src)

libccrtp1_la_LDFLAGS = $(RELEASE)

noinst_HEADERS = private.h
