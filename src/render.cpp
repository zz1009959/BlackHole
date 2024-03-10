#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <glm/glm.hpp>

#include<iostream>
#include <vector>
#include <cmath>
#include"render.h"

GLuint createQuadVAO()
{
    // 定义矩形的顶点数据
    std::vector<glm::vec3> vertices;
    vertices.push_back(glm::vec3(-1, -1, 0));
    vertices.push_back(glm::vec3(-1, 1, 0));
    vertices.push_back(glm::vec3(1, 1, 0));
    vertices.push_back(glm::vec3(1, 1, 0));
    vertices.push_back(glm::vec3(1, -1, 0));
    vertices.push_back(glm::vec3(-1, -1, 0));

    // 创建VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 创建VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                 &vertices[0], GL_STATIC_DRAW);

    // 1st属性缓冲：位置
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

    return vao; // 返回VAO对象ID
}

GLuint createColorTexture(int width, int height, bool hdr)
{
    GLuint colorTexture;
    glGenTextures(1, &colorTexture);

    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, hdr ? GL_RGB16F : GL_RGB, width, height, 0,
                 GL_RGB, hdr ? GL_FLOAT : GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return colorTexture;
}

GLuint createFramebuffer(const FramebufferCreateInfo& info)
{
    GLuint fbo;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 将颜色附着绑定到帧缓冲
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, info.colorTexture, 0);

    if (info.createDepthBuffer)
    {
        // 创建单个渲染缓冲对象，用于同时存储深度和模板数据
        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, info.width, info.height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    }

    // 检查帧缓冲是否完整
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo;
}

static bool bindToTextureUnit(GLuint program, const std::string& name, GLenum textureType, GLuint texture, int textureUnitIndex)
{
    GLint loc = glGetUniformLocation(program, name.c_str());
    if (loc != -1)
    {
        glUniform1i(loc, textureUnitIndex);

        glActiveTexture(GL_TEXTURE0 + textureUnitIndex);
        glBindTexture(textureType, texture);
        return true;
    }
    else
    {
        std::cout << "WARNING: uniform " << name << " is not found in shader"
            << std::endl;
        return false;
    }
}

void renderToTexture(const RenderToTextureInfo& rtti)
{
    static std::map<GLuint, GLuint> textureFramebufferMap;
    GLuint targetFramebuffer;

    // 如果没有为目标纹理创建帧缓冲区，则创建一个新的帧缓冲区。
    if (!textureFramebufferMap.count(rtti.targetTexture))
    {
        FramebufferCreateInfo createInfo;
        createInfo.colorTexture = rtti.targetTexture;
        targetFramebuffer = createFramebuffer(createInfo);
        textureFramebufferMap[rtti.targetTexture] = targetFramebuffer;
    }
    // 直接获取对应的帧缓冲区 ID。
    else
    {
        targetFramebuffer = textureFramebufferMap[rtti.targetTexture];
    }

    static std::map<std::string, GLuint> shaderProgramMap;
    GLuint program;
    if (!shaderProgramMap.count(rtti.fragShader))
    {
        program = createShaderProgram(rtti.vertexShader, rtti.fragShader);
        shaderProgramMap[rtti.fragShader] = program;
    }
    else
    {
        program = shaderProgramMap[rtti.fragShader];
    }

    {
        glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer); // 绑定帧缓冲
        glViewport(0, 0, rtti.width, rtti.height); // 设置视口
        glDisable(GL_DEPTH_TEST); // 禁用深度测试
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f); // 设置清屏颜色
        glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲
        glUseProgram(program); // 使用着色器程序

        // 设置统一变量
        {
            glUniform2f(glGetUniformLocation(program, "resolution"), (float)rtti.width, (float)rtti.height);
            glUniform1f(glGetUniformLocation(program, "time"), (float)glfwGetTime());

            // 更新浮点统一变量
            for (auto const& [name, val] : rtti.floatUniforms)
            {
                GLint loc = glGetUniformLocation(program, name.c_str());
                if (loc != -1)
                {
                    glUniform1f(loc, val);
                }
                else
                {
                    std::cout << "WARNING: uniform " << name << " is not found" << std::endl;
                }
            }

            // 更新纹理统一变量
            int textureUnit = 0;
            for (auto const& [name, texture] : rtti.textureUniforms)
            {
                bindToTextureUnit(program, name, GL_TEXTURE_2D, texture, textureUnit++);
            }
            for (auto const& [name, texture] : rtti.cubemapUniforms)
            {
                bindToTextureUnit(program, name, GL_TEXTURE_CUBE_MAP, texture, textureUnit++);
            }
        }
        glDrawArrays(GL_TRIANGLES, 0, 6); // 绘制矩形
        glUseProgram(0); // 停用着色器程序
    }
}