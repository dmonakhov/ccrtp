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
CXXFLAGS = @CXXFLAGS@ -I$(top_srcdir)/src
ccxxincludedir=$(includedir)/cc++

noinst_PROGRAMS = rtphello rtpduphello audiorx audiotx

rtphello_SOURCES = rtphello.cpp
rtphello_LDFLAGS = ../src/libccrtp.la

rtpduphello_SOURCES = rtpduphello.cpp
rtpduphello_LDFLAGS = ../src/libccrtp.la

audiorx_SOURCES = audiorx.cpp
audiorx_LDFLAGS = ../src/libccrtp.la

audiotx_SOURCES = audiotx.cpp
audiotx_LDFLAGS = ../src/libccrtp.la
