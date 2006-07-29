/*
  Copyright (C) 2005, 2004 Erik Eliasson, Johan Bilien

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
 * Functions to compute SHA1 HAMAC.
 *
 * @author Erik Eliasson <eliasson@it.kth.se>
 * @author Johan Bilien <jobi@via.ecp.fr>
 */

#ifndef HMAC_H
#define HMAC_H

#include <private.h>

/**
 * Compute SHA1 HMAC.
 *
 * This functions takes one data chunk and computes its SHA1 HMAC. It uses
 * the openSSL HAMAC SHA1 implementation.
 *
 * @param key
 *    The MAC key.
 * @param key_length
 *    Lneght of the MAC key in bytes
 * @param data
 *    Points to the data chunk.
 * @param data_length
 *    Length of the data in bytes
 * @param mac
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 20 bytes (SHA1_DIGEST_LENGTH).
 * @param mac_length
 *    Point to an integer that receives the length of the computed HMAC.
 */

__EXPORT void hmac_sha1( uint8* key, int32 key_length,
                const uint8* data, uint32 data_length,
                uint8* mac, int32* mac_length );

/**
 * Compute SHA1 HMAC over several data cunks.
 *
 * This functions takes several data chunk and computes the SHA1 HAMAC. It
 * uses the openSSL HAMAC SHA1 implementation.
 *
 * @param key
 *    The MAC key.
 * @param key_length
 *    Lneght of the MAC key in bytes
 * @param data
 *    Points to an array of pointers that point to the data chunks. A NULL
 *    pointer in an array element terminates the data chunks.
 * @param data_length
 *    Points to an array of integers that hold the length of each data chunk.
 * @param mac
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 20 bytes (SHA1_DIGEST_LENGTH).
 * @param mac_length
 *    Point to an integer that receives the length of the computed HMAC.
 */
__EXPORT void hmac_sha1( uint8* key, int32 key_length,
                const uint8* data[], uint32 data_length[],
                uint8* mac, int32* mac_length );

#endif
