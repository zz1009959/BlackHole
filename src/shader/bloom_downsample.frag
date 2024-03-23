#version 460 core

in vec2 uv;

out vec4 FragColor;

uniform vec2 resolution;
uniform sampler2D texture0;

void main()
{
    vec2 inputTexelSize = 1.0 / resolution * 0.5;// ���������������صĴ�С
    vec4 o = inputTexelSize.xyxy * vec4(-1.0, -1.0, 1.0, 1.0); // �����ĸ��������ص�ƫ����

    // ���ĸ��������ؽ��в�����������ƽ��ֵ
    FragColor = 0.25 * (texture(texture0, uv + o.xy) + texture(texture0, uv + o.zy) +
                        texture(texture0, uv + o.xw) + texture(texture0, uv + o.zw));
}