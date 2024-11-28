/*
 * Copyright (c) 2024 The RefValue Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <array>
#include <ranges>
#include <streambuf>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <essence/abi/vector.hpp>
#include <essence/char8_t_remediation.hpp>
#include <essence/crypto/chunk_processor.hpp>
#include <essence/crypto/digest.hpp>
#include <essence/crypto/file_validation.hpp>
#include <essence/crypto/ostream.hpp>
#include <essence/crypto/symmetric_cipher_provider.hpp>
#include <essence/format_remediation.hpp>
#include <essence/io/fs_operator.hpp>
#include <essence/io/spanstream.hpp>
#include <essence/meta/runtime/enum.hpp>
#include <essence/zstring_view.hpp>

#include <gtest/gtest.h>

using namespace essence;
using namespace essence::io;
using namespace essence::crypto;

#define MAKE_TEST(name) TEST(crypto_test, name)

MAKE_TEST(file_validation) {
    static constexpr std::string_view str{U8("Hello world!")};
    const auto file_name = format(U8("{}.txt"), test_info_->name());

    { get_native_fs_operator().open_write(file_name)->write(str.data(), str.size()); }

    auto pairs       = meta::runtime::get_enum_names<digest_mode>();
    auto valid_modes = pairs | std::views::values | std::views::filter([](const auto& inner) {
        return inner != digest_mode::shake128 && inner != digest_mode::shake256;
    });

    for (auto&& item : valid_modes) {
        make_validation_file(item, file_name);
        ASSERT_TRUE(validate_file(item, file_name));
    }
}

MAKE_TEST(digest) {
    static constexpr std::string_view str{U8("Hello world!")};
    static constexpr std::array digest_cases{
        std::pair{digest_mode::sm3, U8("0E4EBFDE39B5789B457B3D9ED2D38057CEED47BE5D9728A88287AD51F5C1C3D2")},
        std::pair{digest_mode::md5, U8("86FB269D190D2C85F6E0468CECA42A20")},
        std::pair{digest_mode::sha1, U8("D3486AE9136E7856BC42212385EA797094475802")},
        std::pair{digest_mode::sha224, U8("7E81EBE9E604A0C97FEF0E4CFE71F9BA0ECBA13332BDE953AD1C66E4")},
        std::pair{digest_mode::sha256, U8("C0535E4BE2B79FFD93291305436BF889314E4A3FAEC05ECFFCBB7DF31AD9E51A")},
        std::pair{digest_mode::sha384,
            U8("86255FA2C36E4B30969EAE17DC34C772CBEBDFC58B58403900BE87614EB1A34B8780263F255EB5E65CA9BBB8641CCCFE")},
        std::pair{digest_mode::sha512, U8("F6CDE2A0F819314CDDE55FC227D8D7DAE3D28CC556222A0A8AD66D91CCAD4AAD6094F517A218"
                                          "2360C9AACF6A3DC323162CB6FD8CDFFEDB0FE038F55E85FFB5B6")},
        std::pair{digest_mode::sha512_224, U8("B48C4994A3D2B6B48AE7FA6FCC09F33DC0C985109C0B7493FD3C74D0")},
        std::pair{digest_mode::sha512_256, U8("F8162AD49196C1C12BDDBCFF1D362DDACF03AE246B6A7864B75C244B965FE475")},
        std::pair{digest_mode::sha3_224, U8("D3EE9B1BA1990FECFD794D2F30E0207AAA7BE5D37D463073096D86F8")},
        std::pair{digest_mode::sha3_256, U8("D6EA8F9A1F22E1298E5A9506BD066F23CC56001F5D36582344A628649DF53AE8")},
        std::pair{digest_mode::sha3_384,
            U8("F9210511D0B2862BDCB672DAA3F6A4284576CCB24D5B293B366B39C24C41A6918464035EC4466B12E22056BF559C7A49")},
        std::pair{digest_mode::sha3_512, U8("95DECC72F0A50AE4D9D5378E1B2252587CFC71977E43292C8F1B84648248509F1BC18BC6F0"
                                            "B0D0B8606A643EFF61D611AE84E6FBD4A2683165706BD6FD48B334")},
    };

    ASSERT_STREQ(md5_hash(str).c_str(), digest_cases[1].second);

    for (auto&& [mode, hash] : digest_cases) {
        ASSERT_STREQ(make_digest(mode, str).c_str(), hash);
    }

    static constexpr std::string_view hmac_key{U8("123456")};
    static constexpr std::array hmac_cases{
        std::pair{digest_mode::sm3, U8("oSa6DkCGgM1RVYdAdBtPzTXJnDy9go/H0JynXx1N7i8=")},
        std::pair{digest_mode::md5, U8("qSYgzjx1zYRx6md4P7oiFA==")},
        std::pair{digest_mode::sha1, U8("sbtF0ndTgjZikFHXMSX0U95RoeM=")},
        std::pair{digest_mode::sha224, U8("iGTtVE4ZZkY5jRu1wdQeVPWBtnlgmBCADR46dg==")},
        std::pair{digest_mode::sha256, U8("f0jADrIur8rVdJ/yFztA8d3uil9gOJKK69hbCCE3H8Y=")},
        std::pair{digest_mode::sha384, U8("ziks7H1KBv1gJigsWggHXM4PBh4LjLabKrJDxSEeQmvu9JpunVXMhfNgvsPFlsOL")},
        std::pair{digest_mode::sha512,
            U8("k5ucMeh8B5vj/INDpYVBEaZMsBlhhZLJmzFnzhdIviNTXaqtBwjsPTg/cc4ZPXlyxHL75eU6NoeXxlBLjx1uPA==")},
        std::pair{digest_mode::sha3_224, U8("GvN8xwWUI53BgN8Zz6fDp1ccOnR4CCPby6l1Ww==")},
        std::pair{digest_mode::sha3_256, U8("1O8w4E84LFHgsut5YdmPeCZOHirQVemnFLpNcnPBr9k=")},
        std::pair{digest_mode::sha3_384, U8("tytwzoHr+eLNO/niMsYZR845epPj58DizKbyUUFUq7ALMG3fCJBomcJx0JKNFJ8z")},
        std::pair{digest_mode::sha3_512,
            U8("cyTBn/T3NYPc2hfo16fCwdGW7FRw2zcXlynANn66lPBWAxtE1FrLhBg/ibtBq4jIw1iQdjJGGNnX23RDMhJ+Zw==")},
    };

    for (auto&& [mode, hash] : hmac_cases) {
        ASSERT_STREQ(hmac_hash(mode, hmac_key, str).c_str(), hash);
    }
}

MAKE_TEST(base64) {
    static const essence::abi::vector<std::byte> binary{std::byte{0}, std::byte{1}, std::byte{2}, std::byte{3}};
    static constexpr zstring_view str{U8("Something like that!!!")};
    static constexpr zstring_view base64_binary{U8("AAECAw==")};
    static constexpr zstring_view base64_str{U8("U29tZXRoaW5nIGxpa2UgdGhhdCEhIQ==")};

    ASSERT_STREQ(base64_encode(binary).c_str(), base64_binary.c_str());
    ASSERT_STREQ(base64_encode(str).c_str(), base64_str.c_str());
    ASSERT_EQ(base64_decode(base64_binary), binary);
    ASSERT_STREQ(base64_decode_as_string(base64_str).c_str(), str.c_str());
}

MAKE_TEST(hex) {
    static const essence::abi::vector<std::byte> binary{std::byte{0}, std::byte{1}, std::byte{2}, std::byte{3}};
    static constexpr zstring_view str{U8("Something like that!!!")};
    static constexpr zstring_view hex_binary{U8("00010203")};
    static constexpr zstring_view hex_str{U8("536F6D657468696E67206C696B652074686174212121")};

    ASSERT_STREQ(hex_encode(binary).c_str(), hex_binary.c_str());
    ASSERT_STREQ(hex_encode(str).c_str(), hex_str.c_str());
    ASSERT_EQ(hex_decode(hex_binary), binary);
    ASSERT_STREQ(hex_decode_as_string(hex_str).c_str(), str.c_str());
}

MAKE_TEST(symmetric_cipher) {
    static constexpr std::array cases{
        std::array{U8("aes-128-cbc"), U8("Hello world!"), U8("Ym3Ssw7VEm0kzw9ObL+Mmw==")},
        std::array{U8("sm4-cbc"), U8("Hello world!"), U8("5RATMZ5UMyBf9WU1fM3YaA==")},
    };

    static constexpr std::string_view key{U8("0123456789ABCDEF")};
    static constexpr std::string_view iv{U8("ABCDEFGHIJKLMNOP")};

    for (auto&& [name, plaintext, ciphertext] : cases) {
        const symmetric_cipher_provider encryptor{
            make_symmetric_cipher_chunk_processor(name, cipher_padding_mode::pkcs7, key, iv)};

        const symmetric_cipher_provider decryptor{
            make_symmetric_cipher_chunk_processor(name, cipher_padding_mode::pkcs7, key, iv, false)};

        ASSERT_STREQ(encryptor.as_base64(std::string_view{plaintext}).c_str(), ciphertext);
        ASSERT_STREQ(decryptor.string_from_base64(std::string_view{ciphertext}).c_str(), plaintext);
    }
}

MAKE_TEST(symmetric_cipher_chunked) {
    static constexpr zstring_view name{U8("aes-128-cbc")};
    static constexpr std::string_view key{U8("0123456789ABCDEF")};
    static constexpr std::string_view iv{U8("ABCDEFGHIJKLMNOP")};

    static constexpr zstring_view str{
        U8("In a distant corner of the universe, a small planet named Zephyria harbored an ancient secret buried "
           "beneath its vast oceans. The inhabitants, known as the Zephyrians, had evolved uniquely with "
           "bioluminescent skin that glowed under the moonlight, casting a serene blue hue across their villages. They "
           "lived in harmony with nature, drawing power from the vibrant flora that dotted their landscapes.")};

    const auto file_name = format(U8("{}.txt"), test_info_->name());
    {
        ispanstream input_stream{str};
        ostream encryption_stream{file_name,
            chain_chunk_processors(make_symmetric_cipher_chunk_processor(name, cipher_padding_mode::pkcs7, key, iv),
                make_base64_encoder())};

        std::ranges::copy(std::istreambuf_iterator{input_stream}, std::istreambuf_iterator<char>{},
            std::ostreambuf_iterator{encryption_stream});
    }

    std::string buffer(str.size(), U8('\0'));

    {
        const auto input_stream = get_native_fs_operator().open_read(file_name);
        ostream decryption_stream{std::make_shared<ospanstream>(buffer),
            chain_chunk_processors(make_base64_decoder(),
                make_symmetric_cipher_chunk_processor(name, cipher_padding_mode::pkcs7, key, iv, false))};

        std::ranges::copy(std::istreambuf_iterator{*input_stream}, std::istreambuf_iterator<char>{},
            std::ostreambuf_iterator{decryption_stream});
    }

    EXPECT_STREQ(str.c_str(), buffer.c_str());
}

MAKE_TEST(pubkey_cipher) {

}
