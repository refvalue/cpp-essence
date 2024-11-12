function(es_add_wasm_target target_name)
    set(options WHOLE)
    set(one_value_args EXPORT_NAME)
    set(multi_value_args SOURCES)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_split_list ARG SOURCES)

    add_executable(${target_name} ${headers_and_sources})

    file(
        GLOB private_sources
        CONFIGURE_DEPENDS
        ${ARGN}
        util.hpp
        util.cpp
        ${ARG_SOURCES}
    )

    target_sources(${target_name} PRIVATE ${private_sources})

    target_link_libraries(
        ${target_name}
        PRIVATE
        embind
    )

    target_link_options(
        ${target_name}
        PRIVATE
        --closure=1
        -sMODULARIZE
        -sEXPORT_ES6=1
        -sFILESYSTEM=0
        -sENVIRONMENT=web,worker,node
        -sINCOMING_MODULE_JS_API=
        LINKER:--no-entry
    )

    if(ES_EMBEDDED_WASM)
        target_link_options(
            ${target_name}
            PRIVATE
            -sSINGLE_FILE
        )
    endif()

    if(ARG_EXPORT_NAME)
        target_link_options(
            ${target_name}
            PRIVATE
            -sEXPORT_NAME=${ARG_EXPORT_NAME}
        )
    endif()

    if(ARG_WHOLE)
        target_link_libraries(
            ${target_name}
            PRIVATE
            -Wl,--whole-archive
            cpp-essence
            -Wl,--no-whole-archive
        )
    else()
        target_link_libraries(
            ${target_name}
            PRIVATE
            cpp-essence
        )
    endif()

    set_target_properties(
        ${target_name}
        PROPERTIES
        SUFFIX ".js"
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
endfunction()

es_add_wasm_target(
    cpp-essence-cipher
    EXPORT_NAME createCipherModule
    SOURCES cryptography.cpp
)

es_add_wasm_target(
    cpp-essence-compression
    EXPORT_NAME createCompressionModule
    SOURCES compression.cpp
)
