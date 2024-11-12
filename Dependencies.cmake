include_guard()
include(ESUtil)
include(ESBoost)
include(ESOpenSSL)
include(ESEmscripten)
include(ESLangCompiler)
include(ESThirdPartyLibrary)

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

es_deploy_lang_compiler("http://misc.refvalue.org/cpp-essence-lang-compiler")
message(STATUS "ES_LANG_COMPILER_EXECUTABLE: ${ES_LANG_COMPILER_EXECUTABLE}")

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
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    ${extra_cmake_args}
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/fmt
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)

es_make_install_third_party_library(
    spdlog
    REQUIRED
    ${runtime_args}
    PARALLEL_BUILD
    SYNC_BUILD_TYPE
    GENERATOR ${CMAKE_GENERATOR}
    CMAKE_ARGS
    -DSPDLOG_BUILD_PIC=ON
    -DSPDLOG_FMT_EXTERNAL=ON
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    ${extra_cmake_args}
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
