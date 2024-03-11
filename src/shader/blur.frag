#version 460 core
out vec4 FragColor;

uniform sampler2D image;
uniform vec2 resolution;
uniform bool horizontal;
float edgeThreshold = 0.1; // ��Ե��ֵ

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 TexCoords = gl_FragCoord.xy / resolution;
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, TexCoords).rgb * weight[0]; // current fragment's contribution

    // ��Ե���
    float edge = length(texture(image, TexCoords + vec2(tex_offset.x, 0.0)).rgb - texture(image, TexCoords - vec2(tex_offset.x, 0.0)).rgb);
    edge += length(texture(image, TexCoords + vec2(0.0, tex_offset.y)).rgb - texture(image, TexCoords - vec2(0.0, tex_offset.y)).rgb);

    if (edge > edgeThreshold)
    {
        FragColor = vec4(texture(image, TexCoords).rgb, 1.0); // ���ֱ�Ե����
    }

    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}