# For lots of helpful commentary on the commands used this file, look
# at the CMakeLists.txt it was based on:
# https://github.com/juce-framework/JUCE/blob/master/examples/CMake/GuiApp/CMakeLists.txt

cmake_minimum_required(VERSION 3.22)

project(TinyDaw VERSION 0.0.1)

add_subdirectory(tracktion_engine/modules/juce)
add_subdirectory(tracktion_engine/modules)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

juce_add_gui_app(TinyDaw
    PRODUCT_NAME "TinyDaw")

target_sources(TinyDaw
    PRIVATE
        src/Main.cpp
        )

target_compile_definitions(TinyDaw
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:TinyDaw,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:TinyDaw,JUCE_VERSION>"
        JUCE_MODAL_LOOPS_PERMITTED=1
        )

target_link_libraries(TinyDaw
    PRIVATE
        tracktion::tracktion_engine
        juce::juce_core
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
