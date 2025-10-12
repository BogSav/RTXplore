message(STATUS "Configuring engine_gfx module")

# Fetch DirectXTK12
include(FetchContent)

# Prefer legacy FXC over DXC for DirectXTK12 shaders by default, unless overridden
# by a top-level detection or user cache setting.
if(NOT DEFINED BUILD_DXIL_SHADERS)
    set(BUILD_DXIL_SHADERS OFF CACHE BOOL "Use DXC Shader Model 6 for DirectXTK12 shaders")
endif()

FetchContent_Declare(
    directxtk12
    GIT_REPOSITORY https://github.com/microsoft/DirectXTK12.git
    GIT_TAG main
)
FetchContent_MakeAvailable(directxtk12)

set_target_properties(DirectXTK12 PROPERTIES FOLDER "External/DirectXTK12")

# TODO: In future we might want to have audio support through DirectXTK12Audio
# When this happens we can simply link DirectXTK12Audio to engine_gfx
if(TARGET DirectXTK12Audio)
    set_target_properties(DirectXTK12Audio PROPERTIES FOLDER "External/DirectXTK12")
endif()

# Disable compiler warnings for third-party DirectXTK12 targets only
# Since clang is more delicate, it will also display warnings for this library, we need to suppress them.
if(TARGET DirectXTK12)
    target_compile_options(DirectXTK12 PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
        $<$<CXX_COMPILER_ID:Clang>:-w>
        $<$<CXX_COMPILER_ID:GNU>:-w>
    )
endif()
if(TARGET DirectXTK12Audio)
    target_compile_options(DirectXTK12Audio PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
        $<$<CXX_COMPILER_ID:Clang>:-w>
        $<$<CXX_COMPILER_ID:GNU>:-w>
    )
endif()

# Colectare fișiere
file(GLOB_RECURSE GFX_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/gfx/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/gfx/*.hpp"
)

file(GLOB_RECURSE GFX_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Alegem tipul de librărie
if(GFX_SOURCES)
    add_library(engine_gfx STATIC ${GFX_SOURCES} ${GFX_HEADERS})
    set(ENGINE_GFX_HEADER_ONLY OFF)
else()
    add_library(engine_gfx INTERFACE)
    # Nu e nevoie să treci headerele pe target INTERFACE; le expunem prin include dirs.
    set(ENGINE_GFX_HEADER_ONLY ON)
endif()

# Include directories
if(ENGINE_GFX_HEADER_ONLY)
    target_include_directories(engine_gfx
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
else()
    target_include_directories(engine_gfx
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/gfx
    )
endif()

# Link dependencies
target_link_libraries(engine_gfx 
    PRIVATE 
        engine_math
        DirectXTK12
        d3d12.lib
        dxgi.lib
        dxguid.lib
        d3dcompiler.lib
    PUBLIC
        engine_core
)

# Define the shader and texture directory paths
target_compile_definitions(engine_gfx PRIVATE 
    SHADER_DIR="${CMAKE_SOURCE_DIR}/engine/shaders/"
    TEXTURE_DIR="${CMAKE_SOURCE_DIR}/assets/processed/"
)

# Standardul C++
if(ENGINE_GFX_HEADER_ONLY)
    target_compile_features(engine_gfx INTERFACE cxx_std_20)
else()
    target_compile_features(engine_gfx PUBLIC    cxx_std_20)
endif()