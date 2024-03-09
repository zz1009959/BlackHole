#pragma once
#include"glad/glad.h"
#include<GLFW/glfw3.h>
#include<iostream>
#include<vector>
#include<string>
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);
GLuint createColorTexture(int width, int height, bool hdr);