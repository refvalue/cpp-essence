include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/ESUtil.cmake)

# CMAKE_CURRENT_LIST_DIR will change within functions (with a dynamic scope).
set(_es_packaging_absolute_current_dir ${CMAKE_CURRENT_LIST_DIR})

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
    set(property_name ES_INSTALL_FILES)
    get_target_property(target_type ${target_name} TYPE)

    if("${target_type}" STREQUAL "INTERFACE_LIBRARY")
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

function(es_set_target_versions)
    set(options "")
    set(one_value_args VERSION RESULT_VARIABLE_HAS_LIBRARY RESULT_VARIABLE_LIBRARY_TARGETS RESULT_VARIABLE_NON_LIBRARY_TARGETS)
    set(multi_value_args TARGETS)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_make_install_package ARG TARGETS VERSION)

    # A target version may be 1.2.3 and a so version may be 1 respectively.
    if("${ARG_VERSION}" MATCHES [=[^([0-9]+)(\.[0-9]+)?(\.[0-9]+)?$]=])
        set(so_version ${CMAKE_MATCH_1})
    else()
        message(FATAL_ERROR "Invalid VERSION: ${ARG_VERSION}")
    endif()

    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][ARG_VERSION] ${ARG_VERSION}")
    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][so_version] ${so_version}")

    if(ARG_RESULT_VARIABLE_HAS_LIBRARY)
        set(${ARG_RESULT_VARIABLE_HAS_LIBRARY} FALSE PARENT_SCOPE)
    endif()

    set(library_targets "")

    foreach(item IN LISTS ARG_TARGETS)
        get_target_property(target_type ${item} TYPE)

        if(ARG_RESULT_VARIABLE_HAS_LIBRARY AND NOT "${target_type}" STREQUAL "EXECUTABLE")
            list(APPEND library_targets ${item})
            set(${ARG_RESULT_VARIABLE_HAS_LIBRARY} TRUE PARENT_SCOPE)
        endif()

        if("${target_type}" STREQUAL "SHARED_LIBRARY")
            set_target_properties(
                ${item}
                PROPERTIES
                VERSION ${ARG_VERSION}
                SOVERSION ${so_version}
            )
        endif()

        set_target_properties(
            ${item}
            PROPERTIES
            INTERFACE_${item}_MAJOR_VERSION ${so_version}
        )

        set_property(
            TARGET ${item}
            APPEND PROPERTY
            COMPATIBLE_INTERFACE_STRING ${item}_MAJOR_VERSION
        )
    endforeach()

    if(ARG_RESULT_VARIABLE_LIBRARY_TARGETS)
        set(${ARG_RESULT_VARIABLE_LIBRARY_TARGETS} ${library_targets} PARENT_SCOPE)
    endif()

    if(ARG_RESULT_VARIABLE_NON_LIBRARY_TARGETS)
        list(REMOVE_ITEM ARG_TARGETS ${library_targets})
        set(${ARG_RESULT_VARIABLE_NON_LIBRARY_TARGETS} ${ARG_TARGETS} PARENT_SCOPE)
    endif()
endfunction()

function(es_make_install_package)
    set(options "")
    set(one_value_args PACKAGE_NAME VERSION INCLUDE_DIR)
    set(multi_value_args TARGETS PATH_VARS)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    es_ensure_parameters(es_make_install_package ARG TARGETS VERSION)

    es_set_target_versions(
        TARGETS ${ARG_TARGETS}
        VERSION ${ARG_VERSION}
        RESULT_VARIABLE_HAS_LIBRARY has_library_target
        RESULT_VARIABLE_LIBRARY_TARGETS library_targets
        RESULT_VARIABLE_NON_LIBRARY_TARGETS non_library_targets
    )

    if(has_library_target)
        es_ensure_parameters(es_make_install_package ARG PACKAGE_NAME)
    endif()

    if(has_library_target)
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
            VERSION ${ARG_VERSION}
            COMPATIBILITY AnyNewerVersion
        )

        set(package_targets ${ARG_PACKAGE_NAME}Targets)
    endif()

    if(ARG_INCLUDE_DIR)
        set(include_dir ${ARG_INCLUDE_DIR})
    else()
        set(include_dir include)
    endif()

    set(miu_dir lib/miu)
    install(CODE "file(MAKE_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/${include_dir}\")")


    # Exports library targets.
    install(
        TARGETS ${library_targets}
        EXPORT ${package_targets}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION ${include_dir}

        FILE_SET HEADERS
        DESTINATION ${include_dir}

        FILE_SET CXX_MODULES
        DESTINATION ${miu_dir}
    )

    # Exports non-library targets.
    install(
        TARGETS ${non_library_targets}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION ${include_dir}
    )

    foreach(item IN LISTS ARG_TARGETS)
        es_install_include_dirs(${item})
        es_install_files(${item} ${ARGN})
    endforeach()

    # Installs runtime dependencies if vcpkg is enabled.
    if("${CMAKE_TOOLCHAIN_FILE}" MATCHES [=[vcpkg\.cmake$]=])
        message(STATUS "vcpkg is enabled; therefore use x_vcpkg_install_local_dependencies.")
        x_vcpkg_install_local_dependencies(
            TARGETS ${ARG_TARGETS}
            DESTINATION bin
        )
    endif()

    if(has_library_target)
        # Installs the package targets file.
        install(
            EXPORT ${package_targets}
            FILE ${package_targets}.cmake
            NAMESPACE ${ARG_PACKAGE_NAME}::
            DESTINATION ${package_config_dir}
            CXX_MODULES_DIRECTORY ${miu_dir}
        )

        # Installs the config file and the version file of the package.
        install(
            FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${package_config_file}
            ${CMAKE_CURRENT_BINARY_DIR}/${package_version_file}
            DESTINATION ${package_config_dir}
            COMPONENT Devel
        )

        # Deletes non-miu sources that have been exported by the installation system.
        install(
            CODE " \
            file(GLOB_RECURSE non_miu_sources \"\${CMAKE_INSTALL_PREFIX}/${miu_dir}/*.cpp\" \"\${CMAKE_INSTALL_PREFIX}/${miu_dir}/*.cxx\")\n \
            message(STATUS \"non_miu_sources: \${non_miu_sources}\")\n \
            file(REMOVE \${non_miu_sources})"
        )

        # Removes the non-miu sources and links from the target script file.
        install(
            CODE " \
            set(script_file \"\${CMAKE_INSTALL_PREFIX}/lib/cmake/${ARG_PACKAGE_NAME}/${package_targets}.cmake\")\n \
            execute_process(COMMAND python3 \"${_es_packaging_absolute_current_dir}/py/patch_exported_modular_target.py\" \"\${script_file}\")"
        )
    endif()
endfunction()
