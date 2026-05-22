add_library(wavecore SHARED)

target_sources(wavecore
    PRIVATE
    mesh/StructuredQuadMesh.cpp)

target_include_directories(wavecore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(wavecore
    PUBLIC
        cxx_std_20
)

wavecore_set_project_warnings(wavecore)
wavecore_enable_sanitizers(wavecore)
