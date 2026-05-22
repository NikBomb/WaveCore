include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY  https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)

FetchContent_MakeAvailable(doctest)

add_executable(wavecore_tests)

target_sources(wavecore_tests
    PRIVATE
        test_main.cpp
        mesh/test_structured_quad_mesh.cpp
        mesh/test_nodes.cpp 
)

target_link_libraries(wavecore_tests
    PRIVATE
        wavecore
        doctest::doctest

)

wavecore_set_project_warnings(wavecore_tests)
wavecore_enable_sanitizers(wavecore_tests)

add_test(
    NAME wavecore_tests
    COMMAND wavecore_tests
)