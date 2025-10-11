cmake_minimum_required(VERSION 4.1)
project(RTXplore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folder organization in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Global compile definitions
add_compile_definitions(DEBUG)
add_compile_definitions(UNICODE _UNICODE)

#include(CompilerWarnings)
#include(StaticAnalyzers)
#include(ShaderBuild)  # definește funcțiile pentru DXC

#option(ENGINE_ENABLE_RAYTRACING "Enable DXR features" ON)
#option(ENGINE_UNITY_BUILDS "Enable UNITY builds for speed" OFF)

#add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(runtime)
#add_subdirectory(tests)