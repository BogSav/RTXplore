add_subdirectory(core)
add_subdirectory(math)
add_subdirectory(platform)
add_subdirectory(gfx)
#add_subdirectory(shaders)

# Set folder organization in IDE
set_target_properties(engine_core PROPERTIES FOLDER "Engine")
set_target_properties(engine_math PROPERTIES FOLDER "Engine")
set_target_properties(engine_gfx PROPERTIES FOLDER "Engine")
set_target_properties(engine_platform PROPERTIES FOLDER "Engine")

add_library(engine INTERFACE)
target_link_libraries(engine INTERFACE
    engine_core
    engine_math
    engine_platform
    engine_gfx
)
