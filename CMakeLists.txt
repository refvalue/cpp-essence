# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.25)

project(gx-miscellaneous C CXX)

option(ES_WITH_NET "Whether to include the net support." ON)
option(ES_WITH_JNI "Whether to include JNI support." ON)
option(ES_WITH_TESTS "Whether to compile tests." ON)
option(ES_WITH_LANG_COMPILER "Whether to compile the language compiler for globalization." ON)
option(ES_EMBEDDED_WASM "Whether to embed .wasm files into glue scripts." OFF)
option(ES_STATIC_RUNTIME "Whether to statically link to the C++ standard library." OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if(ES_WITH_NET AND EMSCRIPTEN)
    message(FATAL_ERROR "The net support has not been implemented on WebAssembly.")
endif()

if(ANDROID)
    set(ES_WITH_JNI OFF)
    set(ES_STATIC_RUNTIME OFF) # Dynamically linking to libc++.
    string(REPLACE "-linux-android-" "-linux-android${CMAKE_SYSTEM_VERSION}-" CMAKE_C_ANDROID_TOOLCHAIN_PREFIX ${CMAKE_C_ANDROID_TOOLCHAIN_PREFIX})
    message(STATUS "CMAKE_C_ANDROID_TOOLCHAIN_PREFIX: ${CMAKE_C_ANDROID_TOOLCHAIN_PREFIX}")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(ES_PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    string(APPEND CMAKE_CXX_FLAGS " -frtti -fexceptions -fvisibility=hidden")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    string(APPEND CMAKE_C_FLAGS " /utf-8")
else()
    string(APPEND CMAKE_C_FLAGS " -fvisibility=hidden")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    string(APPEND CMAKE_CXX_FLAGS " /utf-8")
endif()

if(ES_STATIC_RUNTIME)
    include(ESCCXXRuntime)
    es_replace_c_cxx_runtime_flags(STATIC_RUNTIME)
endif()

include(CTest)
include(Dependencies.cmake)

add_subdirectory(src)

if(EMSCRIPTEN)
    add_subdirectory(src/port/wasm)
endif()

if(ES_WITH_JNI OR ANDROID)
    add_subdirectory(src/jni)
    add_subdirectory(src/port/jni)
endif()

if(ES_WITH_TESTS)
    add_subdirectory(test)
endif()

if(ES_WITH_LANG_COMPILER)
    add_subdirectory(tool/lang-compiler)
endif()

if(ES_WITH_TESTS)
    add_subdirectory(lang)
endif()
