add_executable(native_func
        echo.cpp
    )

target_link_libraries(native_func faasm emulator util)
