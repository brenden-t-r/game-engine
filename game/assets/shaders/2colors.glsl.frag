#version 330 core

uniform vec4 Color1;
uniform vec4 Color2;
out vec4 FragColor;

void main()
{
  FragColor = Color1;
  FragColor = vec4((Color1 + Color2) * 0.5);
}