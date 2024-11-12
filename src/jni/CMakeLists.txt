set(target_version 1.3.0)
set(target_name cpp-essence-jni-support)
set(package_name CppEssenceJniSupport)

add_library(${target_name})

file(
    GLOB private_sources
    CONFIGURE_DEPENDS
    ${ES_PUBLIC_INCLUDE_DIR}/essence/jni/*.hpp
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

target_include_directories(
    ${target_name}
    INTERFACE
    $<BUILD_INTERFACE:${ES_PUBLIC_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${ES_PUBLIC_INCLUDE_DIR}/essence
    ${JNI_INCLUDE_DIRS}
)

target_compile_definitions(${target_name} PRIVATE ES_IS_JNISUPPORT_IMPL=1)

target_link_libraries(
    ${target_name}
    PRIVATE
    Threads::Threads
    cpp-essence
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
