
#include <stdio.h>

#include <private.h>

#include <gcrypt.h>

/** Implement the locking callback functions for libgcrypt.
 *
 * Unfortunatly we can't use the Commonc++ Mutex here because the
 * Mutex may use (for some cases) the Commonc++ Thread class. OpenSSL
 * does not use this Thread class.
 */

static int initialized = 0;

int initializeGcrypt ()
{

    if (initialized) {
	return 1;
    }
    gcry_check_version(NULL);
    gcry_control(GCRYCTL_DISABLE_SECMEM);
    initialized = 1;
    // threadLockSetup();
    return 1;
}
