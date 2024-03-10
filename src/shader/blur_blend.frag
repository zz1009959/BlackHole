#version 460 core
out vec4 FragColor;
in vec2 uv;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform vec2 resolution;
uniform float bloomStrength=1.0;
void main()
{
    vec3 hdrColor = texture(scene, uv).rgb;
    vec3 bloomColor = texture(bloomBlur, uv).rgb * bloomStrength;
    hdrColor += bloomColor; // additive blending

    FragColor = vec4(hdrColor, 1.0f);
}