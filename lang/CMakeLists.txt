include(ESLangCompiler)

if(ES_WITH_TESTS)
    es_add_lang_resources(
        cpp-essence-test
        NAME essence.unit_test.lang
        ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        NAMESPACE essence::misctest::lang
    )
endif()
