#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<shader.h>
#include<camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<imgui/imgui.h>
#include<imgui/imgui_impl_glfw.h>
#include<imgui/imgui_impl_opengl3.h>

#include"buffer.h"
#include"texture.h"

#include<iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);

unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
const char* glsl_version = "#version 460";
const GLfloat PI = 3.14159265358979323846f;
//������ݻ��ֳ�50*50������
int Y = 50;
int X = 50;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    glfwInit(); //��ʼ��GLFW

    //������Ҫ�ʹ�Ҫ�汾
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//ʹ�õ��Ǻ���ģʽ(Core-profile)��ֻ��ʹ��OpenGL���ܵ�һ���Ӽ�

    //�������ڶ���
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // ���ô��ڲ���������ʾ���ڱ߿�
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "BlackHole", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();//�����߳�
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//ע�ᴰ�ڱ任��������
    glfwSetCursorPosCallback(window, mouse_callback);//����ƶ�
    glfwSwapInterval(1); // ���ô�ֱͬ��������֡��

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);// ���ع��

    /*�ڵ����κ�OpenGL�ĺ���֮ǰ������Ҫ��ʼ��GLAD��*/
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    //------------------------------
    //����imgui������
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    //����imgui���
    ImGui::StyleColorsDark();

    //����ƽ̨����Ⱦ��
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    Shader BlackHoleShader("./src/shader/simple.vert", "./src/shader/blackhole.frag");

    // �ı���
    GLuint quadVAO = setQuadVAO();

    std::vector<std::string> faces
    {
        "./assets/skybox_nebula_dark/right.png",
        "./assets/skybox_nebula_dark/left.png",
        "./assets/skybox_nebula_dark/top.png",
        "./assets/skybox_nebula_dark/bottom.png",
        "./assets/skybox_nebula_dark/front.png",
        "./assets/skybox_nebula_dark/back.png"
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    GLuint colorMap = loadTexture("./assets/color_map.png");

    // ��������
    BlackHoleShader.use();
    BlackHoleShader.setInt("skybox", 0);
    glActiveTexture(GL_TEXTURE1);
    BlackHoleShader.setInt("colorMap", 1);

    //imguiֵ
    ImVec4 clear_color = ImVec4(0.1, 0.1, 0.1, 1.0);
    float speed = 1.0;
    float adiskHeight = 0.2;
    float adiskLit = 0.5;
    float adiskDensityV = 2.0;
    float adiskDensityH = 4.0;
    float adiskNoiseScale = 1;
    float adiskNoiseLOD = 5.0;
    float adiskSpeed = 0.5;
    bool renderBlackHole = true;
    bool gravatationalLensing = true;
    bool adiskEnabled = true;
    bool adiskParticle = true;
    bool mouseControl = true;
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        //ÿ֡ʱ���߼�
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //����
        processInput(window);
        //����imgui���
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("ImGui");
        ImGui::ColorEdit3("clear color", (float*)&clear_color);
        ImGui::SliderFloat("Sky Speed", &speed, -2.0, 2.0);

        ImGui::SliderFloat("adiskHeight", &adiskHeight, 0.1, 0.3);
        ImGui::SliderFloat("adiskLit", &adiskLit, 0.1, 1.0);
        ImGui::SliderFloat("adiskDensityV", &adiskDensityV, 1.0, 3.0);
        ImGui::SliderFloat("adiskDensityH", &adiskDensityH, 3.0, 5.0);
        ImGui::SliderFloat("adiskNoiseScale", &adiskNoiseScale, 0.5, 2.0);
        ImGui::SliderFloat("adiskNoiseLOD", &adiskNoiseLOD, 3.0, 7.0);
        ImGui::SliderFloat("adiskSpeed", &adiskSpeed, 0.1, 1);
        ImGui::Checkbox("renderBlackHole", &renderBlackHole);
        ImGui::Checkbox("gravatationalLensing", &gravatationalLensing);
        ImGui::Checkbox("adiskEnabled", &adiskEnabled);
        ImGui::Checkbox("adiskParticle", &adiskParticle);
        ImGui::Checkbox("mouseControl", &mouseControl);

        BlackHoleShader.use();
        BlackHoleShader.setFloat("renderBlackHole", renderBlackHole);
        BlackHoleShader.setFloat("gravatationalLensing", gravatationalLensing);
        BlackHoleShader.setFloat("adiskEnabled", adiskEnabled);

        BlackHoleShader.setFloat("adiskHeight", adiskHeight);
        BlackHoleShader.setFloat("adiskLit", adiskLit);
        BlackHoleShader.setFloat("adiskDensityV", adiskDensityV);
        BlackHoleShader.setFloat("adiskDensityH", adiskDensityH);
        BlackHoleShader.setFloat("adiskNoiseScale", adiskNoiseScale);
        BlackHoleShader.setFloat("adiskNoiseLOD", adiskNoiseLOD);
        BlackHoleShader.setFloat("adiskSpeed", adiskSpeed);

        BlackHoleShader.setFloat("mouseControl", mouseControl);

        /*ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);*/
        ImGui::End();

        // ---------------

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // blackhole
        BlackHoleShader.use();
        BlackHoleShader.setFloat("time", currentFrame);
        BlackHoleShader.setVec2("resolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
        BlackHoleShader.setFloat("mouseX", lastX);
        BlackHoleShader.setFloat("mouseY", lastY);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, colorMap);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //��Ⱦgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //��������
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // ����ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();//��ȷ�ͷ�/ɾ��֮ǰ�ķ����������Դ
    return 0;
}

//�ӿڵ������ص���������
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//�����ر�
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // ��ת����Ϊ y ������µ���

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}