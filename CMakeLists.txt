cmake_minimum_required(VERSION 3.10)
project(PID_TUNING)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)

include_directories(include)

add_executable(pid_sim
    main.cpp 
    src/glad.c 
    # src/imgui.cpp
    # src/imgui_draw.cpp
    # src/imgui_widgets.cpp
    # src/imgui_tables.cpp
    # src/imgui_impl_glfw.cpp
    # src/imgui_impl_opengl3.cpp
)

target_link_libraries(pid_sim
    PRIVATE 
    OpenGL::GL 
    glfw      
    glm::glm    
    ${CMAKE_DL_LIBS} 
    imgui
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)