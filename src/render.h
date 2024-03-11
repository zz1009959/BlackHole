#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<vector>
#include"shader.h"

#include<iostream>
#include<map>

// ֡�������Ĵ�����Ϣ
struct FramebufferCreateInfo
{
    GLuint colorTexture = 0;  // ��ɫ�����ӵ�֡�������
    int width = 256;         // ֡�������Ŀ��
    int height = 256;        // ֡�������ĸ߶�
    bool createDepthBuffer = false;  // �Ƿ񴴽���Ȼ���
};

// ��Ⱦ���������Ϣ
struct RenderToTextureInfo
{
    std::string vertexShader = "./src/shader/simple.vert";  // ������ɫ��������ļ�·��
    std::string fragShader;  // Ƭ����ɫ��������ļ�·��
    std::map<std::string, float> floatUniforms;  // ������ uniform ���������ƺ�ֵ
    std::map<std::string, GLuint> textureUniforms;  // ���� uniform ���������ƺ�����Ԫ
    std::map<std::string, GLuint> cubemapUniforms;  // ��������ͼ uniform ���������ƺ�����Ԫ
    GLuint targetTexture;  // ��Ⱦ����Ŀ������
    int width;  // ��ȾĿ������Ŀ��
    int height;  // ��ȾĿ������ĸ߶�
};

GLuint createQuadVAO();

GLuint createColorTexture(int width, int height, bool hdr = true);// ����һ����ɫ����

GLuint createFramebuffer(const FramebufferCreateInfo& info);// ����֡�������

void renderToTexture(const RenderToTextureInfo& rtti);// ִ����Ⱦ����������洢��Ŀ��������
