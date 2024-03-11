#version 460 core

in vec2 uv;

out vec4 FragColor;

uniform float tone = 1.0; // ɫ��
uniform float bloomStrength = 0.1; // ����ǿ��

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform vec2 resolution;

void main()
{
    // ����ɫ���ͷ���ǿ�Ȼ�����������������ɫֵ
    FragColor = texture(texture0, uv) * tone + texture(texture1, uv) * bloomStrength;
}