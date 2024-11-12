set(target_version 1.0.0)
set(target_name cpp-essence-jni)
set(package_name CppEssenceJni)

add_library(${target_name} SHARED)

file(
    GLOB private_sources
    CONFIGURE_DEPENDS
    *.hpp
    *.cpp
)

target_sources(
    ${target_name}
    PRIVATE ${private_sources}
)

message(STATUS "ES_TOOLCHAIN_TARGET_NAME: ${ES_TOOLCHAIN_TARGET_NAME}")

if(NOT "${ES_TOOLCHAIN_TARGET_NAME}" STREQUAL "")
    include(ESCompat)
    es_patch_compat(
        TARGET ${target_name}
        TOOLCHAIN_TARGET_NAME ${ES_TOOLCHAIN_TARGET_NAME}
    )
endif()

target_compile_definitions(${target_name} PRIVATE ES_IS_CPPESSENCEJNI_IMPL=1)

target_include_directories(
    ${target_name}
    PRIVATE
    ${JNI_INCLUDE_DIRS}
)

target_link_libraries(
    ${target_name}
    PRIVATE
    Threads::Threads
    cpp-essence
    cpp-essence-jni-support
    ${JNI_LIBRARIES}
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
    )

    set_target_properties(${target_name}
        PROPERTIES
        CXX_VISIBILITY_PRESET hidden
    )
endif()

include(ESPackaging)

es_make_install_package(
    TARGET_NAME ${target_name}
    PACKAGE_NAME ${package_name}
    TARGET_VERSION ${target_version}
    PATH_VARS CMAKE_INSTALL_PREFIX
)
