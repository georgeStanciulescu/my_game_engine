cmake_minimum_required(VERSION 3.10.0)
project(MYOPENPROJECT)

set(CMAKE_CXX_STANDARD 17)

include_directories(/home/george/OpenGL/include)
link_directories(/home/george/OpenGL/lib)

add_compile_options(-Wall -Wextra -Wconversion -Weffc++)


add_executable(MYOPENPROJECT main.cpp glad.c
        Shader.h
        stb_image.cpp
        Camera.h)

target_link_libraries(MYOPENPROJECT glfw3 GL X11 pthread Xrandr Xi dl)

