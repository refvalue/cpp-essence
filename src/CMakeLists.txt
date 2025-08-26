set(target_name cpp-essence)

add_library(${target_name} SHARED)
list(APPEND ES_PACKAGING_TARGETS ${target_name})
set(ES_PACKAGING_TARGETS ${ES_PACKAGING_TARGETS} PARENT_SCOPE)

file(
    GLOB_RECURSE miu_sources
    CONFIGURE_DEPENDS
    *.ixx
)

file(
    GLOB_RECURSE internal_piu_sources
    CONFIGURE_DEPENDS
    *.cxx
)

file(
    GLOB_RECURSE private_sources
    CONFIGURE_DEPENDS
    *.cpp
)

macro(es_filter_sources pattern)
    list(
        FILTER private_sources
        EXCLUDE REGEX "${pattern}"
    )

    list(
        FILTER internal_piu_sources
        EXCLUDE REGEX "${pattern}"
    )

    list(
        FILTER miu_sources
        EXCLUDE REGEX "${pattern}"
    )
endmacro()

# Excludes compat and JNI sources which is an independent target.
es_filter_sources([=[^.*/(jni|compat)/.*(.ixx|.cxx|.cpp)$]=])

if(WIN32)
    es_filter_sources([=[^.*(macos|linux)/.*(.ixx|.cxx|.cpp)$]=])
elseif(APPLE)
    es_filter_sources([=[^.*(win32|linux)/.*(.ixx|.cxx|.cpp)$]=])
elseif(UNIX)
    es_filter_sources([=[^.*(win32|macos)/.*(.ixx|.cxx|.cpp)$]=])
else()
    message(FATAL_ERROR "Unsupported platform.")
endif()

# Excludes all sources depending on <thread>, <atomic> when using Emscripten.
if(EMSCRIPTEN)
    es_filter_sources([=[^((.*(thread|delegate|interruptable_timer|atomic_readable_buffer)|.*/params/.*|.*/crypto/.*(asymm|pubkey).*))(.ixx|.cxx|.cpp)$]=])
endif()

include(ESUtil)
es_dump_list(miu_sources)
es_dump_list(internal_piu_sources)
es_dump_list(private_sources)

target_sources(
    ${target_name}
    PUBLIC
    FILE_SET CXX_MODULES
    FILES ${miu_sources} ${internal_piu_sources}
    PRIVATE
    ${private_sources}
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
    ${ES_PUBLIC_INCLUDE_DIR}
)

target_compile_definitions(
    ${target_name}
    PUBLIC
    $<$<BOOL:${ES_HAVE_STD_FORMAT}>:CPP_ESSENCE_USE_STD_FORMAT=1>
)

target_compile_definitions(
    ${target_name}
    PRIVATE
    ES_IS_CPPESSENCE_IMPL=1
)

target_link_libraries(
    ${target_name}
    PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    PRIVATE
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
    VERSION ${ES_PROJECT_VERSION}
)

target_compile_features(
    ${target_name}
    PRIVATE
    cxx_std_23
)

if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_link_options(
        ${target_name}
        PRIVATE
        LINKER:-Bsymbolic
        LINKER:--exclude-libs,ALL
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
    ${CMAKE_SOURCE_DIR}/cmake/config/user_i18n.cpp.in
    ${CMAKE_SOURCE_DIR}/cmake/config/user_i18n.ixx.in
    |
    ${CMAKE_SOURCE_DIR}/cmake/py/patch_exported_modular_target.py
    DESTINATIONS cmake cmake/config cmake/py
)
