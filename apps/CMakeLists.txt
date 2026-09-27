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

add_executable(chiappa_bulk_wave)

target_sources(chiappa_bulk_wave
    PRIVATE
    chiappa_bulk_wave.cpp
)

target_link_libraries(chiappa_bulk_wave
    PRIVATE wavecore
)

wavecore_set_project_warnings(chiappa_bulk_wave)
wavecore_enable_sanitizers(chiappa_bulk_wave)
