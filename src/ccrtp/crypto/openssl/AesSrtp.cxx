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
 * @author Erik Eliasson <eliasson@it.kth.se>
 * @author Johan Bilien <jobi@via.ecp.fr>
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

extern void initializeOpenSSL();

#include <malloc.h>
#include <openssl/aes.h>                // the include of openSSL
#include <ccrtp/crypto/AesSrtp.h>
#include <string.h>
#include <stdio.h>

AesSrtp::AesSrtp():key(NULL){
}

AesSrtp::AesSrtp( uint8* key, int32 key_length ) {

    if (!(key_length == 16 || key_length == 32)) {
	return;
    }
    void initializeOpenSSL();

    this->key = malloc( sizeof( AES_KEY ) );
    memset( this->key, 0, sizeof( AES_KEY ) );;
    AES_set_encrypt_key( key, key_length*8, (AES_KEY *) this->key );
}

AesSrtp::~AesSrtp() {
    if (key)
	free(key);
}

void AesSrtp::encrypt( const uint8* input, uint8* output ){
    AES_encrypt(input, output, (AES_KEY *)key);
}

void AesSrtp::get_ctr_cipher_stream( uint8* output, uint32 length,
				     uint8* iv ){
    uint16 ctr;
    uint16 input;

    unsigned char* aes_input = new unsigned char[AES_BLOCK_SIZE];
    unsigned char* temp = new unsigned char[AES_BLOCK_SIZE];

    memcpy(aes_input, iv, 14 );
    iv += 14;

    for( ctr = 0; ctr < length/AES_BLOCK_SIZE; ctr++ ){
	input = ctr;
	//compute the cipher stream
	aes_input[14] = (uint8)((input & 0xFF00) >>  8);
	aes_input[15] = (uint8)((input & 0x00FF));

	AES_encrypt(aes_input, &output[ctr*AES_BLOCK_SIZE], (AES_KEY *)key );
    }
    // Treat the last bytes:
    input = ctr;
    aes_input[14] = (uint8)((input & 0xFF00) >>  8);
    aes_input[15] = (uint8)((input & 0x00FF));

    AES_encrypt(aes_input, temp, (AES_KEY *)key );
    memcpy( &output[ctr*AES_BLOCK_SIZE], temp, length % AES_BLOCK_SIZE );

    delete [] temp;
    delete [] aes_input;
}


void AesSrtp::ctr_encrypt( const uint8* input, uint32 input_length,
			   uint8* output, uint8* iv ) {

    if (key == NULL)
	return;

    uint8* cipher_stream = new uint8[input_length];

    get_ctr_cipher_stream( cipher_stream, input_length, iv );

    for( unsigned int i = 0; i < input_length; i++ ){
	output[i] = cipher_stream[i] ^ input[i];
    }
    delete []cipher_stream;
}

void AesSrtp::ctr_encrypt( uint8* data, uint32 data_length, uint8* iv ) {

    if (key == NULL)
	return;

    //unsigned char cipher_stream[data_length];
    uint8* cipher_stream = new uint8[data_length];

    get_ctr_cipher_stream( cipher_stream, data_length, iv );

    for( uint32 i = 0; i < data_length; i++ ){
	data[i] ^= cipher_stream[i];
    }
    delete[] cipher_stream;
}

void AesSrtp::f8_encrypt(const uint8* data, uint32 data_length,
			 uint8* iv, uint8* origKey, int32 keyLen,
			 uint8* salt, int32 saltLen ) {

    f8_encrypt(data, data_length, const_cast<uint8*>(data), iv, origKey, keyLen, salt, saltLen);
}

void AesSrtp::f8_encrypt(const uint8* in, uint32 in_length, uint8* out,
			 uint8* iv, uint8* origKey, int32 keyLen,
			 uint8* salt, int32 saltLen ) {


    unsigned char *saltMask;
    unsigned char *maskedKey;
    unsigned char *cp_in, *cp_in1, *cp_out;
    int i;
    int offset = 0;

    F8_CIPHER_CTX f8ctx;

    if (key == NULL)
	return;

    /*
     * Get memory for the derived IV (IV')
     */
    f8ctx.ivAccent = (unsigned char *)malloc(AES_BLOCK_SIZE);

    /*
     * Get memory for the special key. This is the key to compute the
     * derived IV (IV').
     */
    saltMask = (unsigned char *)malloc(keyLen);
    maskedKey = (unsigned char *)malloc(keyLen);

    /*
     * First copy the salt into the mask field, then fill with 0x55 to
     * get a full key.
     */
    memcpy(saltMask, salt, saltLen);
    memset(saltMask+saltLen, 0x55, keyLen-saltLen);

    /*
     * XOR the original key with the above created mask to
     * get the special key.
     */
    cp_out = maskedKey;
    cp_in = origKey;
    cp_in1 = saltMask;
    for (i = 0; i < keyLen; i++) {
        *cp_out++ = *cp_in++ ^ *cp_in1++;
    }
    /*
     * Prepare the a new AES cipher with the special key to compute IV'
     */
    AesSrtp *aes = new AesSrtp(maskedKey, keyLen);

    /*
     * Use the masked key to encrypt the original IV to produce IV'.
     *
     * After computing the IV' we don't need this cipher context anymore, free it.
     */
    aes->encrypt(iv, f8ctx.ivAccent);
    delete aes;

    memset(maskedKey, 0, keyLen);
    free(saltMask);
    free(maskedKey);                   // both values are no longer needed

    f8ctx.J = 0;                       // initialize the counter
    f8ctx.S = (unsigned char *)malloc(AES_BLOCK_SIZE);  // get the key stream buffer

    memset(f8ctx.S, 0, AES_BLOCK_SIZE); // initial value for key stream

    while (in_length >= AES_BLOCK_SIZE) {
        processBlock(&f8ctx, in+offset, AES_BLOCK_SIZE, out+offset);
        in_length -= AES_BLOCK_SIZE;
        offset += AES_BLOCK_SIZE;
    }
    if (in_length > 0) {
        processBlock(&f8ctx, in+offset, in_length, out+offset);
    }
    memset(f8ctx.ivAccent, 0, AES_BLOCK_SIZE);
    memset(f8ctx.S, 0, AES_BLOCK_SIZE);
    free(f8ctx.ivAccent);
    free(f8ctx.S);
}

int AesSrtp::processBlock(F8_CIPHER_CTX *f8ctx, const uint8* in, int32 length, uint8* out) {

    int i;
    const uint8 *cp_in;
    uint8* cp_in1, *cp_out;
    uint32_t *ui32p;

    /*
     * XOR the previous key stream with IV'
     * ( S(-1) xor IV' )
     */
    cp_in = f8ctx->ivAccent;
    cp_out = f8ctx->S;
    for (i = 0; i < AES_BLOCK_SIZE; i++) {
        *cp_out++ ^= *cp_in++;
    }
    /*
     * Now XOR (S(n-1) xor IV') with the current counter, then increment the counter
     */
    ui32p = (uint32_t *)f8ctx->S;
    ui32p[3] ^= htonl(f8ctx->J);
    f8ctx->J++;
    /*
     * Now compute the new key stream using AES encrypt
     */
    AES_encrypt(f8ctx->S, f8ctx->S, (AES_KEY *)key);
    /*
     * as the last step XOR the plain text with the key stream to produce
     * the ciphertext.
     */
    cp_out = out;
    cp_in = in;
    cp_in1 = f8ctx->S;
    for (i = 0; i < length; i++) {
        *cp_out++ = *cp_in++ ^ *cp_in1++;
    }
    return length;
}


/** EMACS **
 * Local variables:
 * mode: c++
 * c-default-style: ellemtel
 * c-basic-offset: 4
 * End:
 */
