include_guard()

try_compile(
    ES_HAVE_STD_FORMAT
    SOURCES ${CMAKE_SOURCE_DIR}/feature-testing/std_format.cpp
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

message(STATUS "ES_HAVE_STD_FORMAT - ${ES_HAVE_STD_FORMAT}")
