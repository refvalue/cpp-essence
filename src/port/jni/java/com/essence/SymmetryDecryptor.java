package com.essence;

public class SymmetryDecryptor implements CipherMetadata, AutoCloseable {
    private long impl;
    
    static {
        System.loadLibrary("cpp-essence-jni");
    }

    public SymmetryDecryptor(String cipherName, CipherPaddingMode paddingMode, byte[] key, byte[] iv) {
        init(cipherName, paddingMode.ordinal(), key, iv);
    }

    public SymmetryDecryptor(String cipherName, CipherPaddingMode paddingMode, String key, String iv) {
        init(cipherName, paddingMode.ordinal(), key, iv);
    }

    @Override
    public native String getCipherName();

    public native byte[] asBytes(byte[] buffer);

    public native String asString(byte[] buffer);

    public native byte[] bytesFromBase64(String base64);

    public native String stringFromBase64(String base64);

    @Override
    public void close() {
        release();
    }

    private native void init(String cipherName, int paddingMode, byte[] key, byte[] iv);

    private native void init(String cipherName, int paddingMode, String key, String iv);

    private native void release();
}
