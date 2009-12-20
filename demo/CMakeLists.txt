
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(rtpsend_SRCS rtpsend.cpp)
add_executable(rtpsend ${rtpsend_SRCS})
target_link_libraries(rtpsend ccrtp1)


########### next target ###############

set(rtplisten_SRCS rtplisten.cpp)
add_executable(rtplisten ${rtplisten_SRCS})
target_link_libraries(rtplisten ccrtp1)


########### next target ###############

set(rtphello_SRCS rtphello.cpp)
add_executable(rtphello ${rtphello_SRCS})
target_link_libraries(rtphello ${KDE4_KDECORE_LIBS} ccrtp1)


########### next target ###############

set(rtpduphello_SRCS rtpduphello.cpp)
add_executable(rtpduphello ${rtpduphello_SRCS})
target_link_libraries(rtpduphello ccrtp1)


########### next target ###############

set(audiorx_SRCS audiorx.cpp audio.h)
add_executable(audiorx ${audiorx_SRCS})
target_link_libraries(audiorx ccrtp1)


########### next target ###############

set(audiotx_SRCS audiotx.cpp)
add_executable(audiotx ${audiotx_SRCS})
target_link_libraries(audiotx ccrtp1)

########### next target ###############

set(ccrtptest_SRCS ccrtptest.cpp)
add_executable(ccrtptest ${ccrtptest_SRCS})
target_link_libraries(ccrtptest ccrtp1)

########### next target ###############

if (SRTP_SUPPORT)
  set(ccsrtptest_SRCS ccsrtptest.cpp)
  add_executable(ccsrtptest ${ccsrtptest_SRCS})
  target_link_libraries(ccsrtptest ccrtp1)
endif()

########### install files ###############
src_distribution_init()
add_src_dist_files(${rtpsend_SRCS} 
                   ${rtplisten_SRCS}
                   ${rtphello_SRCS}
                   ${rtpduphello_SRCS} 
                   ${audiorx_SRCS} 
                   ${audiotx_SRCS} 
                   ${ccrtptest_SRCS} 
                   ${ccsrtptest_SRCS})

#install(FILES  DESTINATION include/cc++)



#original Makefile.am contents follow:

## Copyright (C) 1999-2005 Open Source Telecom Corporation.
## Copyright (C) 2006-2008 David Sugar, Tycho Softworks.
##
## This file is free software; as a special exception the author gives
## unlimited permission to copy and/or distribute it, with or without
## modifications, as long as this notice is preserved.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
## implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#AUTOMAKE_OPTIONS = no-dependencies dist-shar
#LT_VERSION=@LT_CCXX_VERSION@
##ignored for programs
##$(LT_VERSION) -release $(LT_RELEASE)
#AM_CXXFLAGS = @WARN_FLAGS@ -I$(top_srcdir)/src
#ccxxincludedir=$(includedir)/cc++
#
#if SRTP_GCRYPT
#srtp_src = ccsrtptest
#ccsrtptest_SOURCES = ccsrtptest.cpp
#ccsrtptest_LDFLAGS = ../src/libccrtp1.la
#endif
#
#if SRTP_OPENSSL
#srtp_src = ccsrtptest
#ccsrtptest_SOURCES = ccsrtptest.cpp
#ccsrtptest_LDFLAGS = ../src/libccrtp1.la
#endif
#
#noinst_PROGRAMS = rtpsend rtplisten rtphello rtpduphello audiorx audiotx \
#	ccrtptest $(srtp_src)
#
#rtpsend_SOURCES = rtpsend.cpp
#rtpsend_LDFLAGS = ../src/libccrtp1.la
#
#rtplisten_SOURCES = rtplisten.cpp
#rtplisten_LDFLAGS = ../src/libccrtp1.la
#
#rtphello_SOURCES = rtphello.cpp
#rtphello_LDFLAGS = ../src/libccrtp1.la
#
#rtpduphello_SOURCES = rtpduphello.cpp
#rtpduphello_LDFLAGS = ../src/libccrtp1.la
#
#audiorx_SOURCES = audiorx.cpp audio.h
#audiorx_LDFLAGS = ../src/libccrtp1.la
#
#audiotx_SOURCES = audiotx.cpp
#audiotx_LDFLAGS = ../src/libccrtp1.la
#
#ccrtptest_SOURCES = ccrtptest.cpp
#ccrtptest_LDFLAGS = ../src/libccrtp1.la
