# RTXplore

This repository contains a real‑time rendering engine built on **DirectX 12** with **DirectX Raytracing (DXR)** support. The goal is to place **rasterization** and **ray tracing** side by side on the same scene, making it easy to compare visual results, performance trade‑offs, and pipeline design choices. The project grew out of my bachelor’s thesis; the full write‑up with theory, derivations, and implementation details is available in `assets/Thesis.pdf` - since it was written a long time ago, it might be a bit outdated.

> **⚠ Disclaimer:** This codebase was originally written as an experimental project (for my bachelor thesis) and is primarily intended for learning and demonstration. It does not represent my current programming skills or best practices. Since Fall 2025, I have started a major refactor to modernize and improve the codebase. For ongoing progress, I even have a dedicated Kanban board with refactoring tasks.

## Overview

The engine renders a demonstrative scene featuring **procedural terrain**, an **ocean surface** with multi‑scale waves (small ripples plus **Gerstner** waves for large swells), a **dynamic skybox**, and **atmospheric fog**. Lighting models mix classic Blinn–Phong shading with microfacet‑style parameters where appropriate. Shadows are produced via **shadow mapping** in raster mode, while reflections/refractions are handled by **environment mapping** (raster) or **recursive rays** (DXR).

A key emphasis is the **resource and synchronization model** on DX12: explicit command lists, descriptor heaps, root signatures, pipeline state objects (PSOs), GPU/CPU fences, and (for DXR) acceleration structures and shader tables. The scene is partitioned into **chunks** with **AABBs** to enable **frustum culling** and efficient draw submission.

## Technology Stack

This graphics engine was built using the following technologies:

- **C++ (modern)** — core engine code, systems, resource management.
- **Windows API** — window creation, message loop, timing, basic input.
- **DirectX 12** — programmable graphics pipeline (IA/VS/PS/…); PSOs, root signatures, RTV/DSV/SRV/UAV, command queues & lists.
- **DXR (DirectX Raytracing)** — raygen/closest‑hit/any‑hit/miss shaders, **BLAS/TLAS** acceleration structures, **shader tables**.
- **HLSL** — shaders for both rasterization and ray tracing stages.
- **CMake** — complete multi‑config setup (VS) and single‑config (Ninja) presets, toolchain discovery.
- **DirectXTK (DirectX Tool Kit)** — helpers for textures, loading assets, math/utilities commonly used in DX12 samples.
- _(Optional)_ **DirectXTex / texassemble** — texture processing and cube map assembly (e.g., skybox DDS files).

> The thesis (`thesis.pdf`) provides the theoretical background for wave modeling, noise synthesis for terrain, and the comparative analysis between raster and ray tracing.

## Features

The goal of the engine is to render a complex scene and let you compare rasterization and ray tracing side by side, using the same materials and scene data.

- **Unified Materials & Dynamic Lighting.** One material model (roughness, transparency, opacity) drives both pipelines. Lighting is dynamic in both rasterization and ray tracing so changes are instantly reflected without re-baking.

- **Skyboxes & Dynamic Environment.** Multiple skyboxes are included. In the raster path, a **dynamic cube map** is captured at runtime and used for real-time reflections and refractions; in the RT path the environment is sampled directly by rays.

- **Water Rendering.**

  - **Ray Tracing:** The water surface is procedural and rendered via an intersection shader. Waves are animated over time (Gerstner-like), so the geometry and shading update smoothly without a prebuilt mesh.
  - **Rasterization:** Water uses the dynamic cube map for accurate reflections and refractions, with small-scale detail from normal/displacement modulation.

- **Shadows.**

  - **Rasterization:** Custom, high‑resolution shadow mapping tuned for crisp contacts and stable results.
  - **Ray Tracing:** Ray‑traced shadows via recursive rays, matching the scene’s dynamic lighting.

- **Procedural Terrain.** Terrain height is generated from **Simplex noise** (with fractal layering), producing large shapes with fine detail. The world is split into chunks and culled by the view frustum to keep draw calls bounded.

- **Resource & Shader Organization.** Clear, explicit resource management for buffers and textures, with predictable updates and transitions. Shaders are organized per stage to keep both pipelines easy to follow.

- **Culling, Batching & Profiling.** Command submission is batched to reduce state changes; lightweight timers report frame phases and key statistics to help spot bottlenecks.

## Graphics Capabilities

### 1. Rasterization Pipeline

The raster pipeline follows IA → VS → (optional GS/Tess) → Raster → PS and is tuned for clarity and stability.

- **High‑resolution shadow mapping.** A custom, high‑resolution shadow map is rendered from the light’s perspective; the main pass then resolves visibility with a small PCF kernel for crisp yet stable contacts.

See implementation in:

- `engine/gfx/src/ShadowMap.cpp` (setup and rendering)
- `engine/gfx/include/engine/gfx/ShadowMap.hpp` (API)
- `engine/shaders/Shadow.hlsl` (sampling, filtering)

- **Dynamic cube map for reflections/refractions.** Each frame (or at a chosen cadence), the scene is rendered into a cubemap from the probe’s position; the pixel shader then samples this cube map using reflection/refraction vectors.

See implementation in:

- `engine/gfx/src/DynamicCubeMap.cpp` and `engine/gfx/include/engine/gfx/DynamicCubeMap.hpp` (capture)
- `engine/gfx/src/SkyBoxRenderer.cpp` and `engine/gfx/include/engine/gfx/SkyBoxRenderer.hpp` (skybox)
- `engine/shaders/Default.hlsl` and `engine/shaders/RasterizationHelper.hlsl` (reflection/refraction sampling)

- **Water (raster).** Water uses the dynamic cube map for real‑time reflections and refractions and applies normal/displacement detail for small‑scale waves.

### 2. Ray Tracing Pipeline

The DXR path casts primary rays and, where enabled, recursive rays for reflections, refractions, and hard/soft shadows. BLAS/TLAS acceleration structures organize geometry; the miss shader samples the skybox for the environment.

- **Ray‑traced shadows (shadow rays).** A short ray checks occlusion toward the light; the first hit ends the search to keep it fast.

See implementation in:

- `engine/gfx/src/RayTracingGraphics.cpp` (TLAS/BLAS build, SBT setup)
- `engine/shaders/Default.RT.hlsl` and `engine/shaders/RayTracingHelper.hlsl` (raygen/closest‑hit/miss, shadow rays)

- **Recursive reflections/refractions.** Reflected/refracted rays are traced up to a small max depth for stable, predictable performance.

See implementation in:

- `engine/shaders/Default.RT.hlsl` (recursive reflections/refractions)

- **Procedural water via an intersection shader.** The water surface is not pre‑meshed; the intersection shader evaluates an analytic wave function (Gerstner‑like) and reports hits directly.

See implementation in:

- `engine/gfx/src/WaterRenderer.cpp` and `engine/gfx/include/engine/gfx/WaterRenderer.hpp` (pipeline setup)
- `engine/shaders/Default.RT.hlsl` (intersection/any‑hit for water) and `engine/shaders/Water.hlsl` (raster water)

### 3. Scene Elements

- **Procedural Terrain.** Terrain height is generated via **Simplex noise** with fractal layering, producing large‑scale structure with high‑frequency detail. Chunked LOD via frustum culling keeps draw calls bounded.
- **Ocean Waves.** Large swells are modeled with **Gerstner waves**. Parameters (amplitude, wavelength, direction, steepness) derive tangents/bitangents and normals through analytic partial derivatives, ensuring stable shading and foam cues.
- **Dynamic Skybox.** A rotating cube map provides lighting context. In raster it’s captured into a dynamic cube map for reflections/refractions; in DXR rays sample it directly in the miss shader.
- **Atmospheric Fog.** A linear or smoothstep falloff over view‑space Z softly blends distant geometry into the sky color.

## Build & Run

This repository includes CMake Presets for three common flows on Windows:

- Visual Studio 2022 (generates a solution with the VS toolchain)
- Ninja Multi-Config with Clang
- Ninja Multi-Config with MSVC

The executable target is `runtime/sampleapp`. You can build and run it in `Debug`, `Release`, or `RelWithDebInfo`.

### Prerequisites

- Windows 10/11 with a DirectX 12 GPU (DXR-capable for the ray tracing path)
- CMake 3.23+ with Presets support
- Ninja (for the Ninja flows)
- Visual Studio 2022 (Desktop C++) and/or LLVM/Clang for Windows

### 1) Ninja + Clang (fast, VS Code friendly)

First-time only (configure):

```powershell
cmake --preset ninja-clang
```

Build one of the provided configurations:

```powershell
cmake --build --preset ninja-clang-Debug
cmake --build --preset ninja-clang-Release
cmake --build --preset ninja-clang-RelWithDebInfo
```

Run the app:

```powershell
# Replace <Config> with Debug | Release | RelWithDebInfo
& .\build\ninja-clang\runtime\sampleapp\<Config>\sampleapp.exe
```

### 2) Ninja + MSVC

First-time only (configure):

```powershell
cmake --preset ninja-msvc
```

Build:

```powershell
cmake --build --preset ninja-msvc-Debug
cmake --build --preset ninja-msvc-Release
cmake --build --preset ninja-msvc-RelWithDebInfo
```

Run:

```powershell
& .\build\ninja-msvc\runtime\sampleapp\<Config>\sampleapp.exe
```

### 3) Visual Studio 2022 (solution)

Configure and generate the solution into `build/vs2022`:

```powershell
cmake --preset vs2022
```

Open `build/vs2022/RTXplore.sln` in Visual Studio. Set `runtime/sampleapp` as the Startup Project, choose `Debug` | `Release` | `RelWithDebInfo`, and press F5.

To build from a terminal after configuring:

```powershell
cmake --build .\build\vs2022 --config RelWithDebInfo
```

Run the built executable (example path):

```powershell
& .\build\vs2022\x64\RelWithDebInfo\runtime\sampleapp\sampleapp.exe
```

### VS Code tasks (optional)

For the VSC users I also added `tasks.json` and `launch.json` files for common builds and cleanup commands, that you can simply copy paste into your `.vscode` folder and use like this:

- CMake: Build (RelWithDebInfo) → builds `ninja-clang-RelWithDebInfo`
- CMake: Build (Release) → builds `ninja-clang-Release`
- CMake: Build (Debug) → builds `ninja-clang-Debug`
- CMake: Clean Build Folders → removes `build/*`

Use “Tasks: Run Task…” from the Command Palette to invoke them.

### Notes

- Presets auto-discover DXC/Windows SDK as part of configuration; manual setup is typically unnecessary.
- Assets (DDS) are under `assets/processed`; no conversion step is required at runtime.
- The DirectX Tool kit is automatically downloaded and linked to the `gfx` target when configuring with CMakeWS

## Architecture

The engine is organized into clear modules for maintainability and flexibility:

- **engine/** — Core engine code, split into submodules:

  - **core/**: Platform abstraction, utilities, timing, error handling
  - **gfx/**: Graphics backend, resource management, rendering, DX12/DXR logic
  - **math/**: Math primitives and helpers
  - **platform/**: OS windowing, input, and platform-specific code
  - **shaders/**: All HLSL shaders for both rasterization and ray tracing

- **runtime/** — Contains the main application (`sampleapp`) that links the engine and demonstrates its features. This is the entry point for running the engine as a standalone app.

- **external/** — Third-party dependencies (e.g., DirectXTK, dxc)

- **cmake/** — CMake helper scripts and custom presets for toolchain and build configuration

- **assets/**

  - **processed/**: Ready-to-use textures (DDS), skyboxes, and other binary assets
  - **raw/**: Source textures and skyboxes before conversion

- **tools/** — Scripts and utilities for asset conversion (e.g., `ConvertTextures.bat`)

The build system is driven by CMake, with presets for VS 2022, Ninja+Clang, and Ninja+MSVC. Each engine module is its own CMake target, making it easy to extend or use the engine as a library.

The engine’s rendering flow is modular:

1. **Window & Device**: Platform layer creates the window and initializes the DX12 device
2. **Command Queues & Swap Chain**: Gfx module manages command submission and frame presentation
3. **Descriptor Heaps & PSO/Root Sig**: Explicit resource binding and pipeline state setup
4. **Scene Graph & Materials**: Scene is built from entities with transform, mesh, and material components
5. **Render System Split**: Rasterization and DXR paths share the scene graph and materials, but have separate renderers
6. **Per-frame Flow**: Update, cull, record, submit, present
7. **TLAS/BLAS & Shader Table**: For DXR, acceleration structures and shader tables are built/updated as needed

## Project Structure

The repository uses a modular and conventional layout:

```
engine/
  core/         # Platform, utilities, timing, error handling
  gfx/          # Graphics backend, DX12/DXR, resource management
  math/         # Math primitives and helpers
  platform/     # OS windowing, input
  shaders/      # HLSL for rasterization and ray tracing
runtime/
  sampleapp/    # Main application (entry point)
external/       # Third-party dependencies (DirectXTK, dxc, etc.)
cmake/          # CMake helper scripts and custom presets
assets/
  processed/    # DDS textures, skyboxes, ready-to-use assets
  raw/          # Source textures, skyboxes before conversion
tools/          # Asset conversion scripts (e.g., ConvertTextures.bat)
CMakeLists.txt
CMakePresets.json
```

- **engine/**: All reusable engine code, organized by subsystem
- **runtime/sampleapp/**: The main executable project
- **assets/**: All textures, skyboxes, and other binary resources
- **tools/**: Scripts/utilities for asset processing
- **cmake/**: Build system helpers and custom logic
- **external/**: Downloaded or vendored dependencies

## Documentation

- **`assets/Thesis.pdf`** — the complete documentation with theory, derivations (e.g., Gerstner wave equations and partial derivatives for normals), terrain noise generation, and a side‑by‑side comparison between rasterization and ray tracing, including performance measurements and visual artifacts discussion.

---

## License

This project is released under the terms specified in `LICENSE`. For clarifications or special permissions, please contact the author.
