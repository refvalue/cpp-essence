include(ESLangCompiler)

if(ES_WITH_TESTS)
    es_add_lang_resources(
        cpp-essence-test
        NAME cpp_essence_lang
        ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        NAMESPACE essence::misctest::lang
    )
endif()
