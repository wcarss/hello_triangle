#version 410 core

out vec4 FragColor;
//in vec3 ourColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;

struct Light {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform Light light;

struct Material {
  sampler2D emission;
  sampler2D emission_map;
  vec3 ambient;
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

uniform Material material;

void main()
{
  // ambient
  vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;

  // diffuse
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;

  // specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;

  //vec3 lights = ambient + diffuse + specular;

  vec3 emission = texture(material.emission_map, TexCoord).rgb * texture(material.emission, TexCoord).rgb;
  vec3 result = ambient + diffuse + specular + emission;
  FragColor = vec4(result, 1.0);
  //FragColor = vec4(max(lights.r, emission.r), max(lights.g, emission.g), max(lights.b, emission.b), 1.0);
}