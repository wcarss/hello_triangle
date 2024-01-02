#version 410 core

out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float textureSwap;

void main()
{
  FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), textureSwap);// * vec4(ourColor, 1.0);
  // just the one texture:
  //FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
}