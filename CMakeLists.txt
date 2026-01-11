cmake_minimum_required(VERSION 3.10)
project(SpinningShapes)

# 1. Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Find Dependencies
# On NixOS, these are found via CMAKE_PREFIX_PATH provided by your devShell
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
# 3. Handle Include Directories
# This ensures #include <glad/glad.h> and #include <glm/glm.hpp> work
include_directories(include)

# 4. Define the Executable
# Note: Ensure glad.c is actually at the path specified below
add_executable(spinning_shapes 
    main.cpp 
    src/glad.c  # Changed from glad/src/glad.c to match standard GLAD output
    # src/imgui.cpp
    # src/imgui_draw.cpp
    # src/imgui_widgets.cpp
    # src/imgui_tables.cpp
    # src/imgui_impl_glfw.cpp
    # src/imgui_impl_opengl3.cpp
)

# 5. Link Libraries
target_link_libraries(spinning_shapes 
    PRIVATE 
    OpenGL::GL 
    glfw        # Linked from find_package(glfw3)
    glm::glm    # Linked from find_package(glm)
    ${CMAKE_DL_LIBS} # Required for GLAD to load GPU drivers at runtime
    imgui
)

# 6. Export Compile Commands
# This is essential for fixing VS Code "squiggles" (IntelliSense)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)