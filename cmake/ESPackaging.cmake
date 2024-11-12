include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/ESUtil.cmake)

define_property(
    TARGET
    PROPERTY ES_INSTALL_FILES
    BRIEF_DOCS "Associated user-defined .cmake files installed with the target together"
    FULL_DOCS "Syntax: FILES <F1> <F2> ... <Fa> | <F1> <F2> <F3> ... <Fb> | ... DESTINATIONS <D1> <D2> ... <Dn>"
)

function(es_install_include_dirs target_name)
    set(options REQUIRED)
    set(one_value_args "")
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${options}" "${one_value_args}" "${multi_value_args}")

    get_target_property(target_include_dirs ${target_name} INTERFACE_INCLUDE_DIRECTORIES)

    if(ARG_REQUIRED)
        es_ensure_parameters(es_install_include_dirs "" target_include_dirs)
    endif()

    foreach(item IN LISTS target_include_dirs)
        es_split_generator_expression(${item} stage content)

        # Adds include directories without $<INSTALL_INTERFACE:xxx> items.
        if("${stage}" STREQUAL "" OR "${stage}" STREQUAL "BUILD_INTERFACE")
            install(
                DIRECTORY ${content}/
                DESTINATION include
                FILES_MATCHING
                PATTERN "*.h"
                PATTERN "*.hh"
                PATTERN "*.hxx"
                PATTERN "*.hpp"
            )
        endif()
    endforeach()
endfunction()

function(es_install_files target_name)
    set(options INTERFACE)
    set(one_value_args "")
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${options}" "${one_value_args}" "${multi_value_args}")

    set(property_name ES_INSTALL_FILES)

    if(ARG_INTERFACE)
        string(PREPEND property_name "INTERFACE_")
    endif()

    # If not found, the value is xxx-NOTFOUND.
    get_target_property(target_install_files ${target_name} ${property_name})

    if(NOT target_install_files)
        return()
    endif()

    set(options "")
    set(one_value_args "")
    set(multi_value_args FILES DESTINATIONS)
    cmake_parse_arguments(ARG "${options}" "${one_value_args}" "${multi_value_args}" ${target_install_files})
    es_ensure_parameters(es_install_files ARG FILES DESTINATIONS)

    # Parses all file groups.
    es_split_list(
        LIST ARG_FILES
        GLUED
        DELIMITER |
        RESULT_VARIABLE_PREFIX file_group
        RESULT_VARIABLE_COUNT file_group_count
    )

    list(LENGTH ARG_DESTINATIONS destination_count)

    if(NOT file_group_count EQUAL destination_count)
        message(FATAL_ERROR "The \"file_group_count\" (${file_group_count}) must be equal to the \"destination_count\" (${destination_count}).")
    endif()

    math(EXPR file_group_end "${file_group_count} - 1")

    foreach(index RANGE ${file_group_end})
        list(GET ARG_DESTINATIONS ${index} destination)
        install(
            FILES ${file_group_${index}}
            DESTINATION ${destination}
            COMPONENT Devel
        )
    endforeach()
endfunction()

function(es_make_install_package)
    set(options INTERFACE FILE_SET_HEADERS EXECUTABLE)
    set(one_value_args TARGET_NAME PACKAGE_NAME TARGET_VERSION INCLUDE_DIR)
    set(multi_value_args PATH_VARS)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")

    get_target_property(target_type ${ARG_TARGET_NAME} TYPE)
    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][target_type] ${target_type}")

    if("${target_type}" STREQUAL "EXECUTABLE")
        set(target_executable TRUE)
        set(additional_ensuring_parameters "")
    else()
        set(target_executable FALSE)
        set(additional_ensuring_parameters PACKAGE_NAME TARGET_VERSION)
    endif()

    es_ensure_parameters(es_make_install_package ARG TARGET_NAME ${additional_ensuring_parameters})

    # A target version may be 1.2.3 and a so version may be 1 respectively.
    if("${ARG_TARGET_VERSION}" MATCHES [=[^([0-9]+)(\.[0-9]+)?(\.[0-9]+)?$]=])
        set(so_version ${CMAKE_MATCH_1})
    elseif(NOT target_executable)
        message(FATAL_ERROR "Invalid TARGET_VERSION: ${ARG_TARGET_VERSION}")
    endif()

    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][ARG_TARGET_VERSION] ${ARG_TARGET_VERSION}")
    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][so_version] ${so_version}")

    if(NOT ARG_INTERFACE AND NOT target_executable)
        set_target_properties(${ARG_TARGET_NAME}
            PROPERTIES
            VERSION ${ARG_TARGET_VERSION}
            SOVERSION ${so_version}
            INTERFACE_${ARG_PACKAGE_NAME}_MAJOR_VERSION ${so_version}
        )

        set_property(
            TARGET ${ARG_TARGET_NAME}
            APPEND PROPERTY
            COMPATIBLE_INTERFACE_STRING ${ARG_PACKAGE_NAME}_MAJOR_VERSION
        )
    endif()

    if(target_executable)
        set(export_targets "")
    else()
        include(CMakePackageConfigHelpers)

        set(package_config_file ${ARG_PACKAGE_NAME}Config.cmake)
        set(package_version_file ${ARG_PACKAGE_NAME}ConfigVersion.cmake)
        set(package_config_dir lib/cmake/${ARG_PACKAGE_NAME})

        configure_package_config_file(
            ${package_config_file}.in
            ${CMAKE_CURRENT_BINARY_DIR}/${package_config_file}
            INSTALL_DESTINATION ${package_config_dir}
            PATH_VARS ${ARG_PATH_VARS}
        )

        write_basic_package_version_file(
            ${CMAKE_CURRENT_BINARY_DIR}/${package_version_file}
            VERSION ${ARG_TARGET_VERSION}
            COMPATIBILITY AnyNewerVersion
        )

        set(package_targets ${ARG_PACKAGE_NAME}Targets)

        if(ARG_INCLUDE_DIR)
            set(include_dir ${ARG_INCLUDE_DIR})
        else()
            set(include_dir include)
        endif()

        set(export_targets EXPORT ${package_targets})
    endif()

    # Exports the target.
    if(ARG_FILE_SET_HEADERS)
        install(
            TARGETS ${ARG_TARGET_NAME}
            ${export_targets}
            FILE_SET HEADERS
            DESTINATION ${include_dir}
        )
    else()
        install(
            TARGETS ${ARG_TARGET_NAME}
            ${export_targets}
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION ${include_dir}
        )

        es_install_include_dirs(${ARG_TARGET_NAME})
    endif()

    es_install_files(${ARG_TARGET_NAME} ${ARGN})

    # Installs runtime dependencies if vcpkg is enabled.
    if("${CMAKE_TOOLCHAIN_FILE}" MATCHES [=[vcpkg\.cmake$]=])
        message(STATUS "vcpkg is enabled; therefore use x_vcpkg_install_local_dependencies.")
        x_vcpkg_install_local_dependencies(
            TARGETS ${ARG_TARGET_NAME}
            DESTINATION bin
        )
    endif()

    if(NOT target_executable)
        # Installs the package targets file.
        install(
            EXPORT ${package_targets}
            FILE ${package_targets}.cmake
            NAMESPACE ${ARG_PACKAGE_NAME}::
            DESTINATION ${package_config_dir}
        )

        # Installs the config file and the version file of the package.
        install(
            FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${package_config_file}
            ${CMAKE_CURRENT_BINARY_DIR}/${package_version_file}
            DESTINATION ${package_config_dir}
            COMPONENT Devel
        )
    endif()
endfunction()
