add_executable(cymbolic_tests
    test_expr.cpp
)

target_link_libraries(cymbolic_tests
    PRIVATE
        cymbolic
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cymbolic_tests)
