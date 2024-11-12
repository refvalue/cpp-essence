include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/ESUtil.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/ESCCXXRuntime.cmake)

function(es_make_install_third_party_library_impl name)
    set(options SYNC_BUILD_TYPE NO_INHERIT_TOOLCHAIN_FILE PARALLEL_BUILD REQUIRED MODULE DEFAULT_FIND_PACKAGE STATIC_RUNTIME)
    set(one_value_args SOURCE_DIR BINARY_DIR INSTALL_DIR TOOLCHAIN_FILE GENERATOR)
    set(multi_value_args CMAKE_ARGS RESULT_INSTALL_DIR PACKAGE_CONFIG_DIRS RESULT_FIND_PACKAGE_OPTIONS)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_make_third_party_library ARG SOURCE_DIR RESULT_FIND_PACKAGE_OPTIONS)

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

    # Uses an explicit string variable to avoid semicolons from a list.
    set(cmake_args -DCMAKE_INSTALL_PREFIX=${install_dir})

    if(NOT ARG_NO_INHERIT_TOOLCHAIN_FILE AND NOT ARG_TOOLCHAIN_FILE)
        if(CMAKE_TOOLCHAIN_FILE)
            list(APPEND cmake_args -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
        else()
            list(
                APPEND cmake_args
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            )
        endif()
    elseif(ARG_TOOLCHAIN_FILE)
        list(APPEND cmake_args -DCMAKE_TOOLCHAIN_FILE=${ARG_TOOLCHAIN_FILE})
    endif()

    if(ARG_GENERATOR)
        list(APPEND cmake_args -G ${ARG_GENERATOR})
    endif()

    if(ARG_SYNC_BUILD_TYPE)
        list(APPEND cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
    endif()

    if(NOT EMSCRIPTEN)
        if(ARG_STATIC_RUNTIME)
            es_make_c_cxx_runtime_flags(
                STATIC_RUNTIME
                RESULT_DEBUG_FLAGS debug_flags
                RESULT_RELEASE_FLAGS release_flags
                RESULT_MSVC_RUNTIME_LIBRARY msvc_runtime_library
            )

            list(
                APPEND cmake_args
                -DCMAKE_C_FLAGS_DEBUG=${debug_flags}
                -DCMAKE_C_FLAGS_RELEASE=${release_flags}
                -DCMAKE_CXX_FLAGS_DEBUG=${debug_flags}
                -DCMAKE_CXX_FLAGS_RELEASE=${release_flags}
                -DCMAKE_MSVC_RUNTIME_LIBRARY=${msvc_runtime_library}
            )
        endif()
    endif()

    list(
        APPEND cmake_args
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    )

    file(MAKE_DIRECTORY ${binary_dir})

    if(WIN32)
        set(env_vars "")
    else()
        set(env_vars PKG_CONFIG_PATH=$ENV{PKG_CONFIG_PATH})
    endif()

    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][PKG_CONFIG_PATH] $ENV{PKG_CONFIG_PATH}")

    # (Re-)configure the project only when necessary.
    if(NOT EXISTS ${binary_dir}/CMakeCache.txt)
        es_execute_process(
            COMMAND ${CMAKE_COMMAND} ${cmake_args} ${ARG_CMAKE_ARGS} ${ARG_SOURCE_DIR}
            WORKING_DIRECTORY ${binary_dir}
            ${env_vars}
        )
    endif()

    if(ARG_PARALLEL_BUILD)
        es_thread_pool_worker_count(worker_count)
        set(additional_args -j${worker_count})
    else()
        set(additional_args "")
    endif()

    es_execute_process(
        COMMAND ${CMAKE_COMMAND} --build . ${additional_args} --target install
        WORKING_DIRECTORY ${binary_dir}
        ${env_vars}
    )

    if(ARG_RESULT_INSTALL_DIR)
        foreach(item IN LISTS ARG_RESULT_INSTALL_DIR)
            set(${item} ${install_dir} PARENT_SCOPE)
        endforeach()
    endif()

    if(ARG_PACKAGE_CONFIG_DIR)
        set(package_config_dirs ${install_dir}/${ARG_PACKAGE_CONFIG_DIRS})
    else()
        set(package_config_dirs ${install_dir})
    endif()

    if(ARG_REQUIRED)
        set(find_package_options REQUIRED)
    else()
        set(find_package_options)
    endif()

    if(ARG_MODULE)
        list(APPEND find_package_options MODULE)
    endif()

    if(NOT ARG_DEFAULT_FIND_PACKAGE AND NOT ARG_MODULE)
        list(
            APPEND find_package_options
            HINTS ${package_config_dirs}
            NO_DEFAULT_PATH
            CMAKE_FIND_ROOT_PATH_BOTH
        )
    endif()

    set(${ARG_RESULT_FIND_PACKAGE_OPTIONS} ${find_package_options} PARENT_SCOPE)
endfunction()

macro(es_make_install_third_party_library name)
    es_make_install_third_party_library_impl(
        name
        ${ARGN}
        RESULT_FIND_PACKAGE_OPTIONS es_make_install_third_party_library_find_package_options
        RESULT_INSTALL_DIR es_make_install_third_party_library_install_dir
    )

    # https://cmake.org/cmake/help/latest/policy/CMP0074.html
    cmake_policy(PUSH)
    cmake_policy(SET CMP0074 NEW)
    es_push_root_path_policy()
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
    find_package(${name} ${es_make_install_third_party_library_find_package_options})
    es_pop_root_path_policy()
    cmake_policy(POP)

    if(NOT WIN32)
        es_pkg_config_add_path(${es_make_install_third_party_library_install_dir}/lib/pkgconfig)
    endif()

    unset(es_make_install_third_party_library_find_package_options)
    unset(es_make_install_third_party_library_install_dir)
endmacro()
