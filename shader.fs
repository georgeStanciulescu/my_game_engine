#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float linear;
    float quadratic;
    float constant;
};
#define NR_POINT_LIGHTS 8

struct SpotLight {
    vec3 direction;
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float linear;
    float quadratic;
    float constant;
    float cutOff;
    float outerCutOff;
};

uniform Material material;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

vec3 calculateDirLight(DirLight light,vec3 normal,vec3 viewDir);
vec3 calculatePointLight(PointLight light,vec3 normal,vec3 viewDir,vec3 FragPos);
vec3 calculateSpotLight(SpotLight light,vec3 normal,vec3 FragPos);

out vec4 FragColor;
//in vec3 ourColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 objectColor;
uniform vec3 viewPos;
uniform bool hasFlashed;


void main()
{
vec3 objectNormal = normalize(Normal);
// vec3 objectNormal = normalize(material.texture_normal1);
vec3 viewDirection = normalize(viewPos - FragPos);
vec3 resultingLight;

if(!hasFlashed)
{
    resultingLight += calculateDirLight(dirLight,objectNormal,viewDirection);

    for(int i = 0; i <= NR_POINT_LIGHTS;++i)
        resultingLight += calculatePointLight(pointLights[i],objectNormal,viewDirection,FragPos);
}
else
{
    resultingLight += calculateDirLight(dirLight,objectNormal,viewDirection);
    for(int i = 0; i <= NR_POINT_LIGHTS;++i)
            resultingLight += calculatePointLight(pointLights[i],objectNormal,viewDirection,FragPos);
    resultingLight += calculateSpotLight(spotLight,objectNormal,FragPos);
}

FragColor = vec4(resultingLight ,1.0);
//FragColor = texture(material.texture_diffuse1,TexCoord);
}

vec3 calculateDirLight(DirLight light,vec3 normal,vec3 viewDir)
{
    vec3 lightDirection = normalize(-light.direction);
    vec3 reflectionDirection = reflect(-lightDirection,normal);
    float diffuseStrength = max(dot(normal,lightDirection),0.0f);
    float specularStrength = pow(max(dot(viewDir,reflectionDirection),0.0f),material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,TexCoord));
    vec3 diffuse = light.diffuse * vec3(texture(material.texture_diffuse1,TexCoord)) * diffuseStrength;
    vec3 specular = light.specular * vec3(texture(material.texture_specular1,TexCoord)) * specularStrength;

    return (ambient + diffuse + specular);
}

vec3 calculatePointLight(PointLight light,vec3 normal,vec3 viewDir,vec3 FragPos)
{
    vec3 lightDirection = normalize(light.position - FragPos);
    vec3 reflectionDirection = reflect(-lightDirection,normal);

    float distanceLength = length(light.position - FragPos);
    float attenuation = 1.0f/(light.constant + (distanceLength * light.linear) + (distanceLength * distanceLength) * light.quadratic);

    float diffuseStrength = max(dot(normal,lightDirection),0.0f);
    float specularStrength = pow(max(dot(viewDir,reflectionDirection),0.0f),material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,TexCoord)) * attenuation;
    vec3 diffuse = light.diffuse * vec3(texture(material.texture_diffuse1,TexCoord)) * diffuseStrength * attenuation;
    vec3 specular = light.specular * vec3(texture(material.texture_specular1,TexCoord)) * specularStrength * attenuation;

    return (ambient + diffuse + specular);
}

vec3 calculateSpotLight(SpotLight light,vec3 normal,vec3 FragPos)
{
    vec3 lightDirection = normalize(light.position - FragPos);
    vec3 reflectionDirection = reflect(-lightDirection,normal);

    float distanceLength = length(light.position - FragPos);
    float attenuation = 1.0f/(light.constant + (distanceLength * light.linear) + (distanceLength * distanceLength) * light.quadratic);

    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon,0.0,1.0);

    float diffuseStrength = max(dot(normal,lightDirection),0.0f);
    float specularStrength = pow(max(dot(light.position,reflectionDirection),0.0f),material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,TexCoord)) * attenuation * intensity;
    vec3 diffuse = light.diffuse * vec3(texture(material.texture_diffuse1,TexCoord)) * diffuseStrength * attenuation * intensity;
    // for some reason any specularity makes the object very bright
    vec3 specular = light.specular * vec3(texture(material.texture_specular1,TexCoord)) * specularStrength * attenuation * intensity *0.0f;

    return (ambient + diffuse + specular);
}


