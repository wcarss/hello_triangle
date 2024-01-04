#version 410 core

out vec4 FragColor;
//in vec3 ourColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform float textureSwap;
uniform int texSelect;

void main()
{
  if (texSelect == 0) {
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), textureSwap);
  } else if (texSelect == 1) {
    FragColor = texture(texture1, TexCoord);
  } else if (texSelect == 2) {
    FragColor = texture(texture2, TexCoord);
  } else if (texSelect == 3) {
    FragColor = texture(texture3, TexCoord);
  } else if (texSelect == 4) {
    FragColor = texture(texture4, TexCoord);
  }

  // ambient
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColor;

  // diffuse
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // specular
  float specularStrength = 0.5;
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
  vec3 specular = specularStrength * spec * lightColor;

  vec3 result = (ambient + diffuse + specular) * objectColor;

  FragColor = FragColor * vec4(result, 1.0);

  // just the one texture:
  //FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
}