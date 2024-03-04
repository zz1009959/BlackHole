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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);

unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
const char* glsl_version = "#version 460";
const GLfloat PI = 3.14159265358979323846f;
//将球横纵划分成50*50的网格
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
    glfwInit(); //初始化GLFW

    //设置主要和次要版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用的是核心模式(Core-profile)，只能使用OpenGL功能的一个子集

    //创建窗口对象
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();//结束线程
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//注册窗口变换监听函数
    glfwSetCursorPosCallback(window, mouse_callback);//鼠标移动
    glfwSetScrollCallback(window, scroll_callback);//鼠标滚轮滚动

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);// 隐藏光标

    /*在调用任何OpenGL的函数之前我们需要初始化GLAD。*/
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    //------------------------------
    //创建imgui上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    //设置imgui风格
    ImGui::StyleColorsDark();

    //设置平台和渲染器
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    Shader ourShader("./src/shader/sphere.vert", "./src/shader/sphere.frag");
    Shader skyboxShader("./src/shader/skybox.vert", "./src/shader/skybox.frag");
    Shader sphereShader("./src/shader/simple.vert", "./src/shader/blackhole.frag");

    // 立方体VAO
    GLuint cubeVAO = setCubeVAO();

    // 天空盒 VAO
    GLuint skyboxVAO = setSkyBoxVAO();

    // 球体
    GLuint VAO = setSphereVAO();

    // 四边形
    GLuint quadVAO = setQuadVAO();

    //解绑VAO和VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 加载纹理
    //unsigned int cubeTexture = loadTexture("./image/container.jpg");
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

    // 设置纹理
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    sphereShader.use();
    sphereShader.setInt("skybox", 0);

    //imgui测试值
    ImVec4 clear_color = ImVec4(0.1, 0.1, 0.1, 1.0);
    float speed = 1.0;
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        //每帧时间逻辑
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //输入
        processInput(window);
        //启用imgui框架
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("ImGui");
        ImGui::ColorEdit3("clear color", (float*)&clear_color);
        ImGui::SliderFloat("Sky Speed", &speed, -2.0, 2.0);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // 渲染指令
        // ---------------

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // sphere
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // blackhole
        sphereShader.use();
        //sphereShader.setMat4("view", camera.GetViewMatrix());
        //sphereShader.setVec3("cameraPos", camera.Position);
        sphereShader.setFloat("time", currentFrame);
        sphereShader.setVec2("resolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
        sphereShader.setFloat("mouseX", lastX);
        sphereShader.setFloat("mouseY", lastY);
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        model = glm::rotate(model, glm::radians(float(speed * currentFrame)), glm::vec3(0.0, 1.0, 0.0));
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));//删除平移
        skyboxShader.setMat4("model", model);
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        //渲染gui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //交换缓冲
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 清理ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();//正确释放/删除之前的分配的所有资源
    return 0;
}

//视口调整（回调函数？）
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//按键关闭
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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
    float yoffset = lastY - ypos; // 反转，因为 y 坐标从下到上

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}