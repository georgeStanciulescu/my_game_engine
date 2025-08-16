#version 330 core

layout(triangles) in;
layout (triangle_strip , max_vertices = 3) out;

in TEX_COORDS{
 vec2 TexCoord;
 vec3 ABNORMAL;
 vec3 FragPos;
}tex_in[];

uniform float time;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

vec3 normalCalculator()
{
    vec3 first = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 second = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(first,second));
}

vec4 explode(vec4 position,vec3 normal)
{
    float mag = 2.0f;
    vec3 direction = mag * normal * ((sin(time) + 1.0) /2.0);
    return position + vec4(direction,0.0);
}

void main()
{
    //vec3 normal_gs = normalCalculator();   // use for exploding mesh
    //gl_Position = explode(gl_in[0].gl_Position,normal_gs);

    gl_Position = gl_in[0].gl_Position;
    TexCoord = tex_in[0].TexCoord;
    Normal = tex_in[0].ABNORMAL;
    FragPos = tex_in[0].FragPos;
    EmitVertex();

    //gl_Position = explode(gl_in[1].gl_Position,normal_gs);

    gl_Position = gl_in[1].gl_Position;
    TexCoord = tex_in[1].TexCoord;
    Normal = tex_in[1].ABNORMAL;
    FragPos = tex_in[1].FragPos;
    EmitVertex();

    //gl_Position = explode(gl_in[2].gl_Position,normal_gs);

    gl_Position = gl_in[2].gl_Position;
    TexCoord = tex_in[2].TexCoord;
    Normal = tex_in[2].ABNORMAL;
    FragPos = tex_in[2].FragPos;
    EmitVertex();

    EndPrimitive();
}
