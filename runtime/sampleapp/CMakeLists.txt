message(STATUS "Configuring sandbox application")

file(GLOB APP_SRC CONFIGURE_DEPENDS src/*.cpp)

add_executable(sampleapp ${APP_SRC})

target_link_libraries(sampleapp PRIVATE engine)

# Define the shader and texture directory paths
target_compile_definitions(sampleapp PRIVATE 
    SHADER_DIR="${CMAKE_SOURCE_DIR}/engine/shaders/"
    TEXTURE_DIR="${CMAKE_SOURCE_DIR}/assets/processed/"
)

# Set working directory for debugging
set_target_properties(sampleapp PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)