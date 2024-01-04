#version 410 core
out vec4 FragColor;

struct Light {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform Light light;

void main()
{
  FragColor = vec4(light.specular, 1.0); // set all 4 vector values to 1.0
}
