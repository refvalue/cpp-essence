set(target_version 1.0.0)
set(target_name cpp-essence)
set(target_name_header_only cpp-essence-header-only)
set(package_name CppEssence)
set(package_name_header_only CppEssenceHeaderOnly)

add_library(${target_name} SHARED)
add_library(${target_name_header_only} INTERFACE)

file(
    GLOB meta_sources
    CONFIGURE_DEPENDS
    ${ES_PUBLIC_INCLUDE_DIR}/essence/meta/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/meta/detail/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/meta/detail/nested/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/meta/runtime/*.hpp
)

file(
    GLOB private_sources
    CONFIGURE_DEPENDS
    ${ES_PUBLIC_INCLUDE_DIR}/essence/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/abi/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/cli/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/crypto/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/crypto/abstract/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/crypto/params/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/globalization/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/globalization/abstract/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/imaging/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/io/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/io/abstract/*.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/memory/*.hpp
    *.cpp
    abi/*.cpp
    cli/*.cpp
    crypto/*.hpp
    crypto/*.cpp
    crypto/params/*.hpp
    crypto/params/*.cpp
    globalization/*.hpp
    globalization/*.cpp
    imaging/*.hpp
    imaging/*.cpp
    io/*.hpp
    io/*.cpp
    io/abstract/*.hpp
)

list(
    APPEND private_sources
    ${meta_sources}
)

# Excludes all sources depending on <thread>, <atomic> when using Emscripten.
if(EMSCRIPTEN)
    list(
        FILTER private_sources
        EXCLUDE REGEX "^.*(thread|delegate|interruptable_timer|atomic_readable_buffer)(.hpp|.cpp)$"
    )

    list(
        FILTER private_sources
        EXCLUDE REGEX "^.*/params/.*(.hpp|.cpp)$"
    )

    list(
        FILTER private_sources
        EXCLUDE REGEX "^.*/crypto/.*(asymm|pubkey).*(.hpp|.cpp)$"
    )
elseif(ES_WITH_NET)
    file(
        GLOB private_net_sources
        CONFIGURE_DEPENDS
        ${ES_PUBLIC_INCLUDE_DIR}/essence/net/*.hpp
        net/*.hpp
        net/*.cpp
    )

    list(
        APPEND private_sources
        ${private_net_sources}
    )
endif()

if(ANDROID)
    list(
        FILTER private_sources
        EXCLUDE REGEX "^.*(sse_).*(.hpp|.cpp)$"
    )
endif()

if(NOT ANDROID OR NOT ES_WITH_NET)
    list(
        FILTER private_sources
        EXCLUDE REGEX "^.*jni_entry.cpp$"
    )
endif()

include(ESUtil)
es_dump_list(private_sources)

target_sources(
    ${target_name}
    PRIVATE ${private_sources}
)

target_sources(
    ${target_name_header_only} INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${ES_PUBLIC_INCLUDE_DIR}
    FILES
    ${meta_sources}
    ${ES_PUBLIC_INCLUDE_DIR}/essence/memory/atomic_readable_buffer.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/memory/nonuniform_grid_buffer.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/argb_color.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/array.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/basic_string.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/boolean.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/char.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/char8_t_remediation.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/compat.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/data_view.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/delegate.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/error_extensions.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/flags.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/format_remediation.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/functional.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/hashing.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/json.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/json_compat.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/json_extensions.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/managed_handle.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/native_handle.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/noncopyable.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/numeric_conversion.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/optional.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/range.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/rect.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/reflection.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/scope.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/source_location.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/span.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/string.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/type_identity.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/type_list.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/type_traits.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/yuv_color.hpp
    ${ES_PUBLIC_INCLUDE_DIR}/essence/zstring_view.hpp
)

if(NOT "${ES_TOOLCHAIN_TARGET_NAME}" STREQUAL "")
    message(STATUS "ES_TOOLCHAIN_TARGET_NAME: ${ES_TOOLCHAIN_TARGET_NAME}")

    include(ESCompat)
    es_patch_compat(
        TARGET ${target_name}
        TOOLCHAIN_TARGET_NAME ${ES_TOOLCHAIN_TARGET_NAME}
    )
endif()

target_include_directories(
    ${target_name}
    INTERFACE
    $<BUILD_INTERFACE:${ES_PUBLIC_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${ES_PUBLIC_INCLUDE_DIR}/essence
)

target_compile_definitions(
    ${target_name}
    PUBLIC
    CPP_ESSENCE_HEADER_ONLY=0
    SPDLOG_SHARED_LIB=1
    SPDLOG_COMPILED_LIB=1
    SPDLOG_FMT_EXTERNAL=1
)

target_compile_definitions(
    ${target_name}
    PRIVATE
    ES_IS_CPPESSENCE_IMPL=1
    spdlog_EXPORTS=1
)

target_compile_definitions(
    ${target_name_header_only}
    INTERFACE
    CPP_ESSENCE_HEADER_ONLY=1
)

target_link_libraries(
    ${target_name}
    PUBLIC
    fmt::fmt
    PRIVATE
    spdlog::spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    zstd::libzstd_static
)

if(NOT EMSCRIPTEN)
    target_link_libraries(
        ${target_name}
        PRIVATE
        Threads::Threads
    )

    target_compile_definitions(
        ${target_name}
        PUBLIC
        CPP_ESSENCE_HAS_THREADS=1
    )
endif()

if(ES_WITH_NET)
    target_link_libraries(
        ${target_name}
        PRIVATE
        cpprestsdk::cpprest
    )

    if(CMAKE_CROSSCOMPILING)
        target_link_libraries(
            ${target_name}
            PRIVATE
            Boost::boost
        )
    endif()

    target_compile_definitions(
        ${target_name}
        PUBLIC
        CPP_ESSENCE_HAS_NET=1
    )
endif()

if(ANDROID)
    target_link_libraries(
        ${target_name}
        PRIVATE
        log
    )
endif()

include(ESVersion)

es_target_version_definitions(
    TARGETS ${target_name}
    PUBLIC
    PREFIX CPP_ESSENCE
    VERSION ${target_version}
)

es_target_version_definitions(
    TARGETS ${target_name_header_only}
    INTERFACE
    PREFIX CPP_ESSENCE
    VERSION ${target_version}
)

target_link_libraries(
    ${target_name_header_only}
    INTERFACE
    fmt::fmt-header-only
    spdlog::spdlog_header_only
)

set_target_properties(
    ${target_name}
    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_options(
        ${target_name}
        PRIVATE
        LINKER:-Bsymbolic
        LINKER:--exclude-libs,libfmt.a
        LINKER:--exclude-libs,libcpprest_2_10.a
        LINKER:--exclude-libs,libcrypto.a
        LINKER:--exclude-libs,libssl.a
        LINKER:--exclude-libs,libz.a
        LINKER:--exclude-libs,libzstd.a
        $<$<CONFIG:RELEASE>:-s>
    )

    set_target_properties(${target_name}
        PROPERTIES
        CXX_VISIBILITY_PRESET hidden
    )

    if(NOT EMSCRIPTEN)
        target_link_libraries(
            ${target_name}
            PRIVATE
            atomic
        )
    endif()
endif()

include(ESPackaging)

set_property(
    TARGET ${target_name}
    PROPERTY ES_INSTALL_FILES
    FILES
    ${CMAKE_SOURCE_DIR}/cmake/ArmToolchain.cmake
    ${CMAKE_SOURCE_DIR}/cmake/CMakeRC.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESBoost.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESCCXXRuntime.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESCompat.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESEmscripten.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESLangCompiler.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESOpenSSL.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESPackaging.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESThirdPartyLibrary.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESUtil.cmake
    ${CMAKE_SOURCE_DIR}/cmake/ESVersion.cmake
    ${CMAKE_SOURCE_DIR}/cmake/NDKToolchain.cmake
    ${CMAKE_SOURCE_DIR}/cmake/RV1109Toolchain.cmake
    ${CMAKE_SOURCE_DIR}/cmake/RV3588Toolchain.cmake
    |
    ${CMAKE_SOURCE_DIR}/cmake/config/user_globalization.hpp.in
    ${CMAKE_SOURCE_DIR}/cmake/config/user_globalization.cpp.in
    DESTINATIONS cmake cmake/config
)

es_make_install_package(
    TARGET_NAME ${target_name}
    PACKAGE_NAME ${package_name}
    TARGET_VERSION ${target_version}
    PATH_VARS CMAKE_INSTALL_PREFIX
)

es_make_install_package(
    TARGET_NAME ${target_name_header_only}
    PACKAGE_NAME ${package_name_header_only}
    TARGET_VERSION ${target_version}
    FILE_SET_HEADERS
    INCLUDE_DIR include-header-only
    PATH_VARS CMAKE_INSTALL_PREFIX
)
