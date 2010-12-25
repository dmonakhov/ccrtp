package org.bouncycastle.crypto.params;

import org.bouncycastle.crypto.CipherParameters;

public class ParametersForSkein implements CipherParameters {

    private int macSize;
    private int stateSize;
    private CipherParameters    parameters;

    public ParametersForSkein(
            CipherParameters    parameters,
            int             stateSize,
            int              macSize)
    {
        this.macSize = macSize;
        this.stateSize = stateSize;
        this.parameters = parameters;
    }

    public int getMacSize()
    {
        return macSize;
    }

    public int getStateSize()
    {
        return stateSize;
    }

    public CipherParameters getParameters()
    {
        return parameters;
    }

}
