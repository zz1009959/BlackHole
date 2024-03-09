#version 460 core

uniform sampler2D texture0; // ����������
uniform vec2 resolution; // �ӿڷֱ��ʣ����أ�

const float brightPassThreshold = 1.0; // ������ֵ
const vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721); // ��������

out vec4 fragColor; // �����ɫ

void main()
{
    vec2 texCoord = gl_FragCoord.xy / resolution.xy;

    vec4 c = texture2D(texture0, texCoord);
    float luminance = dot(luminanceVector, c.xyz);// �������ص�����ֵ
    luminance = max(0.0, luminance - brightPassThreshold);// ���㾭��������ֵɸѡ�������ֵ

    c.xyz *= sign(luminance);// �����������ص���ɫ��������������ֵ�������Ծ���
    c.a = 1.0;

    fragColor = c;
}