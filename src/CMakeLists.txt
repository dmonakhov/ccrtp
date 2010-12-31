## Copyright (C) 1999-2005 Open Source Telecom Corporation.
## Copyright (C) 2006-2008 David Sugar, Tycho Softworks.
## Copyright (C) 2009-2010 Werner Dittmann
##
## This file is free software; as a special exception the author gives
## unlimited permission to copy and/or distribute it, with or without
## modifications, as long as this notice is preserved.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
## implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

########### next target ###############

set(openssl_srcs ccrtp/crypto/openssl/hmac.cpp
		 ccrtp/crypto/openssl/AesSrtp.cxx
		 ccrtp/crypto/openssl/InitializeOpenSSL.cxx)

set(gcrypt_srcs ccrtp/crypto/gcrypt/gcrypthmac.cxx
		ccrtp/crypto/gcrypt/gcryptAesSrtp.cxx
		ccrtp/crypto/gcrypt/InitializeGcrypt.cxx)

set(skein_srcs ccrtp/crypto/macSkein.cpp
        ccrtp/crypto/skein.c
        ccrtp/crypto/skein_block.c
        ccrtp/crypto/skeinApi.c)

if (SRTP_SUPPORT)
  if(GCRYPT_FOUND)
    set(crypto_srcs ${gcrypt_srcs})
  else()
    set(crypto_srcs ${openssl_srcs})
  endif()
endif()

set(ccrtp1_srcs
    rtppkt.cpp
    rtcppkt.cpp
    source.cpp
    data.cpp
    incqueue.cpp
    outqueue.cpp
    queue.cpp
    control.cpp
    members.cpp
    socket.cpp
    duplex.cpp
    pool.cpp
    CryptoContext.cxx)

include(SourceDistribution)
src_distribution_init()
add_src_dist_files(${ccrtp1_srcs} ${gcrypt_srcs} ${openssl_srcs} ${skein_srcs}
		   config.h.cmake private.h)

add_subdirectory(ccrtp)
add_src_dist_dirs(ccrtp)

add_library(ccrtp1 SHARED ${ccrtp1_srcs} ${crypto_srcs} ${skein_srcs})
set_target_properties(ccrtp1 PROPERTIES VERSION ${VERSION} SOVERSION ${LT_RELEASE})

message("src libraries: ${LIBS}")
target_link_libraries(ccrtp1 ${LIBS})

########### install files ###############

install(TARGETS ccrtp1 DESTINATION lib${LIB64})
