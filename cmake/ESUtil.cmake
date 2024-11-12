include_guard()

# CMAKE_CURRENT_LIST_DIR will change within functions (with a dynamic scope).
set(_es_util_absolute_current_dir ${CMAKE_CURRENT_LIST_DIR})

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(ES_HOST_EXECUTABLE_SUFFIX ".exe")
else()
    set(ES_HOST_EXECUTABLE_SUFFIX "")
endif()

function(es_split_generator_expression expression stage content)
    # CMake does not allow non-greedy quantifiers.
    if("${expression}" MATCHES [=[^\$\<([^\:]+)\:(.*)\>$]=])
        # Uses set(${var} xxx PARENT_SCOPE) to assign values to function arguments respectively.
        # ${var} may refer to the name of the actual parameter.
        set(${stage} ${CMAKE_MATCH_1} PARENT_SCOPE)
        set(${content} ${CMAKE_MATCH_2} PARENT_SCOPE)
    else()
        set(${stage} "" PARENT_SCOPE)
        set(${content} ${expression} PARENT_SCOPE)
    endif()
endfunction()

macro(es_ensure_parameters name prefix)
    if("${prefix}" STREQUAL "")
        set(final_prefix "")
    else()
        set(final_prefix ${prefix}_)
    endif()

    # ARGN contains all parameters past the explicitly declared parameters.
    # In a macro ARGC ARGV and ARGN is not a variable that needs to be surrounded by ${}.
    foreach(item IN ITEMS ${ARGN})
        if(NOT ${final_prefix}${item})
            message(FATAL_ERROR "[${name}] ${item} cannot be empty.")
        endif()
    endforeach()
endmacro()

function(es_split_list)
    set(options GLUED)
    set(one_value_args LIST DELIMITER RESULT_VARIABLE_PREFIX RESULT_VARIABLE_COUNT)
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_split_list ARG LIST DELIMITER RESULT_VARIABLE_PREFIX RESULT_VARIABLE_COUNT)

    string(LENGTH "${ARG_DELIMITER}" delimiter_length)

    if(NOT delimiter_length EQUAL 1)
        message(FATAL_ERROR "The length of the delimiter must be exactly one.")
    endif()

    set(last_index 0)
    set(result_index 0)
    set(last_item_delimiter FALSE)
    list(LENGTH ${ARG_LIST} list_length)
    math(EXPR list_end "${list_length} - 1")

    if(ARG_GLUED)
        set(match_condition MATCHES "^(\\${ARG_DELIMITER})+$")
    else()
        set(match_condition STREQUAL "${ARG_DELIMITER}")
    endif()

    foreach(index RANGE ${list_end})
        list(GET ${ARG_LIST} ${index} item)

        if("${item}" ${match_condition})
            # Omits all empty groups.   `
            if(NOT last_item_delimiter)
                math(EXPR sub_length "${index} - ${last_index}")
                list(SUBLIST ${ARG_LIST} ${last_index} ${sub_length} result_${result_index})
                set(${ARG_RESULT_VARIABLE_PREFIX}_${result_index} ${result_${result_index}} PARENT_SCOPE)
                math(EXPR result_index "${result_index} + 1")
            endif()

            math(EXPR last_index "${index} + 1")
            set(last_item_delimiter TRUE)
        else()
            set(last_item_delimiter FALSE)
        endif()
    endforeach()

    # Parses the remaining elements.
    if(last_index LESS_EQUAL list_end)
        list(SUBLIST ${ARG_LIST} ${last_index} -1 result_${result_index})
        set(${ARG_RESULT_VARIABLE_PREFIX}_${result_index} ${result_${result_index}} PARENT_SCOPE)
        math(EXPR result_index "${result_index} + 1")
    endif()

    set(${ARG_RESULT_VARIABLE_COUNT} ${result_index} PARENT_SCOPE)
endfunction()

function(es_get_vs_install_dir result)
    get_filename_component(vs_ide_path [=[[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\devenv.exe;]]=] ABSOLUTE BASE_DIR "")
    string(REPLACE [=["]=] "" vs_ide_path ${vs_ide_path})
    get_filename_component(vs_ide_path ${vs_ide_path} DIRECTORY)
    get_filename_component(vs_ide_path ${vs_ide_path} DIRECTORY)
    get_filename_component(vs_ide_path ${vs_ide_path} DIRECTORY)
    set(${result} ${vs_ide_path} PARENT_SCOPE)
endfunction()

function(es_replace_file_name_directory)
    set(options "")
    set(one_value_args FILE DIRECTORY RESULT)
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_replace_file_name_directory ARG FILE DIRECTORY RESULT)

    get_filename_component(file_name ${ARG_FILE} NAME)
    get_filename_component(result ${file_name} ABSOLUTE BASE_DIR ${ARG_DIRECTORY})
    set(${ARG_RESULT} ${result} PARENT_SCOPE)
endfunction()

function(es_make_default_binary_dir_and_install_dir)
    set(options "")
    set(one_value_args SOURCE_DIR PREFIX RESULT_BINARY_DIR RESULT_INSTALL_DIR)
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_replace_file_name_directory ARG PREFIX RESULT_INSTALL_DIR)

    if(ARG_RESULT_BINARY_DIR)
        es_ensure_parameters(es_replace_file_name_directory ARG SOURCE_DIR)
        es_replace_file_name_directory(
            FILE ${ARG_SOURCE_DIR}
            DIRECTORY ${CMAKE_BINARY_DIR}/${ARG_PREFIX}
            RESULT binary_dir
        )

        set(${ARG_RESULT_BINARY_DIR} ${binary_dir} PARENT_SCOPE)
    endif()

    set(${ARG_RESULT_INSTALL_DIR} ${CMAKE_BINARY_DIR}/${ARG_PREFIX}/install PARENT_SCOPE)
endfunction()

function(es_execute_process)
    set(options "")
    set(one_value_args WORKING_DIRECTORY STDOUT_VARIABLE STDERR_VARIABLE)
    set(multi_value_args COMMAND ENVIRONMENT_VARIABLES)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_execute_process ARG COMMAND WORKING_DIRECTORY)

    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][Command] ${ARG_COMMAND}")
    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][Working Directory] ${ARG_WORKING_DIRECTORY}")

    if(ARG_ENVIRONMENT_VARIABLES)
        message(STATUS "[${CMAKE_CURRENT_FUNCTION}][Environment Variables] ${ARG_ENVIRONMENT_VARIABLES}")
        set(additional_args COMMAND ${CMAKE_COMMAND} -E env ${ARG_ENVIRONMENT_VARIABLES} ${ARG_COMMAND})
    else()
        set(additional_args COMMAND ${ARG_COMMAND})
    endif()

    if(ARG_STDOUT_VARIABLE)
        list(APPEND additional_args OUTPUT_VARIABLE stdout_variable)
    endif()

    if(ARG_STDERR_VARIABLE)
        list(APPEND additional_args ERROR_VARIABLE stderr_variable)
    endif()

    execute_process(
        WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
        RESULT_VARIABLE exit_code
        ${additional_args}
    )

    set(${ARG_STDOUT_VARIABLE} ${stdout_variable} PARENT_SCOPE)
    set(${ARG_STDERR_VARIABLE} ${stderr_variable} PARENT_SCOPE)

    if(NOT exit_code EQUAL 0)
        message(FATAL_ERROR "\"${ARG_COMMAND}\" failed with exit code ${exit_code}.")
    endif()
endfunction()

function(es_find_nmake)
    es_execute_process(
        COMMAND ${CMAKE_COMMAND} -G "Visual Studio 17 2022" -P fragments/FindNMake.cmake
        WORKING_DIRECTORY ${_es_util_absolute_current_dir}
        STDOUT_VARIABLE output_text
    )
endfunction()

function(es_push_root_path_policy)
    set(
        local_list
        ${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
        ${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
        ${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
        ${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}
    )

    set(${CMAKE_CURRENT_FUNCTION}_root_path_list ${local_list} PARENT_SCOPE)

    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][PUSH LIST] ${local_list}")
endfunction()

function(es_pop_root_path_policy_impl local_list)
    if(${local_list})
        list(POP_BACK ${local_list} CMAKE_FIND_ROOT_PATH_MODE_PACKAGE)
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE} PARENT_SCOPE)
        message(STATUS "[${CMAKE_CURRENT_FUNCTION}][POP] ${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}")

        list(POP_BACK ${local_list} CMAKE_FIND_ROOT_PATH_MODE_INCLUDE)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE} PARENT_SCOPE)
        message(STATUS "[${CMAKE_CURRENT_FUNCTION}][POP] ${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}")

        list(POP_BACK ${local_list} CMAKE_FIND_ROOT_PATH_MODE_LIBRARY)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY} PARENT_SCOPE)
        message(STATUS "[${CMAKE_CURRENT_FUNCTION}][POP] ${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}")

        list(POP_BACK ${local_list} CMAKE_FIND_ROOT_PATH_MODE_PROGRAM)
        set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM} PARENT_SCOPE)
        message(STATUS "[${CMAKE_CURRENT_FUNCTION}][POP] ${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}")

        unset(${local_list} PARENT_SCOPE)
    endif()
endfunction()

macro(es_pop_root_path_policy)
    es_pop_root_path_policy_impl(es_push_root_path_policy_root_path_list)
endmacro()

macro(es_find_package_no_root_path)
    es_push_root_path_policy()
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
    find_package(${ARGN})
    es_pop_root_path_policy()
endmacro()

function(es_dump_list)
    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][Lists] ${ARGN}")
    foreach(item IN LISTS ${ARGN})
        message(STATUS [${CMAKE_CURRENT_FUNCTION}][Value] ${item})
    endforeach()
endfunction()

function(es_pkg_config_add_path)
    set(pkg_config_path $ENV{PKG_CONFIG_PATH})
    list(INSERT pkg_config_path 0 ${ARGN})
    list(REMOVE_DUPLICATES pkg_config_path)
    set(ENV{PKG_CONFIG_PATH} "${pkg_config_path}")

    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][pkg_config_path] ${pkg_config_path}")
endfunction()

function(es_thread_pool_worker_count result)
    include(ProcessorCount)
    ProcessorCount(processor_count)
    math(EXPR worker_count "${processor_count} * 2 + 1")
    set(${result} ${worker_count} PARENT_SCOPE)
endfunction()

function(es_datetime result)
    if(WIN32)
        set(command cmd.exe /c "echo %date:~0,4%/%date:~5,2%/%date:~8,2% %time:~0,2%:%time:~3,2%:%time:~6,2%")
    else()
        set(command date)
    endif()

    execute_process(
        COMMAND ${command}
        OUTPUT_VARIABLE current_date
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(${result} ${current_date} PARENT_SCOPE)
endfunction()
