package com.essence;

public class SymmetryEncryptor implements CipherMetadata, AutoCloseable {
    private long impl;

    static {
        System.loadLibrary("cpp-essence-jni");
    }

    public SymmetryEncryptor(String cipherName, CipherPaddingMode paddingMode, byte[] key, byte[] iv) {
        init(cipherName, paddingMode.ordinal(), key, iv);
    }

    public SymmetryEncryptor(String cipherName, CipherPaddingMode paddingMode, String key, String iv) {
        init(cipherName, paddingMode.ordinal(), key, iv);
    }

    @Override
    public native String getCipherName();

    public native byte[] asBytes(byte[] buffer);

    public native byte[] asBytes(String buffer);

    public native String asBase64(byte[] buffer);

    public native String asBase64(String buffer);

    @Override
    public void close() {
        release();
    }

    private native void init(String cipherName, int paddingMode, byte[] key, byte[] iv);

    private native void init(String cipherName, int paddingMode, String key, String iv);

    private native void release();
}
