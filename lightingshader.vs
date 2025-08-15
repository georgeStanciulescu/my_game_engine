#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

layout (std140) uniform Perspective
{
    mat4 projection;
    mat4 view;

};

uniform mat4 transform;


out vec3 Normal;
out vec3 Position;


void main()
{
Normal = mat3(transpose(inverse(transform))) * aNormal;
Position = vec3(transform * vec4(aPos+sin(gl_InstanceID),1.0));

gl_Position = projection * view * vec4(Position, 1.0);

}

