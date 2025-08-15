#version 330 core

//in vec2 TexCoord;
out vec4 FragColor;

in TEX_COORDS
{
    vec2 TexCoord;
    vec3 ABNORMAL;
    vec3 FragPos;
}tex_coords;

uniform sampler2D grass;

void main()
{
    vec4 texColor = texture(grass,tex_coords.TexCoord);
//     if(texColor.a < 0.1)
//         discard;
    FragColor = texColor;


}
