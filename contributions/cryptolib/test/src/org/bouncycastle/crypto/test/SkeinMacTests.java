/*
Copyright (c) 2010 Werner Dittmann

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

import java.util.*; 
import org.bouncycastle.crypto.macs.SkeinMac;
import org.bouncycastle.crypto.params.*;

public class SkeinMacTests {

    static final byte[] text1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    static final byte[] key1 =  {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

    /*
     * Results computed using C Skein Mac which uses the optimized Skein
     * implementation provided by the Skein team on NIST CD, V1.3 
     */
    static final byte[] shortMacResult = {(byte)0x5A, (byte)0x56, (byte)0x4F, (byte)0x33};
    static final byte[] longMacResult = {(byte)0x14, (byte)0x31, (byte)0x79, (byte)0xF4, (byte)0x7B, (byte)0xCA, (byte)0x88, (byte)0x57};

    ParametersForSkein pfs;
    
    public SkeinMacTests() {}
    
    boolean sk512_32() {
        pfs = new ParametersForSkein(new KeyParameter(key1), 512, 31);
        SkeinMac sm = new SkeinMac();
        sm.init(pfs);
        sm.update(text1, 0, text1.length);
        byte[] mac1 = new byte[(sm.getMacSize() + 7) / 8];
        sm.doFinal(mac1, 0);
//        hexdump("First short MAC", mac1, mac1.length);
        if (!Arrays.equals(mac1, shortMacResult))
            return false;

        sm.update(text1, 0, text1.length);
        byte[]mac2 = new byte[(sm.getMacSize() + 7) / 8];
        sm.doFinal(mac2, 0);
//        hexdump("Second short MAC", mac2, mac2.length);

        if (!Arrays.equals(mac1, mac2))
            return false;

        pfs = new ParametersForSkein(new KeyParameter(key1), 512, 64);
        sm.init(pfs);
        sm.update(text1, 0, text1.length);
        mac1 = new byte[(sm.getMacSize() + 7) / 8];
        sm.doFinal(mac1, 0);
//        hexdump("First log MAC", mac1, mac1.length);
        if (!Arrays.equals(mac1, longMacResult))
            return false;

        sm.update(text1, 0, text1.length);
        mac2 = new byte[(sm.getMacSize() + 7) / 8];
        sm.doFinal(mac2, 0);
//        hexdump("Second long MAC", mac2, mac2.length);

        if (!Arrays.equals(mac1, mac2))
            return false;

        return true;
    }

    public static void main(String args[]) {

        try {
            SkeinMacTests smt = new SkeinMacTests();
            if (smt.sk512_32()) 
                System.out.println("Skein MAC test PASSED!");
            else
                System.out.println("Skein MAC test FAILE!");
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
