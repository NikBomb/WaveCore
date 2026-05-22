function(wavecore_enable_sanitizers target_name)
    if(NOT WAVECORE_ENABLE_SANITIZERS)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target_name}
            PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )

        target_link_options(${target_name}
            PRIVATE
                -fsanitize=address,undefined
        )
    else()
        message(WARNING "Sanitizers are only configured for GCC/Clang-like compilers")
    endif()
endfunction()