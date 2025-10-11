message(STATUS "Configuring engine_platform module")

# Colectare fișiere
file(GLOB_RECURSE PLATFORM_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform/*.hpp"
)

file(GLOB_RECURSE PLATFORM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Alegem tipul de librărie
if(PLATFORM_SOURCES)
    add_library(engine_platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
    set(ENGINE_PLATFORM_HEADER_ONLY OFF)
else()
    add_library(engine_platform INTERFACE)
    # Nu e nevoie să treci headerele pe target INTERFACE; le expunem prin include dirs.
    set(ENGINE_PLATFORM_HEADER_ONLY ON)
endif()

# Include directories
if(ENGINE_PLATFORM_HEADER_ONLY)
    target_include_directories(engine_platform
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
else()
    target_include_directories(engine_platform
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform
    )
endif()

# Link dependencies
target_link_libraries(engine_platform
    PRIVATE 
        engine_gfx
        engine_math
)

# Standardul C++
if(ENGINE_PLATFORM_HEADER_ONLY)
    target_compile_features(engine_platform INTERFACE cxx_std_20)
else()
    target_compile_features(engine_platform PUBLIC    cxx_std_20)
endif()