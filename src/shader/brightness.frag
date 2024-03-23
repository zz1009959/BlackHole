#version 460 core

uniform sampler2D texture0;
uniform vec2 resolution;

uniform float brightPassThreshold = 1.0; // 亮度阈值
const vec3 luminanceVector = vec3(0.2126, 0.7152, 0.0722); // 亮度向量

out vec4 fragColor; // 输出颜色

void main()
{
    vec2 texCoord = gl_FragCoord.xy / resolution.xy;

    vec4 c = texture2D(texture0, texCoord);
    float luminance = dot(luminanceVector, c.xyz);// 计算像素的亮度值
    luminance = max(0.0, luminance - brightPassThreshold);// 计算经过亮度阈值筛选后的亮度值

    c.xyz *= sign(luminance);// 保留或丢弃像素的颜色分量，根据亮度值的正负性决定
    c.a = 1.0;

    fragColor = c;
}