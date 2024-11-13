include_guard()
include(ESUtil)
include(ESBoost)
include(ESOpenSSL)
include(ESEmscripten)
include(ESLangCompiler)
include(ESThirdPartyLibrary)
include(FetchContent)

if(NOT EMSCRIPTEN)
    find_package(Threads REQUIRED)
endif()

if(ES_WITH_JNI)
    find_package(JNI REQUIRED)
endif()

if(ES_STATIC_RUNTIME)
    set(runtime_args STATIC_RUNTIME)
else()
    set(runtime_args "")
endif()

message(STATUS "runtime_args: ${runtime_args}")

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    FetchContent_Declare(
        perl-windows
        URL https://github.com/refvalue/cpp-essence/releases/download/v5.36.0-perl-windows/perl-5.36-windows.zip
    )

    FetchContent_MakeAvailable(perl-windows)
    es_add_to_env(
        PREPEND
        NAME PATH
        VALUE ${CMAKE_BINARY_DIR}/_deps/perl-windows-src/bin
    )
endif()

if(ES_WITH_TESTS)
    es_deploy_lang_compiler("https://github.com/refvalue/cpp-essence/releases/download/v1.0.0-lang-compiler")
    message(STATUS "ES_LANG_COMPILER_EXECUTABLE: ${ES_LANG_COMPILER_EXECUTABLE}")

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    # For Windows: Prevents overriding the parent project's compiler/linker settings.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

set(extra_openssl_args no-tests no-afalgeng no-aria no-async no-autoload-config no-blake2 no-bf no-camellia no-cast no-chacha no-cmac no-cms no-cmp no-comp no-ct no-des no-dgram no-dh no-dsa no-dtls no-ec2m no-engine no-filenames no-gost no-idea no-ktls no-mdc2 no-md4 no-multiblock no-nextprotoneg no-ocsp no-ocb no-poly1305 no-psk no-rc2 no-rc4 no-rmd160 no-seed no-siphash no-siv no-srp no-srtp no-ssl3 no-ssl3-method no-ts no-ui-console no-whirlpool)

if(CMAKE_CROSSCOMPILING AND NOT EMSCRIPTEN)
    # The Boost library is unnecessary if the websocketpp library is excluded
    # when compiling the cpprestsdk on Windows.
    message(STATUS "======== CROSS-COMPILING ========")
    set(
        extra_cmake_args
        -DES_TOOLCHAIN_PREFIX=${ES_TOOLCHAIN_PREFIX}
    )

    if(ANDROID)
        list(APPEND extra_openssl_args CC ${CMAKE_C_ANDROID_TOOLCHAIN_PREFIX}clang)
        list(APPEND extra_cmake_args -DES_NDK_ROOT=${ES_NDK_ROOT})
    endif()
elseif(EMSCRIPTEN)
    set(extra_cmake_args "")
else()
    set(extra_cmake_args "")
endif()

es_make_openssl(
    REQUIRED
    STATIC
    ${runtime_args}
    PARALLEL_BUILD
    SYNC_BUILD_TYPE
    ${extra_openssl_args}
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/openssl
)

message(STATUS "====OPENSSL_CRYPTO_LIBRARY: ${OPENSSL_CRYPTO_LIBRARY}")

es_make_install_third_party_library(
    fmt
    REQUIRED
    ${runtime_args}
    PARALLEL_BUILD
    SYNC_BUILD_TYPE
    GENERATOR ${CMAKE_GENERATOR}
    CMAKE_ARGS
    -DFMT_TEST=OFF
    -DCMAKE_CXX_STANDARD=20
    -DCMAKE_CXX_STANDARD_REQUIRED=ON
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    ${extra_cmake_args}
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/fmt
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)

if(ES_HAVE_STD_FORMAT)
    set(spdlog_extra_args -DSPDLOG_USE_STD_FORMAT=ON)
else()
    set(spdlog_extra_args -DSPDLOG_FMT_EXTERNAL=ON)
endif()

es_make_install_third_party_library(
    spdlog
    REQUIRED
    ${runtime_args}
    PARALLEL_BUILD
    SYNC_BUILD_TYPE
    GENERATOR ${CMAKE_GENERATOR}
    CMAKE_ARGS
    -DSPDLOG_BUILD_PIC=ON
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    ${extra_cmake_args}
    ${spdlog_extra_args}
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/spdlog
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)

set(
    zlibng_cmake_args
    -DZLIB_ENABLE_TESTS=OFF
    -DZLIB_COMPAT=ON
    -DBUILD_SHARED_LIBS=OFF
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    ${extra_cmake_args}
)

if(EMSCRIPTEN)
    list(
        APPEND zlibng_cmake_args
        -DWITH_OPTIM=OFF
    )
endif()

es_make_install_third_party_library(
    ZLIB
    MODULE
    REQUIRED
    ${runtime_args}
    PARALLEL_BUILD
    SYNC_BUILD_TYPE
    GENERATOR ${CMAKE_GENERATOR}
    CMAKE_ARGS ${zlibng_cmake_args}
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/zlib-ng
    RESULT_INSTALL_DIR ZLIB_ROOT
)

es_make_install_third_party_library(
    zstd
    REQUIRED
    ${runtime_args}
    PARALLEL_BUILD
    SYNC_BUILD_TYPE
    GENERATOR ${CMAKE_GENERATOR}
    CMAKE_ARGS
    -DZSTD_BUILD_STATIC=ON
    -DZSTD_BUILD_SHARED=OFF
    -DZSTD_BUILD_TESTS=OFF
    -DZSTD_BUILD_PROGRAMS=OFF
    -DZSTD_MULTITHREAD_SUPPORT=OFF
    ${extra_cmake_args}
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/zstd/build/cmake
)

# The cpprestsdk library uses Ninja as the build system when cross-compiling.
if(ES_WITH_NET)
    if(WIN32)
        set(cpprestsdk_extra_cmake_args "")
    else()
        # Unix/Linux needs Boost.
        set(
            make_boost_extra_args
            TOOLSET ${CMAKE_CXX_COMPILER}
            TARGET_OS linux
        )

        es_make_boost(
            REQUIRED
            STATIC
            ${runtime_args}
            PARALLEL_BUILD
            SYNC_BUILD_TYPE
            SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/boost-archive-for-cpprestsdk
            ${make_boost_extra_args}
        )

        set(
            cpprestsdk_extra_cmake_args
            -DBOOST_ROOT=${BOOST_ROOT}
            -DBoost_USE_STATIC_LIBS=ON
            -DBoost_USE_STATIC_RUNTIME=${ES_STATIC_RUNTIME}
        )
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND cpprestsdk_extra_cmake_args -DCMAKE_CXX_FLAGS=-Wno-format-truncation)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        list(APPEND cpprestsdk_extra_cmake_args -DCMAKE_CXX_FLAGS=-Wno-enum-constexpr-conversion)
    endif()

    # Boost is excluded without compiling websocketpp as a dependency.
    es_make_install_third_party_library(
        cpprestsdk
        REQUIRED
        ${runtime_args}
        #PARALLEL_BUILD
        SYNC_BUILD_TYPE
        GENERATOR ${CMAKE_GENERATOR}
        CMAKE_ARGS
        -G Ninja
        -DWERROR=OFF
        -DBUILD_TESTS=OFF
        -DBUILD_SAMPLES=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DCPPREST_EXCLUDE_WEBSOCKETS=ON
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DZLIB_ROOT=${ZLIB_ROOT}
        -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}
        -DOPENSSL_USE_STATIC_LIBS=TRUE
        ${extra_cmake_args}
        ${cpprestsdk_extra_cmake_args}
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/cpprestsdk
    )
endif()
