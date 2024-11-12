# 😀CPP Essence -- Modern C++ Utility Library

[TOC]

## 📚Requirements

- MSVC >= 14.30 (Visual Studio 2022 17.0)
- GCC >= 11.2
- Clang >= 16.0
- Emscripten >= 3.1.52
- Host GLIBC >= 2.34 (when compiling on Linux)
- C++ 20 Support
- JDK >= 8 (Optional if `ES_WITH_JNI` = `OFF`)



## 💻Importing the Prebuilt Package with CMake

### Step-by-step Configuration

To use this library on your target platform immediately, you can download the prebuilt packages from the [Release](http://www.baidu.com) page. After obtaining the compressed files, such as `cpp-essence-1.0-windows-x86_64-release.zip` and `cpp-essence-1.0-linux-x86_64.tgz`, you can decompress them to any location on your disk.

```powershell
Expand-Archive -Path './cpp-essence-1.0-windows-x86_64-release.zip' -DestinationPath 'C:\path\to\extract\folder'
```

```bash
tar -zxf ./cpp-essence-1.0-linux-x86_64.tgz
```

Then you need to modify the `CMakeUserPresets.json` file in your own project to add a CMake variable which stores the path of the directory that the file was decompressed to. Here is an example:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "windows-debug-user",
      "inherits": "windows-debug",
      "cacheVariables": {
        "ES_CPP_ESSENCE_ROOT": "E:/Libs/cpp-essence-1.0-windows-x86_64-debug"
      }
    },
    {
      "name": "windows-release-user",
      "inherits": "windows-release",
      "cacheVariables": {
        "ES_CPP_ESSENCE_ROOT": "E:/Libs/cpp-essence-1.0-windows-x86_64-release"
      }
    },
    {
      "name": "linux-release-user",
      "inherits": "linux-release",
      "cacheVariables": {
        "ES_CPP_ESSENCE_ROOT": "/opt/cpp-essence-1.0-linux-x86_64"
      }
    }
  ]
}
```

Then you can add the following code to your CMake script file, such as `Dependencies.cmake` for dependency management:

```cmake
find_package(CppEssence REQUIRED HINTS ${ES_CPP_ESSENCE_ROOT})
```

The final step is to link the found package to your target as demonstrated in the example below:

```cmake
target_link_libraries(your_target_name PRIVATE CppEssence::cpp-essence) # The mandatory package.
target_link_libraries(your_target_name PRIVATE CppEssenceJniSupport::cpp-essence-jni-support) # Optional JNI utilities.
```



### Release or Debug Version

Due to the different ABIs between the Release and Debug versions of Microsoft's STL, Windows DLLs that export any STL symbols cannot be mixed. Forcing this will result in severe issues, such as program crashes or unstable running states. To avoid this problem, it is highly recommended to ensure that your DLL has the same build type as this library.

There is no such issue on Linux/Unix platform, so only the Release version is provided on the [Release](http://www.baidu.com) page.



## 💡Main Features

### 1. Reflection Tool Library
- **100% Macro-Free Design**: Implemented using native C++ with no reliance on macros.
- **Data Member Traversal**: Supports direct traversal of class data members, allowing easy access to all member variables.
- **Member Name Retrieval**: Automatically retrieves the names of class members.
- **Enum Support**: Provides automatic enum-to-string conversion and the ability to read enum ranges.
- **JSON Serialization**: Supports automatic serialization and deserialization of class objects to/from JSON.

### 2. Command Line Tool Library
- **GNU/Linux Bash Compliance**: Supports multiple command-line parameter formats, including:
  - `--name=value`, `-n=value`, `--name value`, `-n value`, `--switch`, `-s`
  - `--values 1,2,3,4,5`, `-v 1,2,3,4,5`, `--values alice,bob,paul`, `-v alice,bob,paul`
  - `-abcde` and many more combinations.
- **Array & Enum Parsing**: Supports parsing of array and enum values, with automatic conversion to data models using reflection.

### 3. Internationalization Support
- **Multi-language Mapping**: Allows the creation of multi-language mapping files, which can be compiled into a compact binary format.
- **Dynamic Language Switching**: Supports runtime switching of languages and fetching of language-specific texts.
- **Independent Language Packs**: Supports independent language packs for modular localization without interference.

### 4. Multicast Delegate System
- **Multicast Delegate**: Implements a multi-subscription delegate system similar to C#'s `MulticastDelegate`, enabling thread-safe event triggering and multiple listeners.

### 5. I/O Basic Support Library
- **C++23 `spanstream` Support**: Implements the C++23 `spanstream` pattern for efficient stream processing.
- **File Type Detection**: Supports quick identification of common image types based on file headers.
- **Abstract File System Operations**: Provides a unified interface for file system operations across platforms.
- **Embedded Resources**: Supports embedded resources with cross-platform compatibility, based on CMRC.

### 6. Networking Library
- **RESTful HTTP Communication**: Supports REST-style HTTP communication.
- **Server-Side Events**: Implements Server-Side Events (SSE) for real-time server-client communication.
- **File Download with Progress**: Supports downloading files with progress tracking.

### 7. JSON Library
- **nlohmann/json Integration**: Built on the popular `nlohmann json` library, with support for automatic member reflection.
- **Flexible Naming Conventions**: Supports automatic conversion between `Camel`, `Pascal`, and `Snake` case styles.

### 8. Cryptography Library
- **Symmetric & Asymmetric Encryption**: Supports common cryptographic algorithms, including AES, SM2, SM4, RSA, and elliptic curve cryptography.
- **Block-Level Encryption**: Provides support for block encryption and decryption for large data.

### 9. Logging System
- **spdlog Integration**: Built on the popular `spdlog` library, with support for logging, log dumping, and log rotation.
- **Internationalization Support**: Allows logging messages in multiple languages.

### 10. Data Hashing
- **Multiple Hashing Algorithms**: Supports SHA1, SHA2, SHA3, MD5, SM3, and more.
- **Large File Checksums**: Supports checksum calculation for large files with chunk-based hashing.

### 11. JNI Support Library(Optional)
- **Automatic Data Mapping**: Provides automatic mapping of C++ containers and native types to JNI types.
- **Custom Java Class Conversion**: Supports custom data conversion rules for Java class types.
- **Managed Object Lifecycle**: Allows control over the lifecycle of managed objects in JNI.

### 12. Functional Programming Library
- **Convenient Templates**: Provides functional programming utilities, including function signature reflection and easy-to-use function objects.
- **Concepts Support**: Includes helpful concepts for functional programming in C++.

### 13. Native API Programming Library
- **Handle Management**: Provides RTTI-based management of raw and managed API handles.
- **Custom System Handle Management**: Allows user-defined management of native API calls and handle lifecycle to avoid memory leaks.

### 14. Hash Utilities
- **Unified Hashing Methods**: Offers unified methods for combining hash values.
- **Case-Insensitive String Comparisons**: Supports case-insensitive string comparisons and dictionary-based string lookups.

### 15. Formatting Library
- **fmt Library Support**: Built on the popular `fmt` library, offering flexible string formatting options.
- **C++20 `format`**: Dynamically wraps the `std::format` library depending on the user's environment.

### 16. String Library
- **Zero-Terminated Strings**: Supports `zstring_view`, a string view type for efficiently passing C-style null-terminated strings.
- **C-Style String Support**: Optimized for interop with lower-level C APIs.

### 17. Ranges Adaptation Library
- **C++20 Range Extensions**: Extends C++20 `Ranges` with additional utilities like `join_with` for advanced use cases.
- **Comprehensive Functionality**: Enhances the business logic with extended range-based operations.

### 18. ABI Compatibility Library
- **ABI Compatibility**: Facilitates ABI-compatible programming for common types like `vector`, `map`, `list`, `string`, etc.
- **JSON Support**: Adds seamless integration for JSON data manipulation.

### 19. Graphics Support Library
- **Color Space Support**: Supports ARGB and YUV color spaces, with over a hundred built-in color values.
- **Automatic Type Conversion**: Supports automatic conversion between different graphics types.

### 20. Character Encoding Library
- **UTF-8, UTF-16, and Native Encoding**: Supports conversion between UTF-8, UTF-16, and native encodings (`char` or `wchar_t`).
- **Comprehensive String Operations**: Provides a wide range of string handling functions.

### 21. Numeric Conversion Library
- **String Conversion**: Provides convenient `from_string` and `to_string` utilities for seamless conversion between various data types and strings.

### 22. Compression Library
- **ZSTD, ZIP, and GZIP**: Supports ZSTD, ZIP, and GZIP compression algorithms for efficient data compression and decompression.



## 🙋Quick Samples

### 👉[P1423R3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1423r3.html) char8_t Backward Compatibility Remediation

#### Compatibility between `char8_t` and `char`

In C++20, the introduction of the new built-in character type `char8_t` and the `std::u8string` (which is essentially `std::basic_string<char8_t>`) was meant to provide a better and more standardized way to handle UTF-8 encoded characters. However, the decision to **ban implicit conversions between `char8_t` and `char`** has introduced some challenges for developers, especially when migrating legacy code that was designed around `std::string` (which uses `char`) and `std::string_view`.

```c++
#include <string>
#include <string_view>

std::u8string u8str = u8"UTF-8 text";
std::u8string_view u8strv = u8str;
std::string str = u8str; // Ill-formed.
std::string_view strv = u8str; // Ill-formed.
std::string_view strv2 = u8strv; // Ill-formed.
```

#### Proposal P1423R3

The proposal **P1423R3** introduces a methodology to handle the compatibility problem caused by the introduction of `char8_t` in C++20, particularly regarding the migration of legacy code. It suggests that `char` and `std::string` should be used universally where possible, avoiding the direct use of `char8_t` and `std::u8string` in most cases. This approach aims to ensure that existing libraries and interfaces that rely on `std::string` can continue to function without modification or the need for explicit casting between `char` and `char8_t`.

The macro `U8` represents a UTF-8 encoded string literal within this implementation of **P1423R3**. Here’s a simple code block demonstrating how this compatibility could be managed:

```c++
#include <array>
#include <string>
#include <string_view>

#include <essence/char8_t_remediation.hpp>

constexpr char u8char = U8('A');

constexpr auto char_array = U8("This is UTF-8 encoded."); // const char[N]
constexpr std::array char_std_array = U8("This is UTF-8 encoded."); // std::array<char, N>
const std::string str = U8("Here's some text in UTF-8."); // Well-formed.
const std::string_view strv = str; // Well-formed.

constexpr auto raw_literal = U8(R"(C:\Windows\System32)");

// Interop between std::string and std::u8string.
const std::u8string u8str = essence::to_u8string(strv);
const std::string strconv = essence::from_u8string(u8str);
```

#### Portable approach to construct `std::filesystem::path` with UTF-8

The `std::filesystem::path` introduced by the C++17 Standard defines one constructor which has a signature as follows:

```c++
template<typename Source>
std::filesystem::path::path(const Source& source, std::filesystem::path::format fmt = std::filesystem::path::auto_format);
```

Any of the character types `char`, `char8_t`, `char16_t`, `char32_t`, `wchar_t` is allowed for the type parameter `Source`. To designate a UTF-8 encoded string precisely, you can call `essence::to_u8string` to convert your `std::string` to `std::u8string` to enable the `char8_t` overload.

```c++
#include <filesystem>

#include <essence/char8_t_remediation.hpp>

const std::filesystem::path path{essence::to_u8string(U8("/usr/local/etc"))}; // Ensures UTF-8.
```



### 👉Globalization with Dynamic Language Switching

To enable multi-language support, you can create the directory structure and files below.

1. **Create the `lang` directory**: This will be the main directory that holds language-specific JSON files.
2. **Create `en-US.json` and `zh-CN.json`**: These files will contain the language-specific text mappings.
3. **Create `CMakeLists.txt`**: This file will include instructions for building the project, especially if you want to handle the language files in the build process.

Here is how the directory structure should look:

```
|-- lang/
|   |-- en-US.json
|   |-- zh-CN.json
|   |-- CMakeLists.txt
```

The  `en-US.json` contains keyword mappings written in American English. 

```json
{
    "hello world": "hello world",
    "file not found": "The file was not found.",
    "illegal pattern": "The regex pattern was illegal."
}
```

The `zh-CN.json` contains keyword mapping written in Chinese Mandarin.

```json
{
    "hello world": "你好世界",
    "file not found": "文件未找到。",
    "illegal pattern": "正则表达式规则无效。"
}
```

The `CMakeLists.txt` describes the language resources.

```cmake
include(ESLangCompiler)

es_add_lang_resources(
    my-test # The target that the language resources will be attached to.
    NAME my_test # The suffix of the automatically generated header file：user_globalization_my_test.hpp。
    ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} # The root directory of the language JSON files.
    NAMESPACE essence::misctest::lang # The namespace of the generated header file.
)
```

To enable globalization integration, you need to rebuild the CMake cache and then the generated header will be available.

```c++
#include "user_globalization_my_test.hpp"

#include <essence/abi/string.hpp>
#include <essence/char8_t_remediation.hpp>
#include <essence/format_remediation.hpp>
#include <essence/spdlog_extension.hpp>

int main() {
    using namespace essence;
    using misctest::lang::get_bounded_locale;
    
    // Switches to zh-CN.
    misctest::lang::get_bounded_translator().set_language(U8("zh-CN"));
    
    // Examples of spdlog::ginfo, spdlog::gwarn, spdlog::gerror, spdlog::gtrace
    // to support globalized text logging.
    spdlog::ginfo(get_bounded_locale(), U8("hello world"));
    spdlog::ginfo(get_bounded_locale(), U8("file not found"));
    spdlog::ginfo(get_bounded_locale(), U8("illegal pattern"));
    
    // Uses gformat, gformat_as to support globalized text formatting.
    auto str1 = gformat(get_bounded_locale(), U8("hello world = {}"), U8("hello world"));
    auto str2 = gformat(get_bounded_locale(), U8("file not found = {}"), U8("file not found"));
    auto str3 = gformat(get_bounded_locale(), U8("illegal pattern = {}"), U8("illegal pattern"));
    
    // Uses custom std::basic_string<> as a return value.
    // Here abi::string is an ABI-compatible string, referring to the ABI-compatible types.
    auto abi_str1 = gformat_as<abi::string>(get_bounded_locale(), U8("hello world = {}"), U8("hello world"));
    auto abi_str2 = gformat_as<abi::string>(get_bounded_locale(), U8("file not found = {}"), U8("file not found"));
    auto abi_str3 = gformat_as<abi::string>(get_bounded_locale(), U8("illegal pattern = {}"), U8("illegal pattern"));
    
    // Ordinary logging outputs.
    spdlog::info(str1);
    spdlog::info(str2);
    spdlog::info(str3);
    
    spdlog::info(abi_str1);
    spdlog::info(abi_str2);
    spdlog::info(abi_str3);
}
```



### 👉Reflecting Classes and Enumerations

All runtime reflection functions are located in `<essence/meta/runtime/*.hpp>`.

All compile-time reflection features can be found in `<essence/meta/*.hpp>`.

#### Background

The C++ Reflection Proposal has not yet been accepted into the standard. However, in modern C++ compilers, there are some "magic" macros that can be evaluated dynamically within a functional context. For example, GCC provides `__PRETTY_FUNCTION__`, which returns a literal string of the enclosing function's signature, while MSVC offers `__FUNCSIG__` with similar functionality. Leveraging the robust compile-time evaluation capabilities introduced in C++20, we can implement a basic reflection library by parsing the output of these macros.

#### Creating a type fingerprint

The `meta::fingerprint` class stores compile-time information about a type, including both the raw and friendly names. The code below shows how to create a fingerprint which is evaluated at compile time.

```c++
#include <type_traits>

#include <essence/char8_t_remediation.hpp>
#include <essence/meta/fingerprint.hpp>

#include <spdlog/spdlog.h>

struct foo{};

constexpr essence::meta::fingerprint foo_metadata{std::type_identity<foo>{}};

spdlog::info(U8("The type name is {} and the friendly name is {}."), foo_metadata.name(), foo_metadata.friendly_name());
```

The implementation contains predefined friendly names of all arithmetic types as well as other built-in types like `char`, `char8_t`, `char16_t`, `char32_t`, `std::byte` and `bool`. Additionally, container types such as `std::basic_string<T>`(with `T` = `char`, `char8_t`, `char16_t`, `char32_t`) and  `std::vector<T>` also have specialized friendly names by default.

| Type                          | Friendly Name                |
| ----------------------------- | ---------------------------- |
| `std::int8_t`                 | `int8`                       |
| `std::int16_t`                | `int16`                      |
| `std::int32_t`                | `int32`                      |
| `std::int64_t`                | `int64`                      |
| `std::uint8_t`                | `uint8`                      |
| `std::uint16_t`               | `uint16`                     |
| `std::uint32_t`               | `uint32`                     |
| `std::uint64_t`               | `uint64`                     |
| `float`                       | `float`                      |
| `double`                      | `double`                     |
| `long double`                 | `long double`                |
| `bool`                        | `boolean`                    |
| `std::byte`                   | `byte`                       |
| `char`                        | `char`                       |
| `wchar_t`                     | `wchar`                      |
| `char8_t`                     | `u8char`                     |
| `char16_t`                    | `u16char`                    |
| `char32_t`                    | `u32char`                    |
| `std::basic_string<char>`     | `string`                     |
| `std::basic_string<wchar_t>`  | `wstring`                    |
| `std::basic_string<char8_t>`  | `u8string`                   |
| `std::basic_string<char16_t>` | `u16string`                  |
| `std::basic_string<char32_t>` | `u32string`                  |
| `std::vector<T>`              | `vector<friendly name of T>` |

More built-in types especially container types may be added in subsequent versions.

#### Usage of `meta::literal_string`

The `meta::literal_string` is a `struct` template satisfying the [`LiteralType`](https://en.cppreference.com/w/cpp/named_req/LiteralType) named requirement (C++20), that can be evaluated in a constexpr context such as non-type template arguments. It allows you to store a string literal at compile time, so partial code of the reflection module contains interfaces that use `meta::literal_string` as return values. Here’s an example to illustrate how such a `struct` template could be used:

```c++
#include <string_view>

#include <essence/char8_t_remediation.hpp>
#include <essence/literal_string.hpp>

using essence::meta::literal_string;

constexpr literal_string str{U8("Constexpr String Here")};
constexpr std::string_view strv{str}; // Implicit conversion to std::string_view.
constexpr const char* strp = str.c_str(); // Gets the underlying pointer.
constexpr bool equal = str == str;

template<literal_string S>
struct some_class_with_literal_string{
    static constexpr std::string_view str{S};
};

const some_class_with_literal_string<U8("Non-type Arg")> obj;
```

`meta::literal_string` provides iteration function pairs `begin` and `end` and therefore all STL algorithms can be applied to any instances of it.

```c++
#include <algorithm>
#include <vector>

#include <essence/char8_t_remediation.hpp>
#include <essence/literal_string.hpp>

using essence::meta::literal_string;

constexpr literal_string str{U8("Hello World")};
std::vector<char> chars;

std::ranges::copy(str, std::back_inserter(chars));
```

#### Converting booleans to/from strings

Several utility functions are available to facilitate the conversion of boolean values to and from string representations, such as the literals `"true"` and `"false"`.

```c++
#include <essence/char8_t_remediation.hpp>
#include <essence/meta/boolean.hpp>

#include <spdlog/spdlog.h>

namespace emr = essence::meta::runtime;

if (auto bool_value = emr::from_string<bool>(U8("true"))) {
    spdlog::info(U8("The string 'true' has been parsed into boolean value {}."), *bool_value);
}
    
spdlog::info(U8("The boolean value {} has been converted to string '{}'."), emr::to_string(false));
```

#### Parsing enumeration names to/from strings

In production scenarios, it is common to retrieve the text representation of an enumeration name or parse a string to obtain the corresponding enumeration value. Although C++ lacks built-in reflection capabilities, we can determine the range of an enumeration type and use the internal `meta::get_literal_string_v` function to convert each enumeration value into its string representation. By default, the range is set to [−64, 64], which is broad enough to cover most typical use cases.

```c++ 
template <typename T>
consteval auto get_enum_searching_range() noexcept {
    return std::pair<std::int64_t, std::int64_t>{-64, 64};
};
```

You can customize the range for a specific enumeration type by overloading the `meta::get_enum_searching_range` function template.

```c++
#include <concepts>
#include <utility>

#include <essence/meta/enum.hpp>

enum class my_kind {
    boy = 100,
    girl = 105,
    other = 109,
};

namespace essence::meta {
    template<std::same_as<my_kind>>
    consteval auto get_enum_searching_range() noexcept {
        return std::pair{100, 109};
    }
}
```

The following code demonstrates how to implement conversions between strings and enumeration names:

```c++
#include <essence/char8_t_remediation.hpp>
#include <essence/meta/runtime/enum.hpp>

#include <spdlog/spdlog.h>

namespace emr = essence::meta::runtime;

enum class star_type {
    sun,
    alpha_centauri,
    sirius,
};

// The from_string<T> function returns a std::optional<T>.
if (auto enum_value = emr::from_string<star_type>(U8("alpha_centauri"))) {
    spdlog::info(U8("The parsed enumeration is {}."), emr::to_string(*enum_value));
}
```

The `meta::runtime::from_string` function template is designed to be flexible with naming conventions. This means it can handle multiple naming styles and still produce the same result. For instance, if you have an enumeration or variable name in `PascalCase`, the function can correctly interpret names written in other common formats like `snake_case` or `camelCase`. This allows for greater flexibility when processing strings, making it easier to work with various naming conventions without worrying about case sensitivity or specific formatting.

```c++
#include <essence/char8_t_remediation.hpp>
#include <essence/meta/runtime/boolean.hpp>
#include <essence/meta/runtime/enum.hpp>

#include <spdlog/spdlog.h>

enum class language_type {
    american_english,
    british_english,
};

namespace emr = essence::meta::runtime;

spdlog::info(emr::to_string(static_cast<bool>(emr::from_string<language_type>(U8("american_english"))))); // Outputs 'true'.
spdlog::info(emr::to_string(static_cast<bool>(emr::from_string<language_type>(U8("americanEnglish"))))); // Outputs 'true'.
spdlog::info(emr::to_string(static_cast<bool>(emr::from_string<language_type>(U8("AmericanEnglish")))); // Outputs 'true'.
```

#### Enumerating non-static data members of a structure

The `meta::runtime::enumerate_data_members` function provides a way to enumerate the public non-static data members of a `struct`. The core implementation relies on the [`Structured Binding`](https://en.cppreference.com/w/cpp/language/structured_binding) feature introduced in C++17, which enables the binding of `struct` members to variables in a single expression. By combining `Structured Binding` with the name-fetching mechanism described earlier, we can obtain name-reference pairs in the final results. Here is an example illustrating how to use this function:

```cpp
#include <essence/char8_t_remediation.hpp>
#include <essence/meta/runtime/struct.hpp>

#include <spdlog/spdlog.h>

struct foo {
    int a{};
    long b{};
    double c{};
};

constexpr foo entity{
    .a = 1, 
    .b = 2,
    .c = 3,
};

meta::runtime::enumerate_data_members(entity, [](const auto&... members) {
    auto handler = [](const auto& arg) { spdlog::info(U8("{} = {}"), arg.name, arg.reference); };

    (handler(members), ...);
});
```

The type of `arg.name` shown above is `meta::literal_string` that can be implicitly converted to `std::string_view`. The `arg.reference` is a reference to the original data member. **Please note that in this implementation, the number of public non-static data members in a `struct` should not exceed 100.**

#### Enumerating enumeration names

Similar to `meta::runtime::enumerate_data_members`, the `meta::runtime::get_enum_names` and `meta::runtime::get_enum_names_only` functions are designed to retrieve the names of an enumeration. The difference between them is illustrated below:

```C++
#include <type_traits>

#include <essence/char8_t_remediation.hpp>
#include <essence/meta/runtime/enum.hpp>

#include <spdlog/spdlog.h>

namespace emr = essence::meta::runtime;

enum class test_enum {
    something,
    anything,
    everything,
};

for (auto&& item : emr::get_enum_names_only<test_enum>()) {
    spdlog::info(item);
}

for (auto&& [name, value] : emr::get_enum_names<test_enum>()) {
    spdlog::info(U8("{} = {}"), name, static_cast<std::underlying_type_t<test_enum>>(value));
}
```
Also, the range of an enumeration is decided by the `meta::get_enum_searching_range` function described above.



### 👉Macro-free JSON Serialization/Deserialization

#### Extensions of `nlohmann::json`

JSON serialization and deserialization are implemented directly using the reflection support within this library, which enables efficient data member extraction instead of the original macro-based methodology within `nlohmann::json`. The header file `<essence/json_compat.hpp>` defines `essence::json` with a custom JSON serializer provided by the `meta::runtime::json_serializer` class. The custom serializer is able to reflect the data members of a `struct` and convert the value to or from a JSON value fast.

```c++
using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, double,
        std::allocator, meta::runtime::json_serializer>;
```

In addition, the header file includes the concept `essence::json_serializable<T>`, which allows users to check if a `struct` can be serialized by `meta::runtime::json_serializer`.

```c++
#include <string>

#include <essence/json_compat.hpp>

if constexpr(essence::json_serializable<std::string>) {
    // This condition is always true here.
}
```

To use the features of this version of JSON class, you simply need to manipulate the JSON value as you would with the regular `nlohmann::json`. Below is an example code:

```c++
#include <fstream>

#include <essence/char8_t_remediation.hpp>
#include <essence/json_compat.hpp>

#include <spdlog/spdlog.h>

auto json = essence::json::parse(std::ifstream{U8("/tmp/test.json"), std::ios_base::in});

spdlog::info(json.dump(4));
```

#### Recapping `meta::runtime::json_serializer`

The default `meta::runtime::json_serializer` class supports serialization and deserialization from arithmetic types, `bool`, enumerations, `std::vector<T, Allocator>`, `std::map<Key, T, Compare, Allocator>`, `std::optional<T>`, `std::basic_string<CharT, CharTraits, Allocator>` and `struct` types with public non-static data members. The list below reveals how C++ types are mapped to their corresponding JSON values. 

| C++ Type                                                     | JSON Value      |
| ------------------------------------------------------------ | --------------- |
| Arithmetic types `std::int**_t`, `std::uint**_t`, `float`, `double` | `Number`        |
| `bool`                                                       | `true`, `false` |
| `std::nullptr_t`, `std::nullopt_t`, `std::optional<T>` (with an empty state) | `null`          |
| `struct` types (satisfying `essence::json_serializable`), `std::optional<T>` (that has a value) | `object`        |
| `std::vector<T, Allocator>`                                  | `array` of  `T` |
| `std::map<Key, T, Compare, Allocator>`                       | `object`        |
| `std::basic_string<CharT, CharTraits, Allocator>`            | `string`        |

This implementation allows users to manually control the naming convention used during serialization and deserialization. The `meta::runtime::json_serializer` will check if a `json_serialization` enumeration is defined within a `struct` type. Currently, `snake_case`, `camelCase`, and `PascalCase` naming conventions are supported. The following code demonstrates how to define this nested enumeration:

```c++
struct entity {
    enum class json_serialization { camel_case };
};

struct model {
    enum class json_serialization { pascal_case };
};

struct record {
    enum class json_serialization { snake_case };
};
```

By default, an enumeration value is serialized as a JSON number, and vice versa. To enable conversion of an enumeration to its string representation, you need to add an `enum_to_string` member to the `json_serialization` enumeration described above.

```c++
struct entity {
    enum class json_serialization {
        camel_case,
        enum_to_string,
    };
};
```

#### Serializing JSON to arbitrary values

Here are some examples of JSON serialization including complex values:

```c++
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <essence/char8_t_remediation.hpp>
#include <essence/json_compat.hpp>

#include <spdlog/spdlog.h>

struct foo {
    int a{};
    long b{};
    double c{};
};

struct entity {
    enum class json_serialization { camel_case };

    int blice_age{};
    long bob_index_first{};
    double john_make_living{};
    std::optional<std::string> str;
    std::optional<std::vector<foo>> items;
    std::map<std::string, std::optional<foo>> mapping;
};

essence::json json(entity{
    .blice_age        = 21,
    .bob_index_first  = 22,
    .john_make_living = 12.34,
    .str = std::nullopt,
    .items = std::vector<foo>{{1, 2, 3}},
    .mapping = {
                  {U8("Today"), foo{.a = 1999}}
               },
});

spdlog::info(json.dump(4));
```

#### Deserializing from various objects

This is some sample code that supports conversion of complex values:

```cpp
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <essence/char8_t_remediation.hpp>
#include <essence/json_compat.hpp>

#include <spdlog/spdlog.h>

struct foo {
    int a{};
    long b{};
    double c{};
};

struct entity {
    enum class json_serialization { pascal_case };

    int blice_age{};
    long bob_index_first{};
    double john_make_living{};
    std::optional<std::string> str;
    std::optional<std::vector<foo>> items;
    std::map<std::string, std::optional<foo>> mapping;
};

essence::json json{
    {U8("BliceAge"), 12},
    {U8("BobIndexFirst"), 13},
    {U8("JohnMakeLiving"), 67.89},
    {U8("Mapping"), {{U8("Tomorrow"), essence::json(foo{.c = 101})}}},
};

auto entity = json.get<entity>();

spdlog::info(json(entity).dump(4));
```



### 👉Command-Line Parser Compatible with GNU/Linux Bash

#### Conventions of `getopt` on GNU/Linux

The `getopt` command in GNU/Linux is used for parsing command-line arguments in scripts. It allows you to define short and long options and then extract the arguments provided to them by the user. Here is the typical contract for the `getopt` command:

- Short Switches: `-s`
- Short Options with Values: `-l short,options`, `-l=short,options`
- Long Switches: `--switch`
- Long Switches with Values: `--list short,options`, `--list=short,options`
- Combined Short Switches: `-abc` (that is equivalent to `-a -b -c`)

#### The compatible implementation: `cli::option<T>`



```cpp
#include <string>
#include <string_view>
#include <vector>

#include <essence/char8_t_remediation.hpp>
#include <essence/cli/option.hpp>
#include <essence/cli/arg_parser.hpp>
#include <essence/range.hpp>

#include <spdlog/spdlog.h>

namespace {
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
}

int main(){
    // 字符串选项。
    auto opt1 = cli::option<std::string>{}
                 .set_name(U8("set_text"))
                 .set_description(U8("设置文本。"))
                 .add_aliases(U8("t"))
                 .set_valid_values(U8("a"), U8("b"), U8("c"))
                 .as_abstract();

    // 开关选项。
    auto opt2 = cli::option<bool>{}
                 .set_name(U8("set_flower"))
                 .set_description(U8("开启花花。"))
                 .add_aliases(U8("f"))
                 .as_abstract();
    
    // 枚举值数组选项。
    auto opt3 = cli::option<std::vector<tex>>{}
                 .set_name(U8("set_target"))
                 .set_description(U8("设置目标列表。"))
                 .add_aliases(U8("d"))
                 .as_abstract();

    // 创建解析器。
    cli::arg_parser parser;
    
    parser.on_error([](std::string_view message) { spdlog::error(message); });
	
    parser.add_option(opt1);
    parser.add_option(opt2);
    parser.add_option(opt3);
	
    // 模拟命令行参数解析。
    parser.parse(std::array<abi::string, 6>{
        U8("--set_text"), U8("a"), U8("-f=false"), U8("hahaha"), U8("--set_target"), U8("fire,bat")});

    // 当 operator bool() 返回 true 时，代表最近一次解析成功。
    if (parser) {
        // 获取原始解析结果。
        for (auto&& [key, value] : parser.cached_result()) {
            spdlog::info(U8("{} = {}"), key, value.raw_value);
        }

        // 将原始解析结果反序列化为数据模型。
        if (auto m = parser.to_model<model>()) {
            spdlog::info(json(m).dump(4));
        }
        
        // 获取剩余未匹配的参数。
        auto joint = join_with(parser.unmatched_args(), std::string_view{U8(",")});

        spdlog::info(U8("未匹配的参数: {}"), std::string{joint.begin(), joint.end()});
    }
}
```



## Acknowledgements

This library includes several third-party submodules, each of which is exceptional and has garnered many stars on GitHub.

![](https://avatars.githubusercontent.com/u/159488?s=48&v=4)[nlohmann/json: JSON for Modern C++](https://github.com/nlohmann/json)

![](https://avatars.githubusercontent.com/u/7280830?s=48&v=4)[fmtlib/fmt: A modern formatting library](https://github.com/fmtlib/fmt)

![](https://avatars.githubusercontent.com/u/6154722?s=48&v=4)[microsoft/cpprestsdk: The C++ REST SDK is a Microsoft project for cloud-based client-server communication in native code using a modern asynchronous C++ API design.](https://github.com/microsoft/cpprestsdk)

![](https://avatars.githubusercontent.com/u/3279138?s=48&v=4)[openssl/openssl: TLS/SSL and crypto library](https://github.com/openssl/openssl)

![](https://avatars.githubusercontent.com/u/6052198?s=48&v=4)[gabime/spdlog: Fast C++ logging library.](https://github.com/gabime/spdlog)

![](https://avatars.githubusercontent.com/u/3170529?s=48&v=4)[boostorg/boost: Super-project for modularized Boost](https://github.com/boostorg/boost)

<img src="https://avatars.githubusercontent.com/u/45075615?s=48&v=4" width="56" height="56">[zlib-ng/zlib-ng: zlib replacement with optimizations for "next generation" systems.](https://github.com/zlib-ng/zlib-ng)

![](https://avatars.githubusercontent.com/u/69631?s=48&v=4)[facebook/zstd: Zstandard - Fast real-time compression algorithm](https://github.com/facebook/zstd)



## 👇下载开发包

- [RV1109 开发板  1.1.11（暂停维护）](http://pub:glsix666@192.168.15.39:3690/Fundamental/release/1.1.11/gx-miscellaneous-1.1.11-no-jni-arm-linux-rv1109.tgz)
- [RV1106 开发版 1.1.10（暂停维护）](http://pub:glsix666@192.168.15.39:3690/Fundamental/release/1.1.10/gx-miscellaneous-1.1.10-no-jni-arm-linux-rv1106.tgz)
- [Windows Debug 1.2.0 + JDK 8](http://pub:glsix666@192.168.15.39:3690/Fundamental/release/1.2.0/gx-miscellaneous-1.2.0-jdk-8-x86_64-windows-debug.zip)
- [Windows Release 1.3.0 + JDK 8 ](http://pub:glsix666@192.168.15.39:3690/Fundamental/release/1.3.0/gx-miscellaneous-1.3.0-jdk-8-x86_64-windows-release.zip)
- [x86_64 GNU Linux 1.3.0 + JDK 8, glibc >= 2.17](http://pub:glsix666@192.168.15.39:3690/Fundamental/release/1.3.0/gx-miscellaneous-1.3.0-jdk-8-x86_64-linux.tgz)
- [AArch64 GNU Linux 1.3.0 + JDK21](http://pub:glsix666@192.168.15.39:3690/Fundamental/release/1.3.0/gx-miscellaneous-1.3.0-jdk-21-aarch64-linux.tgz)
- [Android 1.1.11（暂停维护）](http://pub:glsix666@192.168.15.39:3690/Fundamental/release/1.1.11/gx-miscellaneous-1.1.11-aarch64-linux-android.tgz)
- [下载 Windows C++ 运行时：Visual C++ 2015, 2017 and 2022 x86_64 Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)
