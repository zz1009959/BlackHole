#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<vector>
#include"shader.h"

#include<iostream>
#include<map>

// 帧缓冲对象的创建信息
struct FramebufferCreateInfo
{
    GLuint colorTexture = 0;  // 颜色纹理附加到帧缓冲对象
    int width = 256;         // 帧缓冲对象的宽度
    int height = 256;        // 帧缓冲对象的高度
    bool createDepthBuffer = false;  // 是否创建深度缓冲
};

// 渲染到纹理的信息
struct RenderToTextureInfo
{
    std::string vertexShader = "./src/shader/simple.vert";  // 顶点着色器程序的文件路径
    std::string fragShader;  // 片段着色器程序的文件路径
    std::map<std::string, float> floatUniforms;  // 浮点型 uniform 变量的名称和值
    std::map<std::string, GLuint> textureUniforms;  // 纹理 uniform 变量的名称和纹理单元
    std::map<std::string, GLuint> cubemapUniforms;  // 立方体贴图 uniform 变量的名称和纹理单元
    GLuint targetTexture;  // 渲染到的目标纹理
    int width;  // 渲染目标纹理的宽度
    int height;  // 渲染目标纹理的高度
};

GLuint createQuadVAO();

GLuint createColorTexture(int width, int height, bool hdr = true);// 创建一个颜色纹理

GLuint createFramebuffer(const FramebufferCreateInfo& info);// 创建帧缓冲对象

void renderToTexture(const RenderToTextureInfo& rtti);// 执行渲染操作将结果存储在目标纹理中
