include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/ESUtil.cmake)

function(es_target_version_definitions)
    set(options PUBLIC INTERFACE PRIVATE)
    set(one_value_args PREFIX VERSION)
    set(multi_value_args TARGETS)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(${CMAKE_CURRENT_FUNCTION} ARG PREFIX VERSION TARGETS)

    if(ARG_PRIVATE)
        set(visibility PRIVATE)
    elseif(ARG_INTERFACE)
        set(visibility INTERFACE)
    elseif(ARG_PUBLIC)
        set(visibility PUBLIC)
    else()
        message(FATAL_ERROR "One of PUBLIC, INTERFACE and PRIVATE must be set.")
    endif()

    if(NOT "${ARG_PREFIX}" MATCHES [=[^[A-Za-z][0-9A-Za-z_]*$]=])
        message(FATAL_ERROR "Invalid PREFIX: ${ARG_PREFIX}")
    endif()

    es_datetime(datetime)

    foreach(item IN LISTS ARG_TARGETS)
        message(STATUS "[Function]${CMAKE_CURRENT_FUNCTION} [Target]${item} [Datetime]${datetime}")
        target_compile_definitions(
            ${item}
            ${visibility}
            ${ARG_PREFIX}_VERSION="${ARG_VERSION}"
            ${ARG_PREFIX}_DATETIME="${datetime}"
        )
    endforeach()
endfunction()
