cmake_minimum_required(VERSION 3.24)
project(RTXplore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folder organization for IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Global compile definitions
add_compile_definitions(DEBUG)

# For Windows builds, enable UNICODE - this will enable wide-char support in WinAPI
# Currently, the entire engine is using wide-char strings
add_compile_definitions(UNICODE _UNICODE)


# --- DXC/FXC discovery (prefer DXC/SM6, fallback FXC/SM5.1, else FATAL) ---

# User overrides (full paths to executables are OK)
set(RTX_DXC_PATH "" CACHE FILEPATH "Optional path to dxc.exe (DXC shader compiler)")
set(RTX_FXC_PATH "" CACHE FILEPATH "Optional path to fxc.exe (FXC shader compiler)")

# Helper: add a dir to PATH & CMAKE_PROGRAM_PATH for child processes / find_program
# Ensures downstream calls to find_program see freshly discovered compiler folders
function(_rtx_append_env_path dir)
  if(dir AND EXISTS "${dir}")
    set(ENV{PATH} "${dir};$ENV{PATH}")
    list(APPEND CMAKE_PROGRAM_PATH "${dir}")
    set(CMAKE_PROGRAM_PATH "${CMAKE_PROGRAM_PATH}" CACHE PATH "" FORCE)
    message(STATUS "Added to PATH hint: ${dir}")
  endif()
endfunction()

# 0) Respect explicit user paths supplied via cache variables
# Trust the user-specified locations first so they override any auto-detected SDK
if(RTX_DXC_PATH AND EXISTS "${RTX_DXC_PATH}")
  get_filename_component(_dxc_dir "${RTX_DXC_PATH}" DIRECTORY)
  _rtx_append_env_path("${_dxc_dir}")
endif()
if(RTX_FXC_PATH AND EXISTS "${RTX_FXC_PATH}")
  get_filename_component(_fxc_dir "${RTX_FXC_PATH}" DIRECTORY)
  _rtx_append_env_path("${_fxc_dir}")
endif()

# 1) Repo-bundled DXC (external/dxc)
# When the repo vendors DXC, prefer that copy so builds do not depend on system installs
set(_repo_dxc "${CMAKE_SOURCE_DIR}/external/dxc/bin/x64/dxc.exe")
if(EXISTS "${_repo_dxc}")
  get_filename_component(_repo_dxc_dir "${_repo_dxc}" DIRECTORY)
  _rtx_append_env_path("${_repo_dxc_dir}")
endif()

# 2) Windows SDK scan
# Sweep a variety of SDK hints so machines without the repo-bundled compiler still work
if(WIN32)
  set(_sdk_hint_vars
    "$ENV{WindowsSdkBinPath}/x64" "$ENV{WindowsSdkBinPath}"
    "$ENV{WindowsSdkDir}/bin/x64" "$ENV{WindowsSdkDir}/bin")
  # Normalize each hint path and queue it if it exists on disk
  foreach(_cand IN LISTS _sdk_hint_vars)
    file(TO_CMAKE_PATH "${_cand}" _cand_norm)
    if(EXISTS "${_cand_norm}") 
        _rtx_append_env_path("${_cand_norm}") 
    endif()
  endforeach()

  # Registry KitsRoot10/11 → highest bin version
  foreach(_key IN ITEMS "KitsRoot11" "KitsRoot10")
    cmake_host_system_information(RESULT _kr QUERY
      WINDOWS_REGISTRY "HKLM\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots\\${_key}")
    if(_kr AND EXISTS "${_kr}/bin")
  file(GLOB _ver_dirs LIST_DIRECTORIES TRUE "${_kr}/bin/*")
    # Prefer the newest Windows SDK bin directory to find recent shader compilers
    list(SORT _ver_dirs ORDER DESCENDING)
      foreach(_vd IN LISTS _ver_dirs)
        if(EXISTS "${_vd}/x64")
          _rtx_append_env_path("${_vd}/x64")
          break()
        endif()
        if(EXISTS "${_vd}")
          _rtx_append_env_path("${_vd}")
          break()
        endif()
      endforeach()
    endif()
  endforeach()

  # Program Files fallback
  # Collect Program Files locations without embedding parentheses in the foreach expression
  set(_pf_candidates)
  if(DEFINED ENV{ProgramFiles})
    list(APPEND _pf_candidates "$ENV{ProgramFiles}")
  endif()
  if(DEFINED ENV{ProgramFiles\(x86\)})
    list(APPEND _pf_candidates "$ENV{ProgramFiles\(x86\)}")
  endif()

  # Walk both program-files roots and hunt for Windows Kits installations
  foreach(_pf IN LISTS _pf_candidates)
    if(NOT _pf)
      continue()
    endif()
    foreach(_wk IN ITEMS "Windows Kits/11/bin" "Windows Kits/10/bin")
      file(TO_CMAKE_PATH "${_pf}/${_wk}" _wkdir)
      if(EXISTS "${_wkdir}")
    file(GLOB _bins LIST_DIRECTORIES TRUE "${_wkdir}/*")
    # Scan SDK version folders from newest to oldest for an x64 compiler install
    list(SORT _bins ORDER DESCENDING)
        foreach(_b IN LISTS _bins)
          if(EXISTS "${_b}/x64")
            _rtx_append_env_path("${_b}/x64")
            break()
          endif()
        endforeach()
      endif()
    endforeach()
  endforeach()
endif()

# 3) Find programs (prefer DXC)
#   Note: HINTS uses CMAKE_PROGRAM_PATH we’ve accumulated; PATH is also searched.
#   At this point PATH should include any useful SDK or repo dirs discovered above.
find_program(RTX_DXC_EXE NAMES dxc   HINTS ${CMAKE_PROGRAM_PATH})
find_program(RTX_FXC_EXE NAMES fxc   HINTS ${CMAKE_PROGRAM_PATH})

# 4) Decide & export
# Prefer DXC (SM6) when available, otherwise fall back to FXC (SM5.1) or stop the configure
if(RTX_DXC_EXE)
  set(RTX_SHADER_COMPILER "DXC" CACHE STRING "Chosen shader compiler" FORCE)
  set(BUILD_DXIL_SHADERS ON  CACHE BOOL   "Use DXC Shader Model 6 (DXIL)" FORCE)
  message(STATUS "Using DXC (SM6/DXIL): ${RTX_DXC_EXE}")
elseif(RTX_FXC_EXE)
  set(RTX_SHADER_COMPILER "FXC" CACHE STRING "Chosen shader compiler" FORCE)
  set(BUILD_DXIL_SHADERS OFF CACHE BOOL   "Use DXC Shader Model 6 (DXIL)" FORCE)
  message(STATUS "DXC not found; using FXC (SM5.1/DXBC): ${RTX_FXC_EXE}")
else()
  message(FATAL_ERROR
    "No shader compiler found.\n"
    " - Searched for dxc.exe and fxc.exe on PATH/SDK/externals.\n"
    " - Provide RTX_DXC_PATH or RTX_FXC_PATH, install the Windows SDK (which ships FXC),\n"
    "   or vendor DXC into external/dxc/bin/x64.\n")
endif()

# Cache the resolved absolute paths
if(RTX_DXC_EXE)
  set(RTX_DXC_EXE "${RTX_DXC_EXE}" CACHE FILEPATH "Resolved dxc.exe" FORCE)
endif()
if(RTX_FXC_EXE)
  set(RTX_FXC_EXE "${RTX_FXC_EXE}" CACHE FILEPATH "Resolved fxc.exe" FORCE)
endif()

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
