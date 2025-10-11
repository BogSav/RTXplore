add_subdirectory(core)
add_subdirectory(math)
add_subdirectory(platform)
add_subdirectory(gfx)

# TODO: Add support for shader compilation at build time
# Then, the shader module can be added
#add_subdirectory(shaders) 

# Set folder organization for IDE navigation
set_target_properties(engine_core PROPERTIES FOLDER "Engine")
set_target_properties(engine_math PROPERTIES FOLDER "Engine")
set_target_properties(engine_gfx PROPERTIES FOLDER "Engine")
set_target_properties(engine_platform PROPERTIES FOLDER "Engine")

# Create a top-level interface library to aggregate all engine modules
add_library(engine INTERFACE)
target_link_libraries(engine INTERFACE
    engine_core
    engine_math
    engine_platform
    engine_gfx
)
