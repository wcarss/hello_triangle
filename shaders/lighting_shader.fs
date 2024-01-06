#version 410 core

out vec4 FragColor;
//in vec3 ourColor;
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;

struct PointLight {
  vec3 pos;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};
#define MAX_NUM_OF_LIGHTS 100
uniform PointLight pointLights[MAX_NUM_OF_LIGHTS];

struct DirLight {
  vec3 dir;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform DirLight dirLight;

struct Material {
  sampler2D emission;
  sampler2D emission_map;
  vec3 ambient;
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

uniform Material material;
uniform int lightsUsed;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
  // properties
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  // phase 1: Directional lighting
  vec3 result = CalcDirLight(dirLight, norm, viewDir);

  // phase 2: Point lights
  for (int i = 0; i < lightsUsed; i++) {
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
  }

  result += texture(material.emission_map, TexCoords).rgb * texture(material.emission, TexCoords).rgb;

  // phase 3: Spot light
  //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

  FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.dir);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  // combine results
  vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
  vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
  vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
  return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.pos - fragPos);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  // attenuation
  float distance    = length(light.pos - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  // combine results
  vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
  vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
  vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;
  return (ambient + diffuse + specular);
}
