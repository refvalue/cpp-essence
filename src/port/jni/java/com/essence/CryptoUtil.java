package com.essence;

public final class CryptoUtil {
    static {
        System.loadLibrary("cpp-essence-jni");
    }

    public native static String hexEncode(byte[] buffer);

    public native static byte[] hexDecode(String hex);

    public native static String hexEncode(byte[] buffer, byte delimiter);

    public native static byte[] hexDecode(String hex, byte delimiter);

    public native static String base64Encode(byte[] buffer);

    public native static byte[] base64Decode(String base64);

    public static String hmacHash(DigestMode mode, String key, byte[] buffer) {
        return hmacHash(mode.ordinal(), key, buffer);
    }

    public static String hmacHash(DigestMode mode, String key, String buffer) {
        return hmacHash(mode.ordinal(), key, buffer);
    }

    public static String makeDigest(DigestMode mode, byte[] buffer) {
        return makeDigest(mode.ordinal(), buffer);
    }

    public static String makeDigest(DigestMode mode, String buffer) {
        return makeDigest(mode.ordinal(), buffer);
    }

    private CryptoUtil() {
    }

    private native static String hmacHash(int mode, String key, byte[] buffer);

    private native static String hmacHash(int mode, String key, String buffer);

    private native static String makeDigest(int mode, byte[] buffer);

    private native static String makeDigest(int mode, String buffer);

}
