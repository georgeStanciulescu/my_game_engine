#version 330 core

out vec4 FragColor;


uniform vec3 objectColor;
uniform vec3 lightColor;
uniform sampler2D lightTexture;


uniform samplerCube skybox;
uniform vec3 cameraPos;


in vec3 Normal;
in vec3 Position;

float airToGlass = 1.00f / 1.52f;
float airToDiamond = 1.00f / 2.42f;
float airToWater = 1.00f / 1.33f;


void main()
{
//FragColor = vec4(lightColor * objectColor,1.0);
//FragColor = vec4(0.8f,0.7f,0.5f,1.0f);
//FragColor = vec4(texture(lightTexture,TexCoord));

vec3 ray = normalize(Position - cameraPos);
vec3 reflectRay = reflect(ray,normalize(Normal));
vec3 refractRay = refract(ray,normalize(Normal),airToGlass);


FragColor = vec4(vec3(0.03f,0.03f,0.03f) + texture(skybox,refractRay).rgb,1.0);



}
