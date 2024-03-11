#version 460 core

in vec2 uv; // ��������

out vec4 FragColor;

uniform vec2 resolution;
uniform sampler2D texture0;
uniform sampler2D texture1;

void main() {

    vec2 inputTexelSize = 1.0 / resolution * 0.5;// ���������������صĴ�С
    vec4 o = inputTexelSize.xyxy * vec4(-1.0, -1.0, 1.0, 1.0); // ƫ����

    // ���ĸ��������ؽ��в���������ƽ��ֵ��Ȼ���������1����ɫֵ
    FragColor = 0.25 * (texture(texture0, uv + o.xy) + texture(texture0, uv + o.zy) +
                texture(texture0, uv + o.xw) + texture(texture0, uv + o.zw));

    FragColor += texture(texture1, uv);

    FragColor.a = 1.0;
}