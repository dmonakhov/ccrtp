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

SUBDIRS = ccrtp crypto/openssl zrtp
lib_LTLIBRARIES = libccrtp1.la

if SRTP_SUPPORT
srtp_src = crypto/openssl/hmac256.cxx \
		crypto/openssl/hmac.cxx \
		crypto/openssl/sha256.cxx \
		crypto/openssl/AesSrtp.cxx \
		crypto/openssl/InitializeOpenSSL.cxx
endif

if ZRTP_SUPPORT
zrtp_src = zrtp/ZIDFile.cxx \
		zrtp/ZIDRecord.cxx \
		zrtp/ZRtp.cxx \
		zrtp/ZrtpPacketCommit.cxx \
		zrtp/ZrtpPacketConf2Ack.cxx \
		zrtp/ZrtpPacketConfirm.cxx \
		zrtp/ZrtpPacketDHPart.cxx \
		zrtp/ZrtpPacketError.cxx \
		zrtp/ZrtpPacketHelloAck.cxx \
		zrtp/ZrtpPacketHello.cxx \
		zrtp/ZrtpStateClass.cxx \
		zrtp/ZrtpTextData.cxx \
		zrtp/Base32.cxx \
		zrtp/ZrtpQueue.cpp \
		crypto/openssl/ZrtpDH.cxx
endif

libccrtp1_la_SOURCES = rtppkt.cpp rtcppkt.cpp source.cpp data.cpp incqueue.cpp \
	outqueue.cpp queue.cpp control.cpp members.cpp socket.cpp duplex.cpp pool.cpp \
	CryptoContext.cxx $(srtp_src) $(zrtp_src)

libccrtp1_la_LDFLAGS = $(RELEASE)

noinst_HEADERS = private.h
