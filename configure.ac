# Copyright (C) 1999-2003 Open Source Telecom Corporation.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AC_INIT(src/ccrtp/rtp.h)

cfg_options=""
if test -z "$*" ; then
        if test -f ~/.configure ; then
		cfg_options=`grep ^ccrtp: ~/.configure | sed -e s/^ccrtp://`
        elif test -f /etc/configure.conf ; then
		cfg_options=`grep ^ccrtp: /etc/configure.conf | sed -e s/^ccrtp://`
        fi
fi

if test ! -z "$cfg_options" ; then
        echo "using ~/.configure defaults...$cfg_options"
        case "$SHELL" in
        */bash*)
                exec $SHELL $0 $cfg_options
                exit -1
                ;;
        esac
        exec $0 $cfg_options
        exit -1
fi

VERSION="1.4.1"
REQUIRES="1.3.0"
LT_RELEASE="1.4"
LT_VERSION="0:1"
AC_CONFIG_AUX_DIR(autoconf)
AC_CANONICAL_SYSTEM
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_CXX
# OST_PROG_CC_POSIX
AM_PROG_LIBTOOL
AM_INIT_AUTOMAKE(ccrtp,[$VERSION])
AM_CONFIG_HEADER(src/config.h)

# SRTP support
AC_ARG_ENABLE(srtp,
    AS_HELP_STRING([--enable-srtp],
        [enable SRTP support (default enabled)]))
if test -z "$enable_srtp"; then
   enable_srtp="yes"
fi
if test "${enable_srtp}" = "yes"
then
   AC_DEFINE(SRTP_SUPPORT, [], [SRTP support])
fi
AM_CONDITIONAL(SRTP_SUPPORT, test "${enable_srtp}" = "yes")

# ZRTP support
AC_ARG_ENABLE(zrtp,
    AS_HELP_STRING([--enable-zrtp],
        [enable ZRTP support (default disabled)]))
if test -z "$enable_zrtp"; then
   enable_zrtp = "no"
fi
if test "${enable_zrtp}" = "yes"
then
    if test "${enable_srtp}" != "yes"
    then
        AC_MSG_ERROR([You need SRTP support to enable ZRTP])
    fi
    AC_DEFINE(ZRTP_SUPPORT, [], [ZRTP support])
fi
AM_CONDITIONAL(ZRTP_SUPPORT, test "${enable_zrtp}" = "yes")

# check availability of openSSL crypto library if SRTP support enabled
if test "${enable_srtp}" = "yes"
then
    AC_CHECK_LIB([crypto], [EVP_CipherInit_ex],
                 [], AC_MSG_ERROR([OpenSSL crypto library not found.]))
fi

OST_PROG_COMMON
OST_PROG_LIBVER
OST_PROG_LIBRARY(CCXX,[$LT_VERSION])
OST_AUTOMAKE_MODE
OST_MAINTAINER_MODE
OST_CCXX2_VERSION([$REQUIRES])
# OST_CC_ENDIAN - now in Common C++
# Are we using the GNU compiler?
if test $GCC = yes ; then
	WARN_FLAGS="-Wall -ansi -pedantic"
else
	WARN_FLAGS=""
fi
AC_SUBST(WARN_FLAGS)

OST_DEBUG

dnl --with-nana: check for nana, the GNU assertion checking and logging library
AC_ARG_WITH(nana,[  --with-nana             use GNU nana for asserts and logging],[
	AC_CHECK_HEADER(nana.h, [
		AC_CHECK_HEADERS(DL.h, [
			AC_CHECK_LIB(nana, main)
			])
	])
])

# check availability of openSSL crypto headers if SRTP support enabled
if test "${enable_srtp}" = "yes"
then
     AC_CHECK_HEADERS([openssl/bn.h openssl/aes.h openssl/sha.h],
                      [], AC_MSG_ERROR([OpenSSL headers not found.]))
fi

KDOC_DIR="\$(top_srcdir)/doc"
AC_SUBST(LT_RELEASE)
AC_SUBST(KDOC_DIR)

AC_PATH_PROG(DOXYGEN, doxygen, no)
AC_SUBST(DOXYGEN)
AM_CONDITIONAL(DOXY, test "$DOXYGEN" != "no")

AC_OUTPUT(Makefile src/Makefile src/ccrtp/Makefile m4/Makefile
doc/Makefile demo/Makefile phone/Makefile w32/Makefile ccrtp.spec
ccrtp.list libccrtp1.pc)
