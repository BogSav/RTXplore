message(STATUS "Configuring sample application")

file(GLOB APP_SRC CONFIGURE_DEPENDS src/*.cpp)

add_executable(sampleapp ${APP_SRC})

target_link_libraries(sampleapp PRIVATE engine)

# Set working directory for debugging
set_target_properties(sampleapp PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)