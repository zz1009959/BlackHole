#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include<iostream>
#include <vector>
#include <cmath>

#include"buffer.h"

// �ı���
float vertices[] = {
         1.0f,  1.0f,  // ���Ͻ�
         1.0f, -1.0f,  // ���½�
        -1.0f, -1.0f,  // ���½�
        -1.0f,  1.0f   // ���Ͻ�
};
GLuint indices[] = {
    0, 1, 3,  // ��һ��������
    1, 2, 3   // �ڶ���������
};

float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};
const GLfloat PI = 3.14159265358979323846f;
//������ݻ��ֳ�50*50������
int Y_SEGMENTS = 50;
int X_SEGMENTS = 50;

GLuint setSkyBoxVAO()
{
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    return skyboxVAO;
}

void calculateSphere(std::vector<float>& vertices, std::vector<int>& indices, float r, int X_SEGMENTS, int Y_SEGMENTS)
{
    //������Ķ���
    for (int y = 0; y <= Y_SEGMENTS; y++)
    {
        for (int x = 0; x <= X_SEGMENTS; x++)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = r * cos(xSegment * 2.0f * PI) * sin(ySegment * PI);  // x = r * cos�� * sin��
            float yPos = r * cos(ySegment * PI);                              // y = r * cos��
            float zPos = r * sin(xSegment * 2.0f * PI) * sin(ySegment * PI);  // z = r * cos�� * sin��
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
        }
    }

    //�������Indices
    for (int i = 0; i < Y_SEGMENTS; i++)
    {
        for (int j = 0; j < X_SEGMENTS; j++)
        {
            indices.push_back(i * (X_SEGMENTS + 1) + j);
            indices.push_back((i + 1) * (X_SEGMENTS + 1) + j);
            indices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            indices.push_back(i * (X_SEGMENTS + 1) + j);
            indices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            indices.push_back(i * (X_SEGMENTS + 1) + j + 1);
        }
    }
}

GLuint setSphereVAO()
{
    std::vector<float> sphereVertices;
    std::vector<int> sphereIndices;
    calculateSphere(sphereVertices, sphereIndices, 1.0, X_SEGMENTS, Y_SEGMENTS);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //���ɲ��������VAO��VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //���������ݰ�����ǰĬ�ϵĻ�����
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

    GLuint element_buffer_object;//EBO
    glGenBuffers(1, &element_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

    //���ö�������ָ��
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return VAO;
}

GLuint setQuadVAO()
{
    // ���������������Ͷ��㻺�����
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // �󶨶����������
    glBindVertexArray(VAO);

    // ���������ݸ��Ƶ����㻺�����
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // ���������ݸ��Ƶ������������
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // ���ö�������ָ��
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return VAO;
}