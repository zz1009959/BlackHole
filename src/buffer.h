#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include<iostream>

GLuint setCubeVAO();
GLuint setSkyBoxVAO();
void calculateSphere(std::vector<float>& vertices, std::vector<int>& indices, float r, int X_SEGMENTS, int Y_SEGMENTS);
GLuint setSphereVAO();
GLuint setQuadVAO();