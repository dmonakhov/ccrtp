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
LDFLAGS = @LDFLAGS@ $(LT_VERSION) -release $(LT_RELEASE)
LIBS=@GNULIBS@

SUBDIRS = ccrtp
lib_LTLIBRARIES = libccrtp1.la

libccrtp1_la_SOURCES = rtppkt.cpp rtcppkt.cpp \
	source.cpp data.cpp \
	incqueue.cpp outqueue.cpp queue.cpp \
	control.cpp members.cpp \
	socket.cpp duplex.cpp pool.cpp

noinst_HEADERS = private.h
