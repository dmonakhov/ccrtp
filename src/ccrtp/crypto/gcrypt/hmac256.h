/*
  Copyright (C) 2006, 2005, 2004 Erik Eliasson, Johan Bilien, Werner Dittmann

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

 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

/**
 * Methods to compute a SHA256 HMAC.
 *
 * @author Erik Eliasson <eliasson@it.kth.se>
 * @author Johan Bilien <jobi@via.ecp.fr>
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#ifndef HMAC_SHA256_H
#define HMAC_SHA256_H

#include <cc++/config.h>
#include <gcrypt.h>

#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH 32
#endif

/**
 * Compute SHA256 HMAC.
 *
 * This functions takes one data chunk and computes its SHA256 HMAC. It uses
 * the openSSL HAMAC SHA256 implementation.
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
 *    buffer must have a size of at least 32 bytes (SHA256_DIGEST_LENGTH).
 * @param mac_length
 *    Point to an integer that receives the length of the computed HMAC.
 */
__EXPORT void hmac_sha256( uint8* key, uint32 key_length,
			   uint8* data, int32 data_length,
			   uint8* mac, uint32* mac_length );

/**
 * Compute SHA256 HMAC over several data cunks.
 *
 * This functions takes several data chunk and computes the SHA256 HAMAC. It
 * uses the openSSL HAMAC SHA256 implementation.
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
 *    buffer must have a size of at least 32 bytes (SHA256_DIGEST_LENGTH).
 * @param mac_length
 *    Point to an integer that receives the length of the computed HMAC.
 */

__EXPORT void hmac_sha256( uint8* key, uint32 key_length,
			   uint8* data[], uint32 data_length[],
			   uint8* mac, uint32 * mac_length );
#endif
