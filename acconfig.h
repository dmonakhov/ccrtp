/* src/config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Name of package */
#undef PACKAGE

/* Version number of package */
#undef VERSION

#undef	HAVE_ENDIAN_H
#undef	__BYTE_ORDER

#ifdef	HAVE_ENDIAN_H
#include <endian.h>
#else
#define	__LITTLE_ENDIAN	1234
#define	__BIG_ENDIAN	4321
#endif

#undef HAVE_DEV_URANDOM
