#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<string>
GLuint createShaderProgram(const std::string& vertexShaderFile,
                           const std::string& fragmentShaderFile);