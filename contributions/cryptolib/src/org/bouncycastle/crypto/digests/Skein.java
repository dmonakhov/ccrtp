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
package org.bouncycastle.crypto.digests;

import org.bouncycastle.crypto.ExtendedDigest;
import org.bouncycastle.crypto.engines.ThreefishCipher;

public class Skein implements ExtendedDigest {

    public static final int Normal = 0;

    public static final int ZeroedState = 1;

    public static final int ChainedState = 2;

    public static final int ChainedConfig = 3;

    private final byte[] schema = { 83, 72, 65, 51 }; // "SHA3"

    private ThreefishCipher _cipher;

    private int _cipherStateBits;

    private int _cipherStateBytes;

    private int _cipherStateWords;

    private int _outputBytes;

    private byte[] _inputBuffer;

    private int _bytesFilled;

    private long[] _cipherInput;

    private long[] _state;

    private int _hashSize;

    SkeinConfig Configuration;

    public UbiTweak UbiParameters;

    public int getStateSize() {
        return _cipherStateBits;
    }

    // / <summary>
    // / Initializes the Skein hash instance.
    // / </summary>
    // / <param name="stateSize">The internal state size of the hash in bits.
    // / Supported values are 256, 512, and 1024.</param>
    // / <param name="outputSize">The output size of the hash in bits.
    // / Output size must be divisible by 8 and greater than zero.</param>

    public Skein(int stateSize, int outputSize) throws IllegalArgumentException {

        setup(stateSize, outputSize);

        // Generate the configuration string
        Configuration = new SkeinConfig(this);
        Configuration.SetSchema(schema); // "SHA3"
        Configuration.SetVersion(1);
        Configuration.GenerateConfiguration();
        initialize();
    }

    public Skein(int stateSize, int outputSize, @SuppressWarnings("unused") long treeInfo, byte[] key)
            throws IllegalArgumentException {

        setup(stateSize, outputSize);

        /* compute the initial chaining state values, based on key */
        if (key.length > 0) { /* is there a key? */
            _outputBytes = _cipherStateBytes;
            UbiParameters.StartNewBlockType(UbiTweak.Key);
            update(key, 0, key.length); /* hash the key */
            byte[] preHash = finalPad();

            /* copy over into state variables */
            for (int i = 0; i < _cipherStateWords; i++)
                _state[i] = GetUInt64(preHash, i * 8);
        }
        /*
         * build/process the config block, type == CONFIG (could be precomputed
         * for each key)
         */
        _outputBytes = (outputSize + 7) / 8;

        Configuration = new SkeinConfig(this);
        Configuration.SetSchema(schema); // "SHA3"
        Configuration.SetVersion(1);

        Initialize(ChainedConfig);
    }

    private void setup(int stateSize, int outputSize)
            throws IllegalArgumentException {
        // Make sure the output bit size > 0
        if (outputSize <= 0)
            throw new IllegalArgumentException(
                    "Skein: Output bit size must be greater than zero.");

        _cipherStateBits = stateSize;
        _cipherStateBytes = stateSize / 8;
        _cipherStateWords = stateSize / 64;

        _hashSize = outputSize;
        _outputBytes = (outputSize + 7) / 8;

        // Figure out which cipher we need based on
        // the state size
        _cipher = ThreefishCipher.CreateCipher(stateSize);
        if (_cipher == null)
            throw new IllegalArgumentException("Skein: Unsupported state size.");

        // Allocate buffers
        _inputBuffer = new byte[_cipherStateBytes];
        _cipherInput = new long[_cipherStateWords];
        _state = new long[_cipherStateWords];

        // Allocate tweak
        UbiParameters = new UbiTweak();
    }

    void ProcessBlock(int bytes) {
        // Set the key to the current state
        _cipher.SetKey(_state);

        // Update tweak
        UbiParameters.addBitsProcessed(bytes);

        _cipher.SetTweak(UbiParameters.getTweak());

        // Encrypt block
        _cipher.Encrypt(_cipherInput, _state);

        // Feed-forward input with state
        for (int i = 0; i < _cipherInput.length; i++)
            _state[i] ^= _cipherInput[i];
    }

    public void update(byte[] array, int ibStart, int cbSize) {
        int bytesDone = 0;
        int offset = ibStart;

        // Fill input buffer
        while (bytesDone < cbSize && offset < array.length) {
            // Do a transform if the input buffer is filled
            if (_bytesFilled == _cipherStateBytes) {
                // Copy input buffer to cipher input buffer
                InputBufferToCipherInput();

                // Process the block
                ProcessBlock(_cipherStateBytes);

                // Clear first flag, which will be set
                // by Initialize() if this is the first transform
                UbiParameters.setFirstBlock(false);

                // Reset buffer fill count
                _bytesFilled = 0;
            }
            _inputBuffer[_bytesFilled++] = array[offset++];
            bytesDone++;
        }
    }

    public byte[] doFinal() {
        int i;

        // Pad left over space in input buffer with zeros
        // and copy to cipher input buffer
        for (i = _bytesFilled; i < _inputBuffer.length; i++)
            _inputBuffer[i] = 0;

        InputBufferToCipherInput();

        // Do final message block
        UbiParameters.setFinalBlock(true);
        ProcessBlock(_bytesFilled);

        // Clear cipher input
        for (i = 0; i < _cipherInput.length; i++)
            _cipherInput[i] = 0;

        // Do output block counter mode output
        int j;

        byte[] hash = new byte[_outputBytes];
        long[] oldState = new long[_cipherStateWords];

        // Save old state
        for (j = 0; j < _state.length; j++)
            oldState[j] = _state[j];

        for (i = 0; i < _outputBytes; i += _cipherStateBytes) {
            UbiParameters.StartNewBlockType(UbiTweak.Out);
            UbiParameters.setFinalBlock(true);
            ProcessBlock(8);

            // Output a chunk of the hash
            int outputSize = _outputBytes - i;
            if (outputSize > _cipherStateBytes)
                outputSize = _cipherStateBytes;

            PutBytes(_state, hash, i, outputSize);

            // Restore old state
            for (j = 0; j < _state.length; j++)
                _state[j] = oldState[j];

            // Increment counter
            _cipherInput[0]++;
        }
        reset();
        return hash;
    }

    private byte[] finalPad() {
        int i;

        // Pad left over space in input buffer with zeros
        // and copy to cipher input buffer
        for (i = _bytesFilled; i < _inputBuffer.length; i++)
            _inputBuffer[i] = 0;

        InputBufferToCipherInput();

        // Do final message block
        UbiParameters.setFinalBlock(true);
        ProcessBlock(_bytesFilled);

        byte[] hash = new byte[_outputBytes];

        for (i = 0; i < _outputBytes; i += _cipherStateBytes) {
            // Output a chunk of the hash
            int outputSize = _outputBytes - i;
            if (outputSize > _cipherStateBytes)
                outputSize = _cipherStateBytes;

            PutBytes(_state, hash, i, outputSize);
        }
        return hash;
    }

    // / <summary>
    // / Creates the initial state with zeros instead of the configuration
    // block, then initializes the hash.
    // / This does not start a new UBI block type, and must be done manually.
    // / </summary>
    public void Initialize(int initializationType) {
        switch (initializationType) {
        case Normal:
            // Normal initialization
            initialize();
            return;

        case ZeroedState:
            // Start with a all zero state
            for (int i = 0; i < _state.length; i++)
                _state[i] = 0;
            break;

        case ChainedState:
            // Keep the state as it is and do nothing
            break;

        case ChainedConfig:
            // Generate a chained configuration
            Configuration.GenerateConfiguration(_state);
            // Continue initialization
            initialize();
            return;
        }

        // Reset bytes filled
        _bytesFilled = 0;
    }

    public final void initialize() {
        // Copy the configuration value to the state
        for (int i = 0; i < _state.length; i++)
            _state[i] = Configuration.ConfigValue[i];

        // Set up tweak for message block
        UbiParameters.StartNewBlockType(UbiTweak.Message);

        // Reset bytes filled
        _bytesFilled = 0;
    }

    public final void initialize(long[] externalState) {
        // Copy an external saved state value to internal state
        for (int i = 0; i < _state.length; i++)
            _state[i] = externalState[i];

        // Set up tweak for message block
        UbiParameters.StartNewBlockType(UbiTweak.Message);

        // Reset bytes filled
        _bytesFilled = 0;
    }

    // Moves the byte input buffer to the long cipher input
    void InputBufferToCipherInput() {
        for (int i = 0; i < _cipherStateWords; i++)
            _cipherInput[i] = GetUInt64(_inputBuffer, i * 8);
    }

    public static long GetUInt64(byte[] b, int i) {
        if (i >= b.length + 8) {
            throw new ArrayIndexOutOfBoundsException();
        }
        return (((b[i++] & 255) | ((b[i++] & 255) << 8)
                | ((b[i++] & 255) << 16) | ((b[i++] & 255) << 24)) & 0xffffffffL)
                | (((b[i++] & 255) | ((b[i++] & 255) << 8)
                        | ((b[i++] & 255) << 16) | ((b[i] & 255L) << 24)) << 32);
    }

    public static void PutBytes(long[] input, byte[] output, int offset, int byteCount) {
        int j = 0;
        for (int i = 0; i < byteCount; i++) {
            output[offset++] = (byte) ((input[i >> 3] >> j) & 255);
            j = (j + 8) & 63;
        }
    }

    /**
     * @return the _cipherStateBits
     */
    public int get_cipherStateBits() {
        return _cipherStateBits;
    }

    /**
     * @return the hashSize int bits
     */
    public int getHashSize() {
        return _hashSize;
    }

    public String getAlgorithmName() {
        return "Skein" + _cipherStateBits;
    }

    public int getDigestSize() {
        return _outputBytes;
    }

    public void update(byte in) {
        byte[] tmp = new byte[1];
        update(tmp, 0, 1);
        // TODO Auto-generated method stub

    }

    public int doFinal(byte[] out, int outOff) {
        byte[] hash = doFinal();
        System.arraycopy(hash, 0, out, outOff, hash.length);
        return hash.length;
    }

    public void reset() {
        initialize();
    }

    public int getByteLength() {
        return _cipherStateBytes;
    }
    
    public long[] getState() {
        long[] s = new long[_state.length];
        // Copy state values to external state
        for (int i = 0; i < _state.length; i++)
            s[i] = _state[i];
        return s;
    }
    
    class SkeinConfig {
        private final int _stateSize;

        long[] ConfigValue;

        // Set the state size for the configuration
        long [] ConfigString;

        SkeinConfig(Skein sourceHash)
        {
            _stateSize = sourceHash.get_cipherStateBits();

            // Allocate config value
            ConfigValue = new long[_stateSize / 8];

            // Set the state size for the configuration
            ConfigString = new long[ConfigValue.length];
            ConfigString[1] = sourceHash.getHashSize();
        }

        void GenerateConfiguration()
        {
            ThreefishCipher cipher = ThreefishCipher.CreateCipher(_stateSize);
            UbiTweak tweak = new UbiTweak();

            // Initialize the tweak value
            tweak.StartNewBlockType(UbiTweak.Config);
            tweak.setFinalBlock(true);
            tweak.setBitsProcessed(32);

            cipher.SetTweak(tweak.getTweak());
            cipher.Encrypt(ConfigString, ConfigValue);

            ConfigValue[0] ^= ConfigString[0]; 
            ConfigValue[1] ^= ConfigString[1];
            ConfigValue[2] ^= ConfigString[2];
        }

        void GenerateConfiguration(long[] initialState)
        {
            ThreefishCipher cipher = ThreefishCipher.CreateCipher(_stateSize);
            UbiTweak tweak = new UbiTweak();

            // Initialize the tweak value
            tweak.StartNewBlockType(UbiTweak.Config);
            tweak.setFinalBlock(true);
            tweak.setBitsProcessed(32);

            cipher.SetKey(initialState);
            cipher.SetTweak(tweak.getTweak());
            cipher.Encrypt(ConfigString, ConfigValue);

            ConfigValue[0] ^= ConfigString[0];
            ConfigValue[1] ^= ConfigString[1];
            ConfigValue[2] ^= ConfigString[2];
        }

        void SetSchema(byte[] schema) throws IllegalArgumentException
        {
            if (schema.length != 4) 
                throw new IllegalArgumentException("Skein configuration: Schema must be 4 bytes.");

            long n = ConfigString[0];

            // Clear the schema bytes
            n &= ~0xffffffffL;
            // Set schema bytes
            n |= (long) schema[3] << 24;
            n |= (long) schema[2] << 16;
            n |= (long) schema[1] << 8;
            n |= schema[0];

            ConfigString[0] = n;
        }

        void SetVersion(int version) throws IllegalArgumentException
        {
            if (version < 0 || version > 3)
                throw new IllegalArgumentException("Skein configuration: Version must be between 0 and 3, inclusive.");

            ConfigString[0] &= ~((long)0x03 << 32);
            ConfigString[0] |= (long)version << 32;
        }

        void SetTreeLeafSize(byte size)
        {
            ConfigString[2] &= ~(long)0xff;
            ConfigString[2] |= size;
        }

        void SetTreeFanOutSize(byte size)
        {
            ConfigString[2] &= ~((long)0xff << 8);
            ConfigString[2] |= (long)size << 8;
        }

        void SetMaxTreeHeight(byte height) throws IllegalArgumentException
        {
            if (height == 1)
                throw new IllegalArgumentException("Skein configuration: Tree height must be zero or greater than 1.");

            ConfigString[2] &= ~((long)0xff << 16);
            ConfigString[2] |= (long)height << 16;
        }
    }

    class UbiTweak {

        static final long Key = 0, Config = 4, Personalization = 8,
                PublicKey = 12, KeyIdentifier = 16, Nonce = 20, Message = 48,
                Out = 63;

        private static final long T1FlagFinal = ((long) 1 << 63);

        private static final long T1FlagFirst = ((long) 1 << 62);

        private long[] Tweak = new long[2];

        UbiTweak() {
        }

        // / <summary>
        // / Gets or sets the first block flag.
        // / </summary>
        boolean IsFirstBlock() {
            return (Tweak[1] & T1FlagFirst) != 0;
        }

        void setFirstBlock(boolean value) {
            if (value)
                Tweak[1] |= T1FlagFirst;
            else
                Tweak[1] &= ~T1FlagFirst;
        }

        // / <summary>
        // / Gets or sets the final block flag.
        // / </summary>
        boolean isFinalBlock() {
            return (Tweak[1] & T1FlagFinal) != 0;
        }

        void setFinalBlock(boolean value) {
            if (value)
                Tweak[1] |= T1FlagFinal;
            else
                Tweak[1] &= ~T1FlagFinal;
        }

        // / <summary>
        // / Gets or sets the current tree level.
        // / </summary>
        byte getTreeLevel() {
            return (byte) ((Tweak[1] >> 48) & 0x7f);
        }

        void setTreeLevel(int value) throws Exception {
            if (value > 63)
                throw new Exception(
                        "Tree level must be between 0 and 63, inclusive.");

            Tweak[1] &= ~((long) 0x7f << 48);
            Tweak[1] |= (long) value << 48;
        }

        // / <summary>
        // / Gets or sets the number of bits processed so far, inclusive.
        // / </summary>
        long[] getBitsProcessed() {
            long[] retval = new long[2];
            retval[0] = Tweak[0];
            retval[1] = Tweak[1] & 0xffffffffL;
            return retval;
        }

        void setBitsProcessed(long value) {
            Tweak[0] = value;
        }

        void addBitsProcessed(int value) {
            final int len = 3;
            long carry = value;
            
            long words[] = new long[len];
            words[0] = Tweak[0] & 0xffffffffL;
            words[1] = ((Tweak[0] >>> 32) & 0xffffffffL);
            words[2] = (Tweak[1] & 0xffffffffL);

            for (int i = 0; i < len; i++) {
                carry += words[i];
                words[i] = carry;
                carry >>= 32;
            }        
            Tweak[0] = words[0] & 0xffffffffL;
            Tweak[0] |= (words[1] & 0xffffffffL) << 32;
            Tweak[1] |= words[2] & 0xffffffffL;
        }

        // / <summary>
        // / Gets or sets the current UBI block type.
        // / </summary>
        long getBlockType() {
            return ((Tweak[1] >> 56) & 0x3f);
        }

        void setBlockType(long value) {
            Tweak[1] = value << 56;
        }

        // / <summary>
        // / Starts a new UBI block type by setting BitsProcessed to zero, setting
        // the first flag, and setting the block type.
        // / </summary>
        // / <param name="type">The UBI block type of the new block.</param>
        void StartNewBlockType(long type) {
            setBitsProcessed(0);
            setBlockType(type);
            setFirstBlock(true);
        }

        /**
         * @return the tweak
         */
        long[] getTweak() {
            return Tweak;
        }

        /**
         * @param tweak
         *            the tweak to set
         */
        void setTweak(long[] tweak) {
            Tweak = tweak;
        }

    }

}
