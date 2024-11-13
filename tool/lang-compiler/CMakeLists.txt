set(target_name cpp-essence-lang-compiler)

add_executable(${target_name} ${headers_and_sources})

file(
    GLOB private_sources
    CONFIGURE_DEPENDS
    *.hpp
    *.cpp
)

target_sources(${target_name} PRIVATE ${private_sources})

if(NOT "${ES_TOOLCHAIN_TARGET_NAME}" STREQUAL "")
    message(STATUS "ES_TOOLCHAIN_TARGET_NAME: ${ES_TOOLCHAIN_TARGET_NAME}")

    include(ESCompat)
    es_patch_compat(
        TARGET ${target_name}
        COMPAT_ROOT ${PROJECT_SOURCE_DIR}/src
        TOOLCHAIN_TARGET_NAME ${ES_TOOLCHAIN_TARGET_NAME}
    )
endif()

set_target_properties(
    ${target_name}
    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(
    ${target_name}
    PRIVATE
    cpp-essence
    Threads::Threads
)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_options(
        ${target_name}
        PRIVATE
        LINKER:--exclude-libs,ALL
        LINKER:-rpath=.
        LINKER:-rpath="\$ORIGIN"
        $<$<CONFIG:RELEASE>:-s>
    )

    target_link_libraries(
        ${target_name}
        PRIVATE
        atomic
    )
endif()

include(ESPackaging)

es_make_install_package(
    TARGET_NAME ${target_name}
)
