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

#include "user_globalization_cpp_essence_lang.hpp"

#include <array>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include <essence/abi/list.hpp>
#include <essence/abi/map.hpp>
#include <essence/abi/string.hpp>
#include <essence/abi/vector.hpp>
#include <essence/array.hpp>
#include <essence/char8_t_remediation.hpp>
#include <essence/cli/abstract/option.hpp>
#include <essence/cli/arg_parser.hpp>
#include <essence/cli/option.hpp>
#include <essence/crypto/chunk_processor.hpp>
#include <essence/crypto/digest.hpp>
#include <essence/crypto/ostream.hpp>
#include <essence/crypto/symmetric_cipher_provider.hpp>
#include <essence/crypto/symmetric_cipher_util.hpp>
#include <essence/data_view.hpp>
#include <essence/format_remediation.hpp>
#include <essence/globalization/compiler.hpp>
#include <essence/globalization/translator.hpp>
#include <essence/io/bitstream_type_judger.hpp>
#include <essence/io/compresser.hpp>
#include <essence/io/spanstream.hpp>
#include <essence/json_compat.hpp>
#include <essence/memory.hpp>
#include <essence/memory/atomic_readable_buffer.hpp>
#include <essence/memory/nonuniform_grid_buffer.hpp>
#include <essence/memory/swapping_buffer.hpp>
#include <essence/meta/detail/data_member_binding.hpp>
#include <essence/meta/detail/data_member_matching.hpp>
#include <essence/meta/enum.hpp>
#include <essence/meta/fingerprint.hpp>
#include <essence/meta/identifier.hpp>
#include <essence/meta/literal_string_util.hpp>
#include <essence/meta/runtime/boolean.hpp>
#include <essence/meta/runtime/enum.hpp>
#include <essence/meta/runtime/json_serializer.hpp>
#include <essence/meta/runtime/struct.hpp>
#include <essence/meta/struct.hpp>
#include <essence/numeric_conversion.hpp>
#include <essence/rect.hpp>
#include <essence/span.hpp>
#include <essence/tuple.hpp>
#include <essence/type_list.hpp>

#if CPP_ESSENCE_HAS_THREADS
#include <essence/delegate.hpp>
#endif
#include <essence/encoding.hpp>
#include <essence/exception.hpp>
#include <essence/zstring_view.hpp>

#if CPP_ESSENCE_HAS_NET
#include <essence/net/common_types.hpp>
#include <essence/net/download.hpp>
#include <essence/net/http_client.hpp>
#include <essence/net/ipv4_address.hpp>
#include <essence/net/net_error.hpp>
#include <essence/net/rest_api.hpp>
#ifdef ES_HAS_HTTP_SSE
#include <essence/net/sse_client.hpp>
#include <essence/net/sse_server.hpp>
#endif
#include <essence/net/uri.hpp>
#endif

#include "essence/net/ipv6_address.hpp"

#include <essence/crypto/asymmetric_key.hpp>
#include <essence/crypto/params/ec_keygen_param.hpp>
#include <essence/crypto/params/rsa_keygen_param.hpp>
#include <essence/crypto/params/sm2_keygen_param.hpp>
#include <essence/crypto/pubkey_cipher_provider.hpp>
#include <essence/managed_handle.hpp>
#include <essence/spdlog_extension.hpp>
#include <essence/type_traits.hpp>

#include <spdlog/spdlog.h>

using namespace essence;
using namespace essence::crypto;
using namespace essence::io;

namespace {
    constexpr auto value_category_test() noexcept {
        const int& clvalue = 3;
        int lvalue{100};
        auto lvalue_tuple = std::forward_as_tuple(1, 2, clvalue, std::move(lvalue), lvalue);

        return std::apply(
            overloaded{[](int&&, int&&, const int&, int&&, int&) { return 1; }, [](auto&&...) { return 2; }},
            std::move(lvalue_tuple));
    }

    struct my_error : std::runtime_error {
        using runtime_error::runtime_error;
    };

    void test_aggregate_error() {
        try {
            throw_nested_and_flatten(std::runtime_error{"Hello"},
                [] { throw_nested_and_flatten(std::runtime_error{"My Fault"}, [] { throw my_error{"Exactly"}; }); });
        } catch (const aggregate_error& ex) {
            spdlog::error(ex.what());

            if (auto my_error_ex = ex.extract<my_error>()) {
                spdlog::error(my_error_ex->what());
            }

            if (auto other_ex = ex.extract<std::invalid_argument>()) {
                spdlog::error(other_ex->what());
            }
        }

        spdlog::info(U8("Here +++++++++++++++++"));
    }

    constexpr std::array numbers{1, 2, 3, 4, 5};
    constexpr data_view numbers_view{numbers};
    constexpr auto numbers_view_size           = numbers_view.size();
    constexpr auto numbers_view_size_bytes     = numbers_view.size_bytes();
    constexpr auto numbers_view_subview        = numbers_view.subview(2, 2);
    constexpr std::span numbers_view_span      = numbers_view_subview;
    constexpr data_view numbers_view_span_view = numbers_view_span;

    void test_globalization() {
        using namespace essence;
        using misctest::lang::get_bounded_locale;

        // 切换到中文。
        misctest::lang::get_bounded_translator().set_language(U8("zh-CN"));

        // 使用扩展函数 ginfo, gwarn, gerror, gtrace 实现多语言调用。
        spdlog::ginfo(get_bounded_locale(), U8("hello world"));
        spdlog::ginfo(get_bounded_locale(), U8("file not found"));
        spdlog::ginfo(get_bounded_locale(), U8("illegal pattern"));

        // 使用扩展函数 gformat, gformat_as 实现多语言调用。
        const auto str1 = gformat(get_bounded_locale(), U8("hello world = {}"), U8("hello world"));
        const auto str2 = gformat(get_bounded_locale(), U8("file not found = {}"), U8("file not found"));
        const auto str3 = gformat(get_bounded_locale(), U8("illegal pattern = {}"), U8("illegal pattern"));

        // 采用不同 std::allocator 的 std::basic_string 支持。
        const auto abi_str1 = gformat_as<abi::string>(get_bounded_locale(), U8("hello world = {}"), U8("hello world"));
        const auto abi_str2 =
            gformat_as<abi::string>(get_bounded_locale(), U8("file not found = {}"), U8("file not found"));
        const auto abi_str3 =
            gformat_as<abi::string>(get_bounded_locale(), U8("illegal pattern = {}"), U8("illegal pattern"));

        // 这里使用普通函数输出。
        spdlog::info(str1);
        spdlog::info(str2);
        spdlog::info(str3);

        spdlog::info(abi_str1);
        spdlog::info(abi_str2);
        spdlog::info(abi_str3);
    }

    void test_load_pubkey() {
        crypto::asymmetric_key pubkey{crypto::use_public,
            as_const_byte_span(
                U8("-----BEGIN PUBLIC KEY-----\n"
                   "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAiwVxFwMdjeChihFzmon6bGQ222eTOhnKiKLHqDa+"
                   "GoJsxo3I0Cr+/JsHqq5/"
                   "BJLZnrZQ1XJCyOskFPQZ6wPMhBccdQyDXMuQet0TeLfJmnYsMDWDwERVDNoW2h8pAmnpqHL83fnOmz8Fru27mvwIg"
                   "EvsafEUnhIol1suL8WLy7rD8hc7PjgBMidNMHtOzQnSInkTgWC/EGFDgHeb04S315pgSwAi4u+l5yvJacwWdrvWI/"
                   "xn7TQmFp6ZTANN7ns2J1ExntioreY4jJ425pIRWgiYdyESkzO3Rf5VxCDQYuifhJoQBOAuZE6L63NIh9ZGmimCIP9"
                   "0rZdbLgfXmlPG4QIDAQAB"
                   "\n"
                   "-----END PUBLIC KEY-----"))};

        spdlog::info(U8("PUBKEY INFO --- TYPE: {}, NAME: {}, DESC: {}"), static_cast<std::int32_t>(pubkey.type()),
            pubkey.name(), pubkey.description());

        crypto::pubkey_cipher_provider encryptor{pubkey};
        auto local_timestamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();

        spdlog::info(U8("LOCAL TS: {}"), local_timestamp);

        auto ciphertext = encryptor.as_base64(format(U8("{}{}"), local_timestamp, U8("123456")));

        spdlog::info(U8("ENCRYPTOR: {}, PUBKEY ENCRYPTED: {}"), encryptor.encryptor(), ciphertext);
    }

    void test_threading_cipher() {
        using namespace essence::crypto;
        const auto cipher = "OpGz3jTUXpe9FBR2mAw5+ke68f1PzDB21g7ssz924u88PWyZxy58c5EvSabhna9RPoFkWznkB65/+c0GQOisdwo/"
                            "oxPZ1WvfIdgls68yWAwYYLy/fXqJrhp/CokEKLGrQRWqKp80/I3zToSHRRDFcAGv/oTyslkbl/iapXUA3b4=";

        const symmetric_cipher_provider en{make_symmetric_cipher_chunk_processor(
            "sm4-cbc", cipher_padding_mode::pkcs7, "essenceessence", "0000000000000000")};
        const symmetric_cipher_provider de{make_symmetric_cipher_chunk_processor(
            "sm4-cbc", cipher_padding_mode::pkcs7, "essenceessence", "0000000000000000", false)};

        spdlog::info(U8("解密信息：{}"), de.as_string(en.as_bytes("AAA")));
        spdlog::info(U8("解密信息 cipher：{}"), de.string_from_base64(cipher));
    }

    void test_crypto() {
        auto pair1 = crypto::generate_asymmetric_key_pair(crypto::ec_keygen_param{.curve_name{U8("secp256k1")}});
        auto pair2 = crypto::generate_asymmetric_key_pair(crypto::sm2_keygen_param{});
        auto pair3 = crypto::generate_asymmetric_key_pair(crypto::rsa_keygen_param{.key_bits = 1024});

        pair1.save_public(U8("E:/test.pub"));
        pair1.save_private(U8("E:/test_enc.pem"), U8("sm4-cbc"), "hello");
        pair1.save_private(U8("E:/test.pem"));

        pair2.save_public(U8("E:/test_sm2.pub"));
        pair2.save_private(U8("E:/test_sm2_enc.pem"), U8("sm4-cbc"), "hello");
        pair2.save_private(U8("E:/test_sm2.pem"));

        crypto::asymmetric_key priv_key3{crypto::use_private, U8("E:/test_sm2_enc.pem")};
        crypto::asymmetric_key priv_key4{
            crypto::use_private, U8("E:/test_sm2_enc.pem"), [](std::size_t max_size, bool& cancelled) {
                printf("Max size: %d\n", static_cast<std::int32_t>(max_size));
                return "hello";
            }};

        crypto::asymmetric_key pub_key5{crypto::use_public, U8("E:/test.pub")};
        crypto::asymmetric_key pub_key6{crypto::use_public, U8("E:/test_sm2.pub")};
    }

    namespace ta::tb {
        enum class test_enum { alice, justin, edwards };

        template <meta::literal_string S, typename T>
        struct dummy_str_tag {
            using type = T;
        };

        struct foo {
            enum class json_serialization { camel_case };

            int a{};
            long b{};
            double c{};
        };

        struct entity {
            enum class json_serialization { camel_case };

            struct json_omission {
                std::nullptr_t bob_index_first;
            };

            int blice_age{};
            long bob_index_first{};
            double john_make_living{};
            std::optional<std::string> str;
            std::optional<std::vector<foo>> items;
            std::map<std::string, std::optional<foo>> mapping;
        };

        template <auto V, bool Boolean = false>
        void print_funcsig() noexcept {
            spdlog::info(U8("FUNCSIG = {}"), ES_UNIFORM_FUNCSIG);
        }

        void test_meta() {
            struct foo1 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
            };
            struct foo2 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
                int m;

                enum class json_serialization { snake_case };
            };
            struct foo3 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
                int m;

                enum class json_serialization { snake_case };
            };
            struct foo4 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
                int m;

                enum class json_serialization { snake_case };
            };
            struct foo5 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
                int m;

                enum class json_serialization { snake_case };
            };
            struct foo6 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
                int m;

                enum class json_serialization { snake_case };
            };
            struct foo7 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo8 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo9 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
            };
            struct foo10 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo11 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo12 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo13 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo14 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo15 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo16 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo17 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo18 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo19 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo20 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo21 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo22 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo23 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo24 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo25 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo26 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo27 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo28 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo29 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
            };
            struct foo30 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo31 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo32 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo33 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo34 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo35 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo36 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo37 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
            };
            struct foo38 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;
            };
            struct foo39 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo40 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo41 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo42 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo43 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };
            struct foo44 {
                int a;
                int b;
                int c;
                int d;
                int e;
                int f;
                int g;
                int h;
                int i;
                int j;
                int k;
                int l;


                enum class json_serialization { snake_case };
            };

            [[maybe_unused]] auto str1 = json(foo1{}).dump(1);
#if 1
            [[maybe_unused]] auto str2 = json(foo2{}).dump(1);
            [[maybe_unused]] auto str3 = json(foo3{}).dump(1);
            [[maybe_unused]] auto str4 = json(foo4{}).dump(1);

            [[maybe_unused]] auto str5  = json(foo5{}).dump(1);
            [[maybe_unused]] auto str6  = json(foo6{}).dump(1);
            [[maybe_unused]] auto str7  = json(foo7{}).dump(1);
            [[maybe_unused]] auto str8  = json(foo8{}).dump(1);
            [[maybe_unused]] auto str9  = json(foo9{}).dump(1);
            [[maybe_unused]] auto str10 = json(foo10{}).dump(1);
            [[maybe_unused]] auto str11 = json(foo11{}).dump(1);
            [[maybe_unused]] auto str12 = json(foo12{}).dump(1);
            [[maybe_unused]] auto str13 = json(foo13{}).dump(1);
            [[maybe_unused]] auto str14 = json(foo14{}).dump(1);
            [[maybe_unused]] auto str15 = json(foo15{}).dump(1);
            [[maybe_unused]] auto str16 = json(foo16{}).dump(1);
            [[maybe_unused]] auto str17 = json(foo17{}).dump(1);
            [[maybe_unused]] auto str18 = json(foo18{}).dump(1);
            [[maybe_unused]] auto str19 = json(foo19{}).dump(1);
            [[maybe_unused]] auto str20 = json(foo20{}).dump(1);
            [[maybe_unused]] auto str21 = json(foo21{}).dump(1);
            [[maybe_unused]] auto str22 = json(foo22{}).dump(1);
            [[maybe_unused]] auto str23 = json(foo23{}).dump(1);
            [[maybe_unused]] auto str24 = json(foo24{}).dump(1);
            [[maybe_unused]] auto str25 = json(foo25{}).dump(1);
            [[maybe_unused]] auto str26 = json(foo26{}).dump(1);
            [[maybe_unused]] auto str27 = json(foo27{}).dump(1);
            [[maybe_unused]] auto str28 = json(foo28{}).dump(1);
            [[maybe_unused]] auto str29 = json(foo29{}).dump(1);
            [[maybe_unused]] auto str30 = json(foo30{}).dump(1);
            [[maybe_unused]] auto str31 = json(foo31{}).dump(1);
            [[maybe_unused]] auto str32 = json(foo32{}).dump(1);
            [[maybe_unused]] auto str33 = json(foo33{}).dump(1);
            [[maybe_unused]] auto str34 = json(foo34{}).dump(1);
            [[maybe_unused]] auto str35 = json(foo35{}).dump(1);
            [[maybe_unused]] auto str36 = json(foo36{}).dump(1);
            [[maybe_unused]] auto str37 = json(foo37{}).dump(1);
            [[maybe_unused]] auto str38 = json(foo38{}).dump(1);
            [[maybe_unused]] auto str39 = json(foo39{}).dump(1);
            [[maybe_unused]] auto str40 = json(foo40{}).dump(1);
            [[maybe_unused]] auto str41 = json(foo41{}).dump(1);
            [[maybe_unused]] auto str42 = json(foo42{}).dump(1);
            [[maybe_unused]] auto str43 = json(foo43{}).dump(1);
            [[maybe_unused]] auto str44 = json(foo44{}).dump(1);
#endif

#if 1
#if 0
            using namespace essence::meta;
            namespace d = essence::meta::detail;

            spdlog::info(get_function_name<&test_meta>());
            spdlog::info(meta::get_function_name<&test_meta, {.shortened = true}>());
            spdlog::info(get_literal_string_t<std::string>());
            spdlog::info(get_literal_string_t<std::string, {.shortened = true}>());

            foo refl{};

            spdlog::info("Enumerating data members and values of foo...");

            meta::runtime::enumerate_data_members(refl, [](const auto&... members) {
                auto handler = [&](const auto& arg) { spdlog::info("{} = {}", arg.name, arg.reference); };

                (handler(members), ...);
            });

            for (auto&& [name, value] : meta::runtime::get_enum_names<test_enum>()) {
                spdlog::info("ENUM {} : {}", name, (int) value);
            }

#endif

#if 0
            spdlog::info(
                U8("NAMING CONVENTION: {}"), meta::runtime::to_string(meta::detail::get_json_naming_convention<foo>()));

            auto edwards = meta::runtime::from_string<test_enum>(U8("edwards"));

            if (edwards) {
                spdlog::info("edwards from string : {} -> {}", (int) *edwards, meta::runtime::to_string(*edwards));
            }

            spdlog::info(U8("test_enum::justin -> {}"), meta::runtime::to_string(test_enum::justin));
#endif
            spdlog::info(U8("Custom JSON Serialization: {}"), json(entity{
                                                                       .blice_age        = 20,
                                                                       .bob_index_first  = 30,
                                                                       .john_make_living = 66.66,
                                                                       .str{std::nullopt},
                                                                       .items{std::vector<foo>{{123, 400, 600}}},
                                                                       .mapping{{U8("今日"), foo{.a = 8888}}},
                                                                   })
                                                                  .dump(4));

            json tmp_json{{U8("bliceAge"), 22}, {U8("bobIndexFirst"), 90}, {U8("johnMakeLiving"), 88.88},
                {U8("mapping"), {{U8("明日"), json(foo{.c = 6666})}}}};

            spdlog::info(U8("Custom JSON Constructed: {}"), tmp_json.dump(4));

            auto tmp_foo = tmp_json.get<entity>();

            spdlog::info(U8("Custom JSON Deserization: {}"), abi::json(json(tmp_foo)).dump(4));
#endif
        }
    } // namespace ta::tb

    void test_cli_option() {
#if 0
        enum class tex {
            fire,
            water,
            bat,
        };

        struct model {
            enum class json_serialization { camel_case };

            std::string set_text;
            bool flower{};
            std::vector<tex> set_target;
        };

        auto opt1 = cli::option<std::string>{}
                        .set_name(U8("set_text"))
                        .add_aliases(U8("t"))
                        .set_valid_values(U8("a"), U8("b"))
                        .as_abstract();

        auto opt2 = cli::option<bool>{}.set_name(U8("set_flower")).add_aliases(U8("f")).as_abstract();

        auto opt3 = cli::option<std::vector<tex>>{}.set_name(U8("set_target")).add_aliases(U8("d")).as_abstract();

        cli::arg_parser parser;
        parser.on_error([](std::string_view message) { spdlog::error(message); });

        parser.add_option(opt1);
        parser.add_option(opt2);
        parser.add_option(opt3);

        parser.parse(std::array<abi::string, 6>{
            U8("--set_text"), U8("a"), U8("-f=false"), U8("true"), U8("--set_target"), U8("fire,bat")});

        if (parser) {
            for (auto&& [key, value] : parser.cached_result()) {
                spdlog::info(U8("{} = {}"), key, value.raw_value);
            }

            if (auto m = parser.to_model<model>()) {
                spdlog::info(json(m).dump(4));
            }

            auto joint = join_with(parser.unmatched_args(), std::string_view{U8(",")});

            spdlog::info(U8("Unmatched arguments: {}"), std::string{joint.begin(), joint.end()});
        }
#endif
    }

    void test_net_ip() {
        using namespace net;
        constexpr ipv6_address address{
            {0xFC, 0x00, 0x0, 0x0, 0x13, 0x0F, 0x0, 0x0, 0x0, 0x1, 0x9, 0xC0, 0, 0, 0, 0 /*0x87, 0x6A, 0x13, 0x0B*/},
            "1"};

        spdlog::info("Parsing IPv4 address {} -> {}", "192.168.7.13",
            net::to_string(*net::from_string<ipv4_address>("192.168.7.13")));
        spdlog::info("IPv6 address to string: {}", net::to_string(address));
    }

    void test_spanstream() {
        {
            constexpr auto& str = U8("hello 我的菜！哈哈哈，这是一个测试用字符串！\n第二行表达了 "
                                     "相应的字符\n第三行，我是一个粉刷匠，粉刷本领强。");
            ispanstream stream{str};
            std::string line;

            while (std::getline(stream, line)) {
                spdlog::info(line);
            }

            stream.clear();
            stream.seekg(0, std::ios_base::beg);

            while (std::getline(stream, line)) {
                spdlog::info(line);
            }
        }
        {
            std::array<char, 100> buffer{};
            ospanstream stream{buffer};

            stream << U8("哈哈哈，尝试输出到 span 流。\n");
            stream << U8("这是限定长度：") << buffer.size() << '\n';
            stream.flush();

            spdlog::info(buffer.data());
        }
    }
} // namespace

#if CPP_ESSENCE_HAS_NET
struct dummy_test : net::dummy_body_tag {};

namespace essence::net {
    template <>
    struct rest_message_traits<dummy_test> : make_rest_api<http_method::get, U8("/test"), std::string> {};
} // namespace essence::net

#ifdef ES_HAS_HTTP_SSE
void test_sse() {
    printf("==== starting sse test.====\n");
    static constexpr std::string_view listen_uri{U8("http://127.0.0.1:6230/v1/edgebox/cpp/sse/")};
    net::sse_server sse_server{listen_uri};
    int counter{};
    sse_server.on_connection([&](net::abstract::sse_connection connection) mutable {
        printf("Counter: %d\n", ++counter);
        printf("Connection: %s, %s\n", connection.remote_address().c_str(), connection.request_uri().str().c_str());
        for (std::size_t i = 0; i < 1000; i++) {
            connection.send_message(
                net::sse_message{.data{"Hello world!"}, .last_event_id{abi::to_abi_string(std::to_string(i))}});
            if (connection.canceled()) {
                printf("canceled\n");
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
    });

    sse_server.on_error([](std::string_view message) { printf("sse server error: %s\n", message.data()); });
    // sse_server.open();

    auto client_test = [] {
        net::sse_client sse_client{listen_uri};

        sse_client.on_message([](const net::sse_message& message) {
            for (auto&& item : message.data) {
                printf("data: %s\n", item.c_str());
            }

            if (!message.last_event_id.empty()) {
                printf("id: %s\n", message.last_event_id.c_str());
            }
        });
        sse_client.on_error([](std::string_view message) { printf("sse error: %s\n", message.data()); });
        sse_client.connect(U8("/"));

        std::this_thread::sleep_for(std::chrono::seconds{6});
    };

    std::jthread t1{client_test};
    std::jthread t2{client_test};

    t1.join();
    t2.join();
    sse_server.close();
}
#endif
#endif

int main() try {
#ifdef _WIN32
    std::system(U8("chcp 65001"));
#endif

    [[maybe_unused]] constexpr rect rrrr{
        .x      = 123,
        .y      = 456,
        .width  = 789,
        .height = 100,
    };

    {
        spdlog::info("开始 ostream");

        std::ifstream stream{"D:/三体X修订增补版.txt"};
        auto processor =
            chain_chunk_processors(make_symmetric_cipher_chunk_processor(U8("sm4-cbc"), cipher_padding_mode::pkcs7,
                                       U8("0123456789ABCDEF"), U8("0123456789ABCDEF")),
                make_base64_encoder());
        ostream base64_stream{"D:/三体X修订增补版_s.txt", std::move(processor)};
        std::ranges::copy(std::istreambuf_iterator{stream}, std::istreambuf_iterator<char>{},
            std::ostreambuf_iterator{base64_stream});

        spdlog::info("停止 ostream");
    }

    {
        spdlog::info("开始 ostream -> stringstream");

        std::ifstream stream{"D:/123.txt"};
        auto ss = std::make_shared<std::stringstream>();

        {
            auto processor =
                chain_chunk_processors(make_symmetric_cipher_chunk_processor(U8("aes-128-cbc"),
                                           cipher_padding_mode::pkcs7, U8("0123456789ABCDEF"), U8("0123456789ABCDEF")),
                    make_base64_encoder());
            ostream base64_stream;
            base64_stream.exceptions(std::ios_base::badbit);
            base64_stream.open(ss, std::move(processor));

            std::ranges::copy(std::istreambuf_iterator{stream}, std::istreambuf_iterator<char>{},
                std::ostreambuf_iterator{base64_stream});
        }

        spdlog::info(ss->str());
        spdlog::info("停止 ostream -> stringstream");
    }

    test_cli_option();

#if 1
    ta::tb::test_meta();
#endif

#if 0
    test_crypto();
#endif

    test_load_pubkey();
    test_globalization();
    test_spanstream();
    test_threading_cipher();
    test_net_ip();

#ifdef ES_HAS_HTTP_SSE
    test_sse();
#endif

    spdlog::info(U8("数字 0x12345 转化为字符串：{}"), to_string(0x12345, 16));
    spdlog::info(U8("数字 13333.987654321 转化为字符串：{}"), to_string(13333.666, std::chars_format::hex, 3));
    spdlog::info(U8("字符串 0x12345 转化为数字：{}"), from_string<std::int32_t>(U8("12345"), 16).value_or(0));
    spdlog::info(U8("字符串 13333.987654321 转化为数字：{}"), from_string<double>(U8("13333.987654321")).value_or(0.0));

    using meta::fingerprint;
    constexpr fingerprint fp1{type_identity<int>{}};
    constexpr fingerprint fp2{type_identity<int>{}};
    constexpr bool eqfp12 = fp1 == fp2;

    auto close_handle  = [](int value) { printf(U8("释放句柄：%d\n"), value); };
    auto close_handle2 = [](void* value) { printf(U8("释放句柄：%p\n"), value); };
    auto close_handle3 = [](void** value) { printf(U8("释放句柄：%p\n"), value); };
    {
        shared_handle<close_handle, std::uintptr_t, int> x{123};
        shared_handle<close_handle2> y{(void*) (0xFFFFEEEE)};
        shared_handle<close_handle3> z{(void*) (0xFFFFEEEE)};

        auto h1 = x.get();
        auto h2 = y.get();
        auto h3 = z.get();
    }

    spdlog::info(format(U8("{:016}"), 12345));
    spdlog::info("===============!!!!");
    spdlog::info(make_digest(digest_mode::sha512, abi::vector<std::uint8_t>{1, 2, 3, 4, 5}));
    spdlog::info("===============!!!!");

    spdlog::info(format(U8("FILE HASH ==================")));
#ifdef _WIN32
    spdlog::info(make_file_digest(digest_mode::sha512, U8("C:/Windows/Notepad.exe")));
#else
    spdlog::info(make_file_digest(digest_mode::sha512, U8("/usr/bin/tar")));
#endif
    spdlog::info(format(U8("================== END FILE HASH")));

    std::cout << "Here\n";
    spdlog::info(std::to_string((unsigned long long) stdout));

    auto enc1 = to_utf8_string(to_utf16_string(U8("我是测试。")));

    spdlog::info(enc1);

#if CPP_ESSENCE_HAS_NET
    if (false) {
        net::http_client tmp{"http://123.com/"};
        tmp.commit_message(dummy_test{});
    }

    constexpr bool check1 = net::rest_message<dummy_test>;
    static_assert(check1);
#endif

    constexpr auto category_test = value_category_test();

#if CPP_ESSENCE_HAS_THREADS
    delegate<int(int, int)> del;

    del += [](int a, int b) { return a + b; };
    del(1, 2);
#endif

    std::locale::global(std::locale::classic());

    spdlog::info("================ CIPHER START");
    for (auto&& item : get_all_symmetric_cipher_names()) {
        spdlog::info(item);
    }
    spdlog::info("================ CIPHER END");

#if 0
    test_aggregate_error();
#endif

    std::vector<std::string> strs;

    spdlog::info(U8("ZSTRING_VIEW TEST: {}\n"), zstring_view{U8("你好啊。")});
    spdlog::info("=============== HEX TEST");

    auto hex   = hex_encode(U8("我是一个中国人。"));
    auto unhex = hex_decode(hex);
    std::string unhex_str{(char*) unhex.data(), (char*) unhex.data() + unhex.size()};

    spdlog::info(hex);
    spdlog::info(unhex_str);

    constexpr zstring_view aes_algorithm{U8("aes-256-cbc")};
    constexpr zstring_view aes_key{U8("748ac937045c4569af16b922868964c5")};
    constexpr zstring_view aes_iv{U8("dbda9b4915c6495b")};

    symmetric_cipher_provider encryptor{
        make_symmetric_cipher_chunk_processor(aes_algorithm, cipher_padding_mode::pkcs7, aes_key, aes_iv)};
    symmetric_cipher_provider decryptor{
        make_symmetric_cipher_chunk_processor(aes_algorithm, cipher_padding_mode::pkcs7, aes_key, aes_iv, false)};

    auto ciphertext = encryptor.as_base64(U8("Hello world!"));
    auto plaintext  = decryptor.string_from_base64(ciphertext);

    spdlog::info(ciphertext);
    spdlog::info(plaintext);

    spdlog::info(U8("Starting test compression ======="));

    compresser compresser_test{compression_mode::zstd};

    std::vector<std::byte> compress_test;

    for (std::size_t i = 0; i < 12; i++) {
        compress_test.emplace_back(static_cast<std::byte>(i));
    }

    auto compressed   = compresser_test.as_bytes(compress_test, 3);
    auto decompressed = compresser_test.inverse_as_bytes(compressed);

    spdlog::info(U8("Compressed ======="));

    for (auto&& item : compressed) {
        printf("%d\n", static_cast<int>(item));
    }

    spdlog::info(U8("Decompressed ======="));

    for (auto&& item : decompressed) {
        printf("%d\n", static_cast<int>(item));
    }

#if CPP_ESSENCE_HAS_NET
    net::http_client client{U8("https://www.baidu.com/"),
        net::http_client_config{.validate_certificates = false, .https_to_http_redirects = true}};
    net::http_client_config default_config;

    auto force_verify = default_config.validate_certificates;

    // https://obs-r.facebeacon.com/obs-ctcc/person/eaffd935-347b-4e36-99b6-5a77963cd985.jpg
    {
        auto image_data =
            essence::net::download_file(U8("http://img.ahwang.cn/a/10001/202203/4a9a23802e5e9f8ed9de139001e9db1a.jpeg"),
                net::http_progress_handlers{.on_percentage = [](net::message_direction direction, float percentage) {
                    printf("Direction: %d, Percentage: %f\n", static_cast<std::int32_t>(direction), percentage);
                }});
        std::ofstream stream{U8("F:/delivered.jpg"), std::ios::binary};

        stream.write(reinterpret_cast<const char*>(image_data.data()), image_data.size());
    }

    net::uri uria{"http://www.aa.com/a/b/c/d/?1=2&3=4&5=6"};

    auto uria_path  = uria.split_path();
    auto uria_query = uria.split_query();

    spdlog::info(net::uri{U8("https://www.baidu.com/df/zzj/ghfgdfg/?a=33")}.str());
    spdlog::info(U8("{}"), net::uri{U8("https://www.baidu.com/df/zzj/ghfgdfg/?a=33")}
                               == net::uri{U8("https://www.baidu.com/df/zzj/ghfgdfg/?a=33")});
    net::uri x{"http://www.baidu.com/a"};
    net::uri y{x};
    net::uri z{"/bbb"};

    auto md5_1 = md5_hash("123");

    spdlog::info("=========== md5 test");
    spdlog::info(md5_1);
    spdlog::info(std::to_string(md5_1.size()));
    spdlog::info(z.str());
    spdlog::info(x.resolve_uri(z.str()));
    spdlog::info("123: " + md5_hash("123"));

    auto result = client.commit_json(net::http_method::post, U8("/"), abi::json{});

    spdlog::info(result.dump(4));
#endif
} catch (const aggregate_error& ex) {
    spdlog::info("==== ENTER !!!====");
    spdlog::error(ex.what());

#if CPP_ESSENCE_HAS_NET
    if (auto ex1 = ex.extract<net::net_error>()) {
        spdlog::info("net_error detected!");
        spdlog::info(ex1->what());
    }
#endif
} catch (const std::exception& ex) {
    spdlog::error(ex.what());
}
