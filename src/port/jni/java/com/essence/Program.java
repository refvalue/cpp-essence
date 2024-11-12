package com.essence;

public final class Program {
    public static void main(String[] args) {
        // HEX 编解码（不带分隔符）
        var v1 = CryptoUtil.hexEncode(new byte[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 });
        var v2 = CryptoUtil.hexDecode(v1);

        // HEX 编解码（带分隔符）
        var v3 = CryptoUtil.hexEncode(new byte[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }, (byte) ':');
        var v4 = CryptoUtil.hexDecode(v1, (byte) ':');

        System.out.println(v1);
        printBytes(v2);
        System.out.println(v3);
        printBytes(v4);

        // Base64 编解码
        var v5 = CryptoUtil.base64Encode(new byte[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 });
        var v6 = CryptoUtil.base64Decode(v5);

        System.out.println(v5);
        printBytes(v6);

        // HMAC 摘要算法
        var v7 = CryptoUtil.hmacHash(DigestMode.SHA1, "0123456789ABCDEF", "Hello");
        var v8 = CryptoUtil.hmacHash(DigestMode.SHA1, "0123456789ABCDEF", new byte[] { 'H', 'e', 'l', 'l', 'o' });

        System.out.println(v7);
        System.out.println(v8);
        System.out.println(v7.equals(v8));

        // 简单摘要算法（含 SM3）
        var v9 = CryptoUtil.makeDigest(DigestMode.SM3, "Hello");
        var v10 = CryptoUtil.makeDigest(DigestMode.SM3, new byte[] { 'H', 'e', 'l', 'l', 'o' });

        System.out.println(v9);
        System.out.println(v10);
        System.out.println(v9.equals(v10));

        // 对称加解密测试（含 SM4）
        try (var encryptor = new SymmetryEncryptor("sm4-cbc", CipherPaddingMode.PKCS7, "0123456789ABCDEF",
                "0000000000000000");
                var decryptor = new SymmetryDecryptor("sm4-cbc", CipherPaddingMode.PKCS7, "0123456789ABCDEF",
                        "0000000000000000")) {
            var c1 = encryptor.asBase64("我是中国人。");
            var c2 = decryptor.stringFromBase64(c1);
            var c3 = encryptor.asBase64(new byte[] { 1, 2, 3, 4, 5 });
            var c4 = decryptor.bytesFromBase64(c3);
            
            System.out.println(c1);
            System.out.println(c2);
            System.out.println(c3);
            printBytes(c4);
        }
    }

    private static void printBytes(byte[] buffer) {
        for (var i : buffer) {
            System.out.print(i);
            System.out.print(" ");
        }

        System.out.println();
    }
}
