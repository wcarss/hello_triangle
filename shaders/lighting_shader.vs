#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
//layout(location = 2) in vec3 aColor;

//out vec3 ourColor;
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  //ourColor = aColor;
  TexCoords = aTexCoord;
  Normal = mat3(transpose(inverse(model))) * aNormal;
  //Normal = aNormal;
  FragPos = vec3(model * vec4(aPos, 1.0));
}