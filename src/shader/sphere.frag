#version 460 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}