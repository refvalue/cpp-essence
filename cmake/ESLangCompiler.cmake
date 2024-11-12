include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/ESUtil.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeRC.cmake)

# CMAKE_CURRENT_LIST_DIR will change within functions (with a dynamic scope).
set(_es_lang_compiler_absolute_current_dir ${CMAKE_CURRENT_LIST_DIR})

function(es_deploy_lang_compiler base_uri)
    include(FetchContent)

    string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} lower_case_host_system_name)
    string(TOLOWER ${CMAKE_HOST_SYSTEM_PROCESSOR} lower_case_host_processor)

    set(file_name cpp-essence-lang-compiler)
    set(component_name ${file_name}-bin)

    FetchContent_Declare(
        ${component_name}
        URL "${base_uri}/${file_name}-${lower_case_host_system_name}-${lower_case_host_processor}.tgz"
    )

    FetchContent_MakeAvailable(${component_name})
    set(compiler_executable ${CMAKE_BINARY_DIR}/_deps/${component_name}-src/${file_name}${CMAKE_EXECUTABLE_SUFFIX})

    if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        string(APPEND compiler_executable ".sh")
    endif()

    set(ES_LANG_COMPILER_EXECUTABLE ${compiler_executable} PARENT_SCOPE)
endfunction()

function(es_add_lang_resources target_name)
    if("${ES_LANG_COMPILER_EXECUTABLE}" STREQUAL "")
        message(FATAL_ERROR "The variable ES_LANG_COMPILER_EXECUTABLE does not exist. Please invoke es_deploy_lang_compiler first.")
    endif()

    set(options PUBLIC INTERFACE PRIVATE)
    set(one_value_args NAME ROOT_DIRECTORY NAMESPACE)
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_execute_process ARG NAME ROOT_DIRECTORY NAMESPACE)

    set(cmrc_target_name ${target_name}-lang-resources)
    set(output_dir ${CMAKE_BINARY_DIR}/_deps/lang/${target_name})
    set(glob_pattern ${ARG_ROOT_DIRECTORY}/*.json)
    set(header_access_level PRIVATE)

    if(ARG_PUBLIC)
        set(header_access_level PUBLIC)
    endif()

    if(ARG_INTERFACE)
        set(header_access_level INTERFACE)
    endif()

    if(ARG_PRIVATE)
        set(header_access_level PRIVATE)
    endif()

    file(
        GLOB_RECURSE source_files
        CONFIGURE_DEPENDS
        ${glob_pattern}
    )

    file(
        GLOB_RECURSE generated_files
        CONFIGURE_DEPENDS
        RELATIVE ${ARG_ROOT_DIRECTORY}
        ${glob_pattern}
    )

    list(TRANSFORM generated_files REPLACE [=[^(.+)\.json$]=] "${output_dir}/\\1.lang")
    es_dump_list(generated_files)

    cmrc_add_resource_library(
        ${cmrc_target_name}
        NAMESPACE ${ARG_NAMESPACE}
        WHENCE ${output_dir}
        PREFIX lang
        ${generated_files}
    )

    add_custom_command(
        OUTPUT ${generated_files}
        COMMAND ${ES_LANG_COMPILER_EXECUTABLE} . ${output_dir}
        WORKING_DIRECTORY ${ARG_ROOT_DIRECTORY}
        DEPENDS ${source_files}
        VERBATIM
    )

    get_target_property(target_type ${target_name} TYPE)

    if(target_type STREQUAL "SHARED_LIBRARY")
        set(ES_WITH_EXPORTS 1)
    else()
        set(ES_WITH_EXPORTS 0)
    endif()

    # Deploys the additional utility source files.
    set(ES_USER_NAME ${ARG_NAME})
    set(ES_USER_NAMESPACE ${ARG_NAMESPACE})

    set(header_file ${output_dir}/user_globalization_${ES_USER_NAME}.hpp)
    set(source_file ${output_dir}/user_globalization_${ES_USER_NAME}.cpp)

    configure_file(
        ${_es_lang_compiler_absolute_current_dir}/config/user_globalization.hpp.in
        ${header_file}
        @ONLY
    )

    configure_file(
        ${_es_lang_compiler_absolute_current_dir}/config/user_globalization.cpp.in
        ${source_file}
        @ONLY
    )

    target_sources(
        ${target_name}
        PRIVATE
        ${header_file}
        ${source_file}
    )

    if(ES_WITH_EXPORTS)
        target_compile_definitions(
            ${target_name}
            PRIVATE
            ES_IS_${ARG_NAME}_IMPL=1
        )
    endif()

    target_include_directories(
        ${target_name}
        ${header_access_level}
        ${output_dir}
    )

    target_link_libraries(
        ${target_name}
        PRIVATE
        ${cmrc_target_name}
    )
endfunction()

function(es_add_lang_library target_name)
    add_library(${target_name} SHARED)
    es_add_lang_resources(
        ${target_name}
        INTERFACE
        ${ARGN}
    )
endfunction()
