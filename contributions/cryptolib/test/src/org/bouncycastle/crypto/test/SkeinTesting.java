/*
Copyright (c) 2010 Alberto Fajardo

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

The tweaks and modifications for Java:
Copyright (c) 2010, Werner Dittmann. 

The same permissions granted.
*/

package org.bouncycastle.crypto.test;

import org.bouncycastle.crypto.digests.*;

public class SkeinTesting {

    public SkeinTesting()
    {
    }

    /// <summary>
    /// Benchmarks this instance of the Skein hash function.
    /// </summary>
    /// <param name="iterations">Number of hash computations to perform.</param>
    /// <returns>Resulting speed in megabytes per second.</returns>
    public double Benchmark(long iterations, Skein skein)
    {
        int hashBytes = skein.getHashSize()/8;
        byte[] hash = new byte[hashBytes];
        
        long start = System.currentTimeMillis();

        for (long i = 0; i < iterations; i++)
            skein.update(hash, 0, hashBytes);

        hash = skein.doFinal();
        long stop = System.currentTimeMillis();
        
        long duration = stop - start;

        double opsPerTick = iterations / (double)duration;
        double opsPerSec = opsPerTick * 1000;

        double mbs = opsPerSec * hashBytes  / 1024 / 1024;
        System.out.println("duration: " + duration + "ms, ops per tick: " + opsPerTick);
        System.out.println("ops per sec: " + opsPerSec + ", mbs: " + mbs);
         
        return 0.0;
    }

    /// <summary>
    /// Tests the 256, 512, and 1024 bit versions of Skein against
    /// known test vectors.
    /// </summary>
    /// <returns>True if the test succeeded without errors, false otherwise.</returns>
    public boolean TestHash() throws Exception
    {
        Skein skein256 = new Skein(256, 256);
        Skein skein512 = new Skein(512, 512);
        Skein skein512_256 = new Skein(512, 256);
        Skein skein1024 = new Skein(1024, 1024);

        byte[] result256 = {
            (byte)0xDF, 0x28, (byte)0xE9, 0x16, 0x63, 0x0D, 0x0B, 0x44, 
            (byte)0xC4,(byte) 0xA8, 0x49, (byte)0xDC, (byte)0x9A, 0x02, (byte)0xF0, 0x7A,
            0x07, (byte)0xCB, 0x30, (byte)0xF7, 0x32, 0x31, (byte)0x82, 0x56, 
            (byte)0xB1, 0x5D, (byte)0x86, 0x5A, (byte)0xC4, (byte)0xAE, 0x16, 0x2F
        };

        byte[] result512 = { 
            (byte)0x91 ,(byte)0xcc ,(byte)0xa5 ,0x10 ,(byte)0xc2 ,0x63 ,(byte)0xc4 ,(byte)0xdd ,(byte)0xd0 ,0x10 ,0x53 ,0x0a ,0x33 ,0x07 ,0x33 ,0x09,
            0x62 ,(byte)0x86 ,0x31 ,(byte)0xf3 ,0x08 ,0x74 ,0x7e ,0x1b ,(byte)0xcb ,(byte)0xaa ,(byte)0x90 ,(byte)0xe4 ,0x51 ,(byte)0xca ,(byte)0xb9 ,0x2e,
            0x51 ,(byte)0x88 ,0x08 ,0x7a ,(byte)0xf4 ,0x18 ,(byte)0x87 ,0x73 ,(byte)0xa3 ,0x32 ,0x30 ,0x3e ,0x66 ,0x67 ,(byte)0xa7 ,(byte)0xa2,
            0x10 ,(byte)0x85 ,0x6f ,0x74 ,0x21 ,0x39 ,0x00 ,0x00 ,0x71 ,(byte)0xf4 ,(byte)0x8e ,(byte)0x8b ,(byte)0xa2 ,(byte)0xa5 ,(byte)0xad ,(byte)0xb7
        };

        byte[] result1024 = {
            0x1F, 0x3E, 0x02, (byte)0xC4, 0x6F, (byte)0xB8, 0x0A, 0x3F, (byte)0xCD, 0x2D, (byte)0xFB, (byte)0xBC, 0x7C, 0x17, 0x38, 0x00,
            (byte)0xB4, 0x0C, 0x60, (byte)0xC2, 0x35, 0x4A, (byte)0xF5, 0x51, 0x18, (byte)0x9E, (byte)0xBF, 0x43, 0x3C, 0x3D, (byte)0x85, (byte)0xF9,
            (byte)0xFF, 0x18, 0x03, (byte)0xE6, (byte)0xD9, 0x20, 0x49, 0x31, 0x79, (byte)0xED, 0x7A, (byte)0xE7, (byte)0xFC, (byte)0xE6, (byte)0x9C, 0x35,
            (byte)0x81, (byte)0xA5, (byte)0xA2, (byte)0xF8, 0x2D, 0x3E, 0x0C, 0x7A, 0x29, 0x55, 0x74, (byte)0xD0, (byte)0xCD, 0x7D, 0x21, 0x7C,
            0x48, 0x4D, 0x2F, 0x63, 0x13, (byte)0xD5, (byte)0x9A, 0x77, 0x18, (byte)0xEA, (byte)0xD0, 0x7D, 0x07, 0x29, (byte)0xC2, 0x48,
            0x51, (byte)0xD7, (byte)0xE7, (byte)0xD2, 0x49, 0x1B, (byte)0x90, 0x2D, 0x48, (byte)0x91, (byte)0x94, (byte)0xE6, (byte)0xB7, (byte)0xD3, 0x69, (byte)0xDB,
            0x0A, (byte)0xB7, (byte)0xAA, 0x10, 0x6F, 0x0E, (byte)0xE0, (byte)0xA3, (byte)0x9A, 0x42, (byte)0xEF, (byte)0xC5, 0x4F, 0x18, (byte)0xD9, 0x37,
            0x76, 0x08, 0x09, (byte)0x85, (byte)0xF9, 0x07, 0x57, 0x4F, (byte)0x99, 0x5E, (byte)0xC6, (byte)0xA3, 0x71, 0x53, (byte)0xA5, 0x78
        };

        // Hashes are computed twice to make sure the hasher
        // re-initializes itself properly

        byte[] hash;
        int i;

        // Make test vector for 256-bit hash
        byte[] testVector = new byte[64];

//        testVector[0] = 0;
//        skein512_256.update(testVector, 0, 0);
//        hash = skein512_256.doFinal();
//        hexdump("0-0", hash, hash.length);
        
        for (i = 0; i < testVector.length; i++)
            testVector[i] = (byte) (255 - i);

        skein256.update(testVector, 0, testVector.length);
        hash = skein256.doFinal();

        // Compare with 256-bit test vector)
        for (i = 0; i < result256.length; i++)
            if (hash[i] != result256[i]) return false;      

        // Make the test vector for the 512 and 1024-bit hash
        testVector = new byte[128];
        for (i = 0; i < testVector.length; i++)
            testVector[i] = (byte)(255 - i);

        skein512.update(testVector, 0, testVector.length);
        hash = skein512.doFinal();

        // Compare with 512-bit test vector
        for (i = 0; i < result512.length; i++)
            if (hash[i] != result512[i]) return false;

        skein1024.update(testVector, 0, testVector.length);
        hash = skein1024.doFinal();

        // Compare with 1024-bit test vector
        for (i = 0; i < result1024.length; i++)
            if (hash[i] != result1024[i]) return false;

        return true;
    }
    
    public static void main(String args[]) {

        try {
            SkeinTesting skt = new SkeinTesting();
            if (skt.TestHash()) {
                System.out.println("Self test PASSED!");
                skt.Benchmark(1000000, new Skein(512, 512));
                skt.Benchmark(10000000, new Skein(512, 512));
            }
            else
                System.out.println("Self test FAILED!");
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        System.out.println("Skein test done");
    }
    
    private static final char[] hex = "0123456789abcdef".toCharArray();

    /**
     * Dump a buffer in hex and readable format.
     * 
     * @param title Printed at the beginning of the dump
     * @param buf   Byte buffer to dump
     * @param len   Number of bytes to dump, should be less or equal 
     *              the buffer length
     */
    public static void hexdump(String title, byte[] buf, int len) {
        byte b;
        System.err.println(title);
        for(int i = 0 ; ; i += 16) {
            for(int j=0; j < 16; ++j) {
                if (i+j >= len) {
                    System.err.print("   ");
                }
                else {
                    b = buf[i+j];
                    System.err.print(" "+ hex[(b>>>4) &0xf] + hex[b&0xf] );
                }
            }
            System.err.print("  ");
            for(int j = 0; j < 16; ++j) {
                if (i+j >= len) break;
                b = buf[i+j];
                if ( (byte)(b+1) < 32+1) {
                    System.err.print( '.' );
                }
                else {
                    System.err.print( (char)b );
                }
            }
            System.err.println();
            if (i+16 >= len) {
                break;
            }
        }
    }

}

