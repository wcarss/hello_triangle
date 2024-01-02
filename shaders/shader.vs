#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec3 ourColor;
uniform vec3 offset;
out vec2 TexCoord;

void main()
{
  gl_Position = vec4(aPos.x + offset.x, aPos.y + offset.y, aPos.z + offset.z, 1.0);
  ourColor = aColor;
  TexCoord = aTexCoord;
}