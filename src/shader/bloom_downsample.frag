#version 460 core

in vec2 uv;

out vec4 FragColor;

uniform vec2 resolution;
uniform sampler2D texture0;

void main()
{
    vec2 inputTexelSize = 1.0 / resolution * 0.5;// 计算输入纹理像素的大小
    vec4 o = inputTexelSize.xyxy * vec4(-1.0, -1.0, 1.0, 1.0); // 计算四个相邻像素的偏移量

    // 对四个相邻像素进行采样，并计算平均值
    FragColor = 0.25 * (texture(texture0, uv + o.xy) + texture(texture0, uv + o.zy) +
                        texture(texture0, uv + o.xw) + texture(texture0, uv + o.zw));
}