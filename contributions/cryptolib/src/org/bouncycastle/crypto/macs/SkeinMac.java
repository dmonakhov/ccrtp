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
package org.bouncycastle.crypto.macs;

import org.bouncycastle.crypto.CipherParameters;
import org.bouncycastle.crypto.DataLengthException;
import org.bouncycastle.crypto.Mac;
import org.bouncycastle.crypto.digests.Skein;
import org.bouncycastle.crypto.params.KeyParameter;
import org.bouncycastle.crypto.params.ParametersForSkein;

public class SkeinMac implements Mac{

    private Skein skein;
    private long[] Xsave;
    
    public SkeinMac()
    {}
    
    public void init(CipherParameters params) throws IllegalArgumentException {
        ParametersForSkein p = (ParametersForSkein)params;
        KeyParameter kp = (KeyParameter)(p.getParameters());
        
        skein = new Skein(p.getStateSize(), p.getMacSize(), 0, kp.getKey());
        Xsave = skein.getState();
    }

    public String getAlgorithmName() {
        return skein.getAlgorithmName() + "/HMAC";
    }

    public int getMacSize() {
        return skein.getHashSize();
    }

    public void update(byte in) throws IllegalStateException {
        skein.update(in);
    }

    public void update(byte[] in, int inOff, int len)
            throws DataLengthException, IllegalStateException {
        skein.update(in, inOff, len);
    }

    public int doFinal(byte[] out, int outOff) throws DataLengthException,
            IllegalStateException {
        int len = skein.doFinal(out, outOff);
        reset();
        return len;
    }

    public void reset() {
        skein.initialize(Xsave);
    }

}
