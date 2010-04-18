package org.bouncycastle.crypto.prng;

import java.util.Arrays;

import org.bouncycastle.crypto.Digest;
import org.bouncycastle.crypto.BlockCipher;
import org.bouncycastle.crypto.digests.SHA256Digest;
import org.bouncycastle.crypto.engines.AESFastEngine;
import org.bouncycastle.crypto.params.KeyParameter;

/**
 * This class implements the Fortuna random number generator.
 * 
 * This class is a modified variant of the original Fortuna generator
 * in GNU Classpath (see note below). The class now uses the Bouncycastle
 * hash and cipher classes and provides a Buncycastle compliant interface.
 *
 * I did some small enhancements of the reseed loop that, otherwise the
 * algorithms were not touched.
 * 
 * License: the Bouncycastle license applies to this file. Also notice
 * the GNU Classpath license exception (see below). 
 * 
 * The Fortuna continuously-seeded pseudo-random number generator. This
 * generator is composed of two major pieces: the entropy accumulator and the
 * generator function. The former takes in random bits and incorporates them
 * into the generator's state. The latter takes this base entropy and generates
 * pseudo-random bits from it.
 * <p>
 * There are some things users of this class <em>must</em> be aware of:
 * <dl>
 * <dt>Adding Random Data</dt>
 * <dd>This class does not do any polling of random sources, but rather
 * provides an interface for adding entropy data (additional seed). Applications
 * that use this code <em>must</em> provide this mechanism. We use this design 
 * because an application writer who knows the system he is targeting is in a 
 * better position to judge what random data is available.</dd>
 * <dt>Storing the Seed</dt>
 * <dd>This class implements functions to read and restore the seed in such a 
 * way that it returns a 64 byte seed byte array to the application, and sets 
 * it back again when the application sets the seed status again. 
 * This is the extent of seed file management, however, and those
 * using this class are encouraged to think deeply about when, how often, and
 * where to store the seed.</dd>
 * </dl>
 * <p>
 * <b>References:</b>
 * <ul>
 * <li>Niels Ferguson and Bruce Schneier, <i>Practical Cryptography</i>, pp.
 * 155--184. Wiley Publishing, Indianapolis. (2003 Niels Ferguson and Bruce
 * Schneier). ISBN 0-471-22357-3.</li>
 * </ul>
 * 
 * Copyright (C) 2010 Werner Dittmann <Werner.Dittmann@t-online.de>
 */

/* 
 * Copyright note of the original Fortuna source. Not that THIS file is not
 * longer part of GNU Classpath.
 * 
 * Fortuna.java -- The Fortuna PRNG.
Copyright (C) 2004, 2006 Free Software Foundation, Inc.

This file is a part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version.  */

public class FortunaGenerator implements RandomGenerator {

    private static final int SEED_FILE_SIZE = 64;
    private static final int NUM_POOLS = 32;
    private static final int MIN_POOL_SIZE = 64;
    private final Generator generator;
    private final Digest[] pools;
    private long lastReseed = 0;
    private int pool = 0;
    private int pool0Count = 0;
    private int reseedCount = 0;

    /** A temporary buffer to serve random bytes. */
    private byte[] buffer;

    /** The index into buffer of where the next byte will come from. */
    protected int ndx;

    public FortunaGenerator(byte[] seed) {
        generator = new Generator(new AESFastEngine(), new SHA256Digest());
        pools = new Digest[NUM_POOLS];
        for (int i = 0; i < NUM_POOLS; i++)
            pools[i] = new SHA256Digest();

        buffer = new byte[256];
        generator.init(seed);
        fillBlock();
    }

    private void fillBlock()  {
        if (pool0Count >= MIN_POOL_SIZE
                && System.currentTimeMillis() - lastReseed > 100)
        {
            long powerOfTwo = 1;
            reseedCount++;
            byte[] randomBytes = new byte[pools[0].getDigestSize()];
            for (int i = 0; i < NUM_POOLS; i++) {
                if (i == 0 || reseedCount % powerOfTwo == 0) {
                    pools[i].doFinal(randomBytes, 0);
                    generator.addRandomBytes(randomBytes, 0, randomBytes.length);
                }
                else
                    break;

                powerOfTwo <<= 1;
            }
            lastReseed = System.currentTimeMillis();
            pool0Count = 0;
        }
        generator.nextBytes(buffer, 0, buffer.length);
    }

    public void nextBytes(byte[] out) {
        nextBytes(out, 0, out.length);
    }

    public void nextBytes(byte[] out, int offset, int length) {
        if (length == 0)
            return;

        if (offset < 0 || length < 0 || offset + length > out.length)
            throw new ArrayIndexOutOfBoundsException("offset=" + offset + " length="
                    + length + " limit="
                    + out.length);
        
        if (ndx >= buffer.length) {
            fillBlock();
            ndx = 0;
        }
        int count = 0;
        while (count < length) {
            int amount = Math.min(buffer.length - ndx, length - count);
            System.arraycopy(buffer, ndx, out, offset + count, amount);
            count += amount;
            ndx += amount;
            if (ndx >= buffer.length) {
                fillBlock();
                ndx = 0;
            }
        }
    }

    public void addSeedMaterial(long b) {
        pools[pool].update((byte)(b & 0xff));
        pools[pool].update((byte)((b >> 8) & 0xff));
        pools[pool].update((byte)((b >> 16) & 0xff));
        pools[pool].update((byte)((b >> 24) & 0xff));
        if (pool == 0)
            pool0Count += 4;;
        pool = (pool + 1) % NUM_POOLS;
    }

    public void addSeedMaterial(byte[] buf) {
        addSeedMaterial(buf, 0, buf.length);
    }

    public void addSeedMaterial(byte[] buf, int offset, int length) {
        pools[pool].update(buf, offset, length);
        if (pool == 0)
            pool0Count += buf.length;
        pool = (pool + 1) % NUM_POOLS;
    }

    public byte[] getSeedStatus() {
        byte[] seed = new byte[SEED_FILE_SIZE];
        generator.nextBytes(seed, 0, seed.length);
        return seed;
    }

    public void setSeedStatus(byte[] seedStatus) {
        generator.addRandomBytes(seedStatus, 0, seedStatus.length);
    }

    /**
     * The Fortuna generator function. The generator is a PRNG in its own right;
     * Fortuna itself is basically a wrapper around this generator that manages
     * reseeding in a secure way.
     */
    private static class Generator  {
        private static final int LIMIT = 1 << 20;
        private final BlockCipher cipher;
        private final Digest hash;
        private final byte[] counter;
        private final byte[] key;

        /** A temporary buffer to serve random bytes. */
        private byte[] buffer;

        /** The index into buffer of where the next byte will come from. */
        protected int ndx = 0;

        private Generator(final BlockCipher cipher, final Digest hash)
        {
            this.cipher = cipher;
            this.hash = hash;
            counter = new byte[cipher.getBlockSize()];
            buffer = new byte[cipher.getBlockSize()];
            key = new byte[32];
        }

        private void nextBytes(byte[] out, int offset, int length) {
            int count = 0;
            do {
                int amount = Math.min(LIMIT, length - count);
                nextBytesInternal(out, offset + count, amount);
                count += amount;
                for (int i = 0; i < key.length; i += counter.length) {
                    fillBlock();
                    int l = Math.min(key.length - i, cipher.getBlockSize());
                    System.arraycopy(buffer, 0, key, i, l);
                }
                resetKey();
            } while (count < length);

            fillBlock();
            ndx = 0;
        }

        private void addRandomBytes(byte[] seed, int offset, int length) {
            hash.update(key, 0, key.length);
            hash.update(seed, offset, length);
            byte[] newkey = new byte[hash.getDigestSize()];
            hash.doFinal(newkey, 0);
            System.arraycopy(newkey, 0, key, 0, Math.min(key.length, newkey.length));
            resetKey();
            incrementCounter();
        }

        private void fillBlock() {
            cipher.processBlock(counter, 0, buffer, 0);
            incrementCounter();
        }

        private void init(byte[] seed) {
            Arrays.fill(key, (byte) 0);
            Arrays.fill(counter, (byte) 0);
            if (seed != null)
                addRandomBytes(seed, 0, seed.length);
            fillBlock();
        }

        private void nextBytesInternal(byte[] out, int offset, int length) {

            if (length == 0)
                return;

            if (offset < 0 || length < 0 || offset + length > out.length)
                throw new ArrayIndexOutOfBoundsException("offset=" + offset + " length="
                        + length + " limit="
                        + out.length);
            if (ndx >= buffer.length) {
                fillBlock();
                ndx = 0;
            }
            int count = 0;
            while (count < length) {
                int amount = Math.min(buffer.length - ndx, length - count);
                System.arraycopy(buffer, ndx, out, offset + count, amount);
                count += amount;
                ndx += amount;
                if (ndx >= buffer.length) {
                    fillBlock();
                    ndx = 0;
                }
            }
        }

        /**
         * Resets the cipher's key. This is done after every reseed, which combines
         * the old key and the seed, and processes that throigh the hash function.
         */
        private void resetKey() {
            cipher.reset();
            cipher.init(true, new KeyParameter(key));
        }

        /**
         * Increment `counter' as a sixteen-byte little-endian unsigned integer by
         * one.
         */
        private void incrementCounter() {
            for (int i = 0; i < counter.length; i++) {
                counter[i]++;
                if (counter[i] != 0)
                    break;
            }
        }
    }
}

