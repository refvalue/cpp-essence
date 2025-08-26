set(target_name cpp-essence-lang-compiler)

add_executable(${target_name})
list(APPEND ES_PACKAGING_TARGETS ${target_name})
set(ES_PACKAGING_TARGETS ${ES_PACKAGING_TARGETS} PARENT_SCOPE)

file(
    GLOB private_sources
    CONFIGURE_DEPENDS
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

target_compile_features(
    ${target_name}
    PRIVATE
    cxx_std_23
)

target_link_libraries(
    ${target_name}
    PRIVATE
    cpp-essence
    Threads::Threads
)

if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_link_options(
        ${target_name}
        PRIVATE
        LINKER:--exclude-libs,ALL
        LINKER:-rpath=.
        LINKER:-rpath=\$ORIGIN
        $<$<CONFIG:RELEASE>:-s>
    )

    target_link_libraries(
        ${target_name}
        PRIVATE
        atomic
    )
endif()
