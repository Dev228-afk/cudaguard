find_package(GTest QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(cudaguard_tests
    unit/DiagnosticsTest.cpp
    unit/RuleRegistryTest.cpp
    unit/CommandLineParserTest.cpp
)

target_link_libraries(cudaguard_tests PRIVATE
    cudaguard_core
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cudaguard_tests)
