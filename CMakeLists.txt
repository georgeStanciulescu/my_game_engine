cmake_minimum_required(VERSION 3.10.0)
project(MYOPENPROJECT)

set(CMAKE_CXX_STANDARD 23)

include_directories(/home/george/OpenGL/include)
link_directories(/home/george/OpenGL/lib)

add_compile_options(-Wall -Wextra -Wconversion -Weffc++)


add_executable(MYOPENPROJECT main.cpp glad.c
        Shader.h
        stb_image.cpp
        Camera.h
        Mesh.h
        Model.h
        Framebuffer.h
        Globals.h
        UBO.h
        VertexInformation.h
        ArrayBuffer.h
        Input.h
)

target_link_libraries(MYOPENPROJECT glfw3 assimp GL X11 pthread Xrandr Xi dl)

