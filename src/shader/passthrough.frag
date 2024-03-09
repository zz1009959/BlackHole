#version 460 core

out vec4 FragColor;

uniform vec2 resolution;
uniform sampler2D texture0;

void main()
{
  vec2 uv = gl_FragCoord.xy / resolution;
  FragColor = texture(texture0, uv);
}