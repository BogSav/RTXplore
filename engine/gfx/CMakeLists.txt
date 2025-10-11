message(STATUS "Configuring engine_gfx module")

# Fetch DirectXTK12
include(FetchContent)

FetchContent_Declare(
    directxtk12
    GIT_REPOSITORY https://github.com/microsoft/DirectXTK12.git
    GIT_TAG main
)
FetchContent_MakeAvailable(directxtk12)

# Organize DirectXTK12 targets in a folder
set_target_properties(DirectXTK12 PROPERTIES FOLDER "External/DirectXTK12")
if(TARGET DirectXTK12Audio)
    set_target_properties(DirectXTK12Audio PROPERTIES FOLDER "External/DirectXTK12")
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

# Standardul C++
if(ENGINE_GFX_HEADER_ONLY)
    target_compile_features(engine_gfx INTERFACE cxx_std_20)
else()
    target_compile_features(engine_gfx PUBLIC    cxx_std_20)
endif()