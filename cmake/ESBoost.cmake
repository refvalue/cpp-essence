include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/ESUtil.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/ESEmscripten.cmake)

function(es_write_boost_user_config_file)
    set(options "")
    set(one_value_args TOOLSET FILE RESULT_TOOLSET_ABBREVIATION)
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_write_boost_project_jam_file ARG TOOLSET FILE RESULT_TOOLSET_ABBREVIATION)

    get_filename_component(toolset_file_name ${ARG_TOOLSET} NAME_WE)
    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][toolset_file_name] ${toolset_file_name}")

    # arm-none-linux-gnueabihf-gcc --> gcc
    # arm-none-linux-gnueabihf-g++ --> g++
    if(NOT "${toolset_file_name}" MATCHES [=[^(.+-)*(.+)$]=])
        message(FATAL_ERROR "Invalid format of the toolset file name: ${toolset_file_name}.")
    endif()

    set(map_cc gcc)
    set(map_c++ gcc)
    set(map_g++ gcc)
    set(map_clang++ clang)
    set(map_em++ emscripten)

    # For gcc-11, g++-11
    set(map_g++- gcc)
    set(map_gcc- gcc)

    set(toolset_file_first_part ${CMAKE_MATCH_1})
    set(toolset_file_second_part ${CMAKE_MATCH_2})

    # Handles situations like gcc-11 g++-11.
    if(toolset_file_second_part MATCHES [=[[0-9]+]=])
        set(toolset_key ${toolset_file_first_part})
    else()
        set(toolset_key ${toolset_file_second_part})
    endif()

    set(toolset_map_key "map_${toolset_key}")

    if(NOT "${${toolset_map_key}}" STREQUAL "")
        set(toolset_name ${${toolset_map_key}})
    else()
        set(toolset_name ${toolset_key})
    endif()

    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][toolset_name] ${toolset_name}")

    file(WRITE ${ARG_FILE} "using ${toolset_name} : : \"${ARG_TOOLSET}\" : ;")
    set(${ARG_RESULT_TOOLSET_ABBREVIATION} ${toolset_name} PARENT_SCOPE)
endfunction()

function(es_make_boost_impl)
    set(options SYNC_BUILD_TYPE REQUIRED PARALLEL_BUILD STATIC STATIC_RUNTIME)
    set(one_value_args SOURCE_DIR BINARY_DIR INSTALL_DIR TOOLSET TARGET_OS RESULT_FIND_PACKAGE_OPTIONS)
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_make_boost ARG SOURCE_DIR RESULT_FIND_PACKAGE_OPTIONS)

    es_make_default_binary_dir_and_install_dir(
        SOURCE_DIR ${ARG_SOURCE_DIR}
        PREFIX third-party
        RESULT_BINARY_DIR default_binary_dir
        RESULT_INSTALL_DIR default_install_dir
    )

    if(ARG_BINARY_DIR)
        set(binary_dir ${ARG_BINARY_DIR})
    else()
        set(binary_dir ${default_binary_dir})
    endif()

    if(ARG_INSTALL_DIR)
        set(install_dir ${ARG_INSTALL_DIR})
    else()
        set(install_dir ${default_install_dir})
    endif()

    # Replaces CMAKE_EXECUTABLE_SUFFIX with host_executable_suffix here to avoid .js when Emscripten designated.
    set(b2_program ${ARG_SOURCE_DIR}/b2${ES_HOST_EXECUTABLE_SUFFIX})

    # Bootstrap the boost archive if necessary.
    if(NOT EXISTS ${b2_program})
        # Assumes cross-compiling using em++ on Windows.
        if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
            es_execute_process(
                COMMAND bootstrap.bat
                WORKING_DIRECTORY ${ARG_SOURCE_DIR}
            )
        else()
            es_execute_process(
                COMMAND sh bootstrap.sh
                WORKING_DIRECTORY ${ARG_SOURCE_DIR}
            )
        endif()
    endif()

    if(ARG_SYNC_BUILD_TYPE)
        string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
        set(additional_args variant=${build_type})
    else()
        set(additional_args "")
    endif()

    if(ARG_PARALLEL_BUILD)
        es_thread_pool_worker_count(worker_count)
        list(APPEND additional_args -j${worker_count})
    endif()

    if(ARG_STATIC)
        list(APPEND additional_args link=static)
        set(Boost_USE_STATIC_LIBS ON PARENT_SCOPE)
    endif()

    set(cflags "cflags=")
    set(cxxflags "cxxflags=")

    if(ARG_STATIC_RUNTIME)
        list(APPEND additional_args runtime-link=static)
        set(Boost_USE_STATIC_RUNTIME ON PARENT_SCOPE)

        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            string(APPEND cflags "-static-libgcc")
            string(APPEND cxxflags "-static-libgcc")
        endif()
    endif()

    list(APPEND additional_args ${cflags} ${cxxflags})

    if(ARG_TOOLSET)
        set(user_config_file ${binary_dir}/user-config.jam)
        es_write_boost_user_config_file(
            TOOLSET ${ARG_TOOLSET}
            FILE ${user_config_file}
            RESULT_TOOLSET_ABBREVIATION toolset_abbreviation
        )

        list(
            APPEND additional_args
            toolset=${toolset_abbreviation}
            --user-config=${user_config_file}
        )
    endif()

    if(ARG_TARGET_OS)
        list(APPEND additional_args target-os=${ARG_TARGET_OS})
    endif()

    if(EMSCRIPTEN)
        list(APPEND additional_args --disable-icu)
    endif()

    if(NOT EXISTS ${binary_dir})
        file(MAKE_DIRECTORY ${binary_dir})
    endif()

    es_execute_process(
        COMMAND ${b2_program} --prefix=${install_dir} --build-dir=${binary_dir} ${additional_args} install
        WORKING_DIRECTORY ${ARG_SOURCE_DIR}
    )

    if(ARG_REQUIRED)
        set(find_package_options REQUIRED)
    else()
        set(find_package_options "")
    endif()

    set(BOOST_ROOT ${install_dir} PARENT_SCOPE)
    set(${ARG_RESULT_FIND_PACKAGE_OPTIONS} ${find_package_options} PARENT_SCOPE)
endfunction()

macro(es_make_boost)
    es_make_boost_impl(
        ${ARGN}
        RESULT_FIND_PACKAGE_OPTIONS es_make_boost_find_package_options
    )

    message(STATUS "[es_make_boost][Boost_USE_STATIC_RUNTIME] ${Boost_USE_STATIC_RUNTIME}")

    # https://cmake.org/cmake/help/latest/policy/CMP0074.html
    cmake_policy(PUSH)
    cmake_policy(SET CMP0074 NEW)
    es_push_root_path_policy()
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
    find_package(Boost ${es_make_boost_find_package_options})
    es_pop_root_path_policy()
    cmake_policy(POP)

    unset(es_make_openssl_find_package_options)
endmacro()
