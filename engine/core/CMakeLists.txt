message(STATUS "Configuring engine_core module")

# Colectare fișiere
file(GLOB_RECURSE CORE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/core/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/core/*.hpp"
)

file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Alegem tipul de librărie
if(CORE_HEADERS)
    add_library(engine_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
    set(ENGINE_CORE_HEADER_ONLY OFF)
else()
    add_library(engine_core INTERFACE)
    # Nu e nevoie să treci headerele pe target INTERFACE; le expunem prin include dirs.
    set(ENGINE_CORE_HEADER_ONLY ON)
endif()

# Include directories
if(ENGINE_CORE_HEADER_ONLY)
    target_include_directories(engine_core
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
else()
    target_include_directories(engine_core
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/core
    )
endif()

# Standardul C++
if(ENGINE_CORE_HEADER_ONLY)
    target_compile_features(engine_core INTERFACE cxx_std_20)
else()
    target_compile_features(engine_core PUBLIC    cxx_std_20)
endif()