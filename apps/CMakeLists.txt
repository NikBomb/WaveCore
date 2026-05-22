add_executable(wavecore_app)

target_sources(wavecore_app
    PRIVATE
    wavecore.cpp
)

target_link_libraries(wavecore_app
    PRIVATE wavecore
)

wavecore_set_project_warnings(wavecore_app)
wavecore_enable_sanitizers(wavecore_app)