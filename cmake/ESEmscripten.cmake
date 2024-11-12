include_guard()

function(es_check_emscripten c_result cxx_result root_dir)
    string(TOLOWER "${CMAKE_C_COMPILER}" lcase_c_compiler)
    string(TOLOWER "${CMAKE_CXX_COMPILER}" lcase_cxx_compiler)
    get_filename_component(lcase_c_compiler_name "${lcase_c_compiler}" NAME_WE)
    get_filename_component(lcase_cxx_compiler_name "${lcase_cxx_compiler}" NAME_WE)

    set(compiler_list "")

    if(lcase_c_compiler_name STREQUAL "emcc")
        set(use_emcc TRUE)
        list(APPEND compiler_list ${CMAKE_C_COMPILER})
    else()
        set(use_emcc FALSE)
    endif()

    if(lcase_cxx_compiler_name STREQUAL "em++")
        set(use_emxx TRUE)
        list(APPEND compiler_list ${CMAKE_CXX_COMPILER})
    else()
        set(use_emxx FALSE)
    endif()

    set(${c_result} ${use_emcc} PARENT_SCOPE)
    set(${cxx_result} ${use_emxx} PARENT_SCOPE)

    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][use_emcc] ${use_emcc}")
    message(STATUS "[${CMAKE_CURRENT_FUNCTION}][use_emxx] ${use_emxx}")

    foreach(item IN LISTS compiler_list)
        # Expands the CMAKE_CXX_COMPILER to its full path (e.g. em++ -> /path/to/em++).
        find_program(compiler_full_path ${item})

        if(compiler_full_path)
            get_filename_component(compiler_dir ${compiler_full_path} DIRECTORY)
            set(${root_dir} ${compiler_dir} PARENT_SCOPE)
            message(STATUS "[${CMAKE_CURRENT_FUNCTION}][root_dir] ${compiler_dir}")
            break()
        endif()
    endforeach()
endfunction()

function(es_has_emscripten result)
    es_check_emscripten(use_emcc use_emxx root_dir)

    if(use_emcc OR use_emxx)
        set(${result} TRUE PARENT_SCOPE)
    else()
        set(${result} FALSE PARENT_SCOPE)
    endif()
endfunction()
