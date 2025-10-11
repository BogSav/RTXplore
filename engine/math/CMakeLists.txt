message(STATUS "Configuring engine_math module")

# Colectare fișiere
file(GLOB_RECURSE MATH_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/math/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/math/*.hpp"
)

file(GLOB_RECURSE MATH_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Alegem tipul de librărie
if(MATH_SOURCES)
    add_library(engine_math STATIC ${MATH_SOURCES} ${MATH_HEADERS})
    set(ENGINE_MATH_HEADER_ONLY OFF)
else()
    add_library(engine_math INTERFACE)
    # Nu e nevoie să treci headerele pe target INTERFACE; le expunem prin include dirs.
    set(ENGINE_MATH_HEADER_ONLY ON)
endif()

# Include directories
if(ENGINE_MATH_HEADER_ONLY)
    target_include_directories(engine_math
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
else()
    target_include_directories(engine_math
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/math
    )
endif()

# Standardul C++
if(ENGINE_MATH_HEADER_ONLY)
    target_compile_features(engine_math INTERFACE cxx_std_20)
else()
    target_compile_features(engine_math PUBLIC    cxx_std_20)
endif()