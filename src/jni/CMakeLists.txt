set(target_name cpp-essence-jni-support)

add_library(${target_name})
list(APPEND ES_PACKAGING_TARGETS ${target_name})
set(ES_PACKAGING_TARGETS ${ES_PACKAGING_TARGETS} PARENT_SCOPE)

file(
    GLOB miu_sources
    CONFIGURE_DEPENDS
    *.ixx
)

file(
    GLOB internal_piu_sources
    CONFIGURE_DEPENDS
    *.cxx
)

file(
    GLOB private_sources
    CONFIGURE_DEPENDS
    *.cpp
)

target_sources(
    ${target_name}
    PUBLIC
    FILE_SET CXX_MODULES
    FILES ${miu_sources} ${internal_piu_sources}
    PRIVATE
    ${private_sources}
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
    ${ES_PUBLIC_INCLUDE_DIR}
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
    )

    set_target_properties(${target_name}
        PROPERTIES
        CXX_VISIBILITY_PRESET hidden
    )
endif()
