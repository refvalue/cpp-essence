enable_testing()
set(target_name cpp-essence-test)

add_executable(${target_name} ${headers_and_sources})
list(APPEND ES_PACKAGING_TARGETS ${target_name})
set(ES_PACKAGING_TARGETS ${ES_PACKAGING_TARGETS} PARENT_SCOPE)

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

target_compile_features(
    ${target_name}
    PRIVATE
    cxx_std_23
)

if(EMSCRIPTEN)
    target_link_libraries(
        ${target_name}
        PRIVATE
        -Wl,--whole-archive
        cpp-essence
        -Wl,--no-whole-archive
    )

    set_target_properties(
        ${target_name}
        PROPERTIES
        SUFFIX ".html"
    )
else()
    target_link_libraries(
        ${target_name}
        PRIVATE
        cpp-essence
        Threads::Threads
        GTest::gtest_main
    )

    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_link_options(
            ${target_name}
            PRIVATE
            LINKER:--exclude-libs,ALL
            $<$<CONFIG:RELEASE>:-s>
        )

        target_link_libraries(
            ${target_name}
            PRIVATE
            atomic
        )
    endif()
endif()

include(GoogleTest)
gtest_discover_tests(${target_name})
