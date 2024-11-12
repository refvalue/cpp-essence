include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/ESUtil.cmake)

function(es_patch_compat_impl)
    set(options "")
    set(one_value_args TARGET TOOLCHAIN_TARGET_NAME BASE_DIR RESULT_CMAKE_FILE COMPAT_ROOT)
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_patch_compat ARG TARGET TOOLCHAIN_TARGET_NAME RESULT_CMAKE_FILE)

    if(ARG_BASE_DIR)
        set(base_dir ${ARG_BASE_DIR})
    else()
        set(base_dir ${CMAKE_SOURCE_DIR})
    endif()

    string(TOLOWER "${ARG_TOOLCHAIN_TARGET_NAME}" toolchain_target_name)

    if(ARG_COMPAT_ROOT)
        set(compat_root ${ARG_COMPAT_ROOT})
    else()
        set(compat_root ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    set(patch_source_file ${compat_root}/compat/${toolchain_target_name}.cpp)
    set(cmake_file ${base_dir}/cmake/compat/${toolchain_target_name}.cmake)

    if(NOT EXISTS ${patch_source_file})
        message(STATUS "[${CMAKE_CURRENT_FUNCTION}] \"${patch_source_file}\" does not exist; skipping...")
    else()
        target_sources(${ARG_TARGET} PRIVATE ${patch_source_file})
    endif()

    if(NOT EXISTS ${cmake_file})
        message(STATUS "[${CMAKE_CURRENT_FUNCTION}] \"${cmake_file}\" does not exist; skipping...")
        set(cmake_file "")
    endif()

    set(${ARG_RESULT_CMAKE_FILE} ${cmake_file} PARENT_SCOPE)
endfunction()

macro(es_patch_compat)
    es_patch_compat_impl(
        ${ARGN}
        RESULT_CMAKE_FILE es_patch_compat_cmake_file
    )

    if(es_patch_compat_cmake_file)
        message(STATUS "[es_patch_compat][${es_patch_compat_cmake_file}] Patching...")
        include(${es_patch_compat_cmake_file})
    endif()

    unset(es_patch_compat_cmake_file)
endmacro()
