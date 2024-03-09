#version 460 core

in vec2 uv; // 输入纹理坐标

out vec4 FragColor; // 输出颜色

uniform vec2 resolution; // 分辨率
uniform sampler2D texture0; // 输入颜色纹理

void main()
{
    vec2 inputTexelSize = 1.0 / resolution * 0.5;// 计算输入纹理像素的大小
    vec4 o = inputTexelSize.xyxy * vec4(-1.0, -1.0, 1.0, 1.0); // 计算四个相邻像素的偏移量

    // 对四个相邻像素进行采样，并计算平均值
    FragColor = 0.25 * (texture(texture0, uv + o.xy) + texture(texture0, uv + o.zy) +
                        texture(texture0, uv + o.xw) + texture(texture0, uv + o.zw));
}