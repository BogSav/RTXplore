cmake_minimum_required(VERSION 3.24)
project(RTXplore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folder organization for IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Global compile definitions
# You can enable debug features from here, by uncomenting the line below
#add_compile_definitions(_DEBUG)

# For Windows builds, enable UNICODE - this will enable wide-char support in WinAPI
# Currently, the entire engine is using wide-char strings
add_compile_definitions(UNICODE _UNICODE)

# Shader compiler discovery (prefer DXC/SM6, fall back to FXC/SM5.1)
include(utils/ShaderCompilerDiscovery)
rtx_discover_shader_compilers()

# TODO: Enable shader compilation at build time - see comment from engine/CMakeLists.txt
#include(ShaderBuild)

# TODO: Enable compiler warnings and static analyzers
#include(CompilerWarnings)
#include(StaticAnalyzers)

# TODO: Make engine features configurable through options - the most important being:
# - raytracing (DXR)
# - unity builds
#option(ENGINE_ENABLE_RAYTRACING "Enable DXR features" ON)
#option(ENGINE_UNITY_BUILDS "Enable UNITY builds for speed" OFF)

add_subdirectory(engine)
add_subdirectory(runtime)

# TODO: Add unit tests - sometime in the future - first I should make the app stable...
#add_subdirectory(tests)
