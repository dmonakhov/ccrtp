# Copyright (C) 1999-2001 Open Source Telecom Corporation.
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
LDFLAGS = @LDFLAGS@ $(LT_VERSION) -release $(LT_RELEASE)
LIBS=@GNULIBS@
ccxxincludedir=$(includedir)/cc++2/cc++

lib_LTLIBRARIES = libccrtp1.la

libccrtp1_la_SOURCES = packet.cpp socket.cpp queue.cpp duplex.cpp \
		      control.cpp data.cpp members.cpp source.cpp

ccxxinclude_HEADERS = rtp.h rtpext.h
noinst_HEADERS = private.h

kdoc_headers = rtp.h rtpext.h

kdoc:
	kdoc -f html -d $(KDOC_DIR) -L$(KDOC_DIR) -n ccrtp $(kdoc_headers)


