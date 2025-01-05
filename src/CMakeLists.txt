add_subdirectory(ui)
add_subdirectory(utils)
add_subdirectory(state)

target_sources("${TARGET_NAME}"
    PRIVATE
        Main.cpp
        )
