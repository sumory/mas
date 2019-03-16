file(GLOB mas_unittest_SRCS *_test.cpp)
include_directories(${PROJECT_SOURCE_DIR}/ /usr/include /usr/local/include)
find_library(GTEST gtest /usr/lib /usr/local/lib)
find_library(GTEST_MAIN gtest_main /usr/lib /usr/local/lib)

add_executable(mas_unittest ${mas_unittest_SRCS})
target_link_libraries(mas_unittest mas_static ${GTEST} ${GTEST_MAIN})

include (CTest)
add_test(NAME mas_unittest COMMAND mas_unittest)
