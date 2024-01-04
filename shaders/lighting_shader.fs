#version 410 core

out vec4 FragColor;
//in vec3 ourColor;
in vec2 TexCoord;

uniform vec3 objectColor;
uniform vec3 lightColor;

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

  FragColor = FragColor * vec4(lightColor * objectColor, 1.0);

  // just the one texture:
  //FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
}