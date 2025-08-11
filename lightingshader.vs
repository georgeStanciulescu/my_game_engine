#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

void main()
{
gl_Position = projection* view * transform * vec4(aPos.x ,aPos.y,aPos.z, 1.0) ; // see how we directly give a vec3 to vec4's constructor

TexCoord = aTex;
}

