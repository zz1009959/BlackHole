#include "shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include<GLFW/glfw3.h>

static std::string readFile(const std::string& file)
{
    std::string shaderCode;
    std::ifstream ifs(file, std::ios::in);
    if (ifs.is_open())
    {
        std::stringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }
    else
    {
        throw "Failed to open file: " + file;
    }
}

static GLuint compileShader(const std::string& shaderSource,
                            GLenum shaderType)
{
    // ������ɫ��
    GLuint shader = glCreateShader(shaderType);

    // ������ɫ��
    char const* pShaderSource = shaderSource.c_str();
    glShaderSource(shader, 1, &pShaderSource, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // maxLength ������ NULL �ַ�
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
        std::cout << infoLog[0] << std::endl;
        glDeleteShader(shader);
        throw "Failed to compile the shader.";
    }

    return shader;
}
GLuint createShaderProgram(const std::string& vertexShaderFile,
                           const std::string& fragmentShaderFile)
{
    // ���붥���Ƭ����ɫ��
    std::cout << "Compiling vertex shader: " << vertexShaderFile << std::endl;
    GLuint vertexShader = compileShader(readFile(vertexShaderFile), GL_VERTEX_SHADER);

    std::cout << "Compiling fragment shader: " << fragmentShaderFile << std::endl;
    GLuint fragmentShader = compileShader(readFile(fragmentShaderFile), GL_FRAGMENT_SHADER);

    // ������ɫ������
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // ������ɫ������
    glLinkProgram(program);
    GLint isLinked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        int maxLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        if (maxLength > 0)
        {
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, NULL, &infoLog[0]);
            std::cout << infoLog[0] << std::endl;
            throw "Failed to link the shader.";
        }
    }

    // �ɹ����Ӻ������ɫ��
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program; // ������ɫ�������OpenGL����ID
}