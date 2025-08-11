#version 330 core

out vec4 FragColor;


uniform vec3 objectColor;
uniform vec3 lightColor;
uniform sampler2D lightTexture;

in vec2 TexCoord;

void main()
{
//FragColor = vec4(lightColor * objectColor,1.0);
//FragColor = vec4(0.8f,0.7f,0.5f,1.0f);
FragColor = vec4(texture(lightTexture,TexCoord));

}
