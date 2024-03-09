#version 460 core

in vec2 uv;

out vec4 FragColor;

uniform float tone = 1.0; // 色调
uniform float bloomStrength = 0.1; // 泛光强度

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform vec2 resolution;

void main()
{
    // 根据色调和泛光强度混合两个输入纹理的颜色值
    FragColor = texture(texture0, uv) * tone + texture(texture1, uv) * bloomStrength;
}