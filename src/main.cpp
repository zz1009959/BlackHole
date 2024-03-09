#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<imgui/imgui.h>
#include<imgui/imgui_impl_glfw.h>
#include<imgui/imgui_impl_opengl3.h>

#include"render.h"
#include"texture.h"
#include"shader.h"

#include<iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double x, double y);
void processInput(GLFWwindow* window);

// 定义一个宏，用于创建ImGui的复选框
#define IMGUI_TOGGLE(NAME, DEFAULT)                                            \
  static bool NAME = DEFAULT;                                                  \
  ImGui::Checkbox(#NAME, &NAME);                                               \
  rtti.floatUniforms[#NAME] = NAME ? 1.0f : 0.0f;

// 定义一个宏，用于创建ImGui的滑动条
#define IMGUI_SLIDER(NAME, DEFAULT, MIN, MAX)                                  \
  static float NAME = DEFAULT;                                                 \
  ImGui::SliderFloat(#NAME, &NAME, MIN, MAX);                                  \
  rtti.floatUniforms[#NAME] = NAME;

unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
const char* glsl_version = "#version 460";

static float mouseX, mouseY; // 存储鼠标坐标的全局变量

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

class PostProcessPass
{
private:
    GLuint program; // 着色器程序ID

public:
    PostProcessPass(const std::string& fragShader)
    {
        this->program = createShaderProgram("./src/shader/simple.vert", fragShader);

        glUseProgram(this->program);
        glUniform1i(glGetUniformLocation(program, "texture0"), 0);
        glUseProgram(0);
    }

    // 渲染方法，接受输入颜色纹理和目标帧缓冲
    void render(GLuint inputColorTexture, GLuint destFramebuffer = 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, destFramebuffer);

        glDisable(GL_DEPTH_TEST); // 禁用深度测试

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // 设置清除颜色
        glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲

        glUseProgram(this->program); // 使用后期处理着色器程序

        glUniform2f(glGetUniformLocation(this->program, "resolution"), (float)SCR_WIDTH, (float)SCR_HEIGHT); // 传递分辨率给着色器

        glUniform1f(glGetUniformLocation(this->program, "time"), (float)glfwGetTime()); // 传递时间给着色器

        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, inputColorTexture); // 绑定输入颜色纹理

        glDrawArrays(GL_TRIANGLES, 0, 6); // 绘制一个矩形

        glUseProgram(0); // 停用着色器程序
    }
};

int main()
{
    glfwInit(); //初始化GLFW

    //设置主要和次要版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用的是核心模式(Core-profile)，只能使用OpenGL功能的一个子集

    //创建窗口对象
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // 配置窗口参数，不显示窗口边框
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "BlackHole", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();//结束线程
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//注册窗口变换监听函数
    glfwSetCursorPosCallback(window, mouseCallback);//鼠标移动
    glfwSwapInterval(1); // 启用垂直同步，控制帧率
    glfwSetWindowPos(window, 0, 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);// 隐藏光标

    /*在调用任何OpenGL的函数之前我们需要初始化GLAD。*/
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

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

    // 创建帧缓冲对象和颜色纹理
    GLuint fboBlackhole, texBlackhole;
    texBlackhole = createColorTexture(SCR_WIDTH, SCR_HEIGHT);

    // 创建帧缓冲对象的配置信息
    FramebufferCreateInfo info = {};
    info.colorTexture = texBlackhole;

    // 创建帧缓冲对象
    if (!(fboBlackhole = createFramebuffer(info)))
    {
        assert(false); // 检查帧缓冲对象是否创建成功，否则终止程序
    }

    // 四边形
    GLuint quadVAO = createQuadVAO();
    glBindVertexArray(quadVAO);

    // 创建后期处理效果的主循环
    PostProcessPass passthrough("./src/shader/passthrough.frag");

    std::vector<std::string> faces
    {
        "./assets/skybox_nebula_dark/right.png",
        "./assets/skybox_nebula_dark/left.png",
        "./assets/skybox_nebula_dark/top.png",
        "./assets/skybox_nebula_dark/bottom.png",
        "./assets/skybox_nebula_dark/front.png",
        "./assets/skybox_nebula_dark/back.png"
    };
    GLuint galaxy = loadCubemap(faces);
    GLuint colorMap = loadTexture("./assets/color_map.png");

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
        // 创建颜色纹理
        static GLuint texBlackhole = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti;
            rtti.fragShader = "./src/shader/blackhole.frag"; // 设置着色器程序
            rtti.cubemapUniforms["galaxy"] = galaxy; // 设置立方贴图的 uniform
            rtti.textureUniforms["colorMap"] = colorMap; // 设置2D纹理的 uniform
            rtti.floatUniforms["mouseX"] = mouseX; // 设置鼠标X坐标的 uniform
            rtti.floatUniforms["mouseY"] = mouseY; // 设置鼠标Y坐标的 uniform
            rtti.targetTexture = texBlackhole; // 设置目标纹理
            rtti.width = SCR_WIDTH; // 设置纹理宽度
            rtti.height = SCR_HEIGHT; // 设置纹理高度

            IMGUI_TOGGLE(gravatationalLensing, true); // 创建并处理 ImGui 的复选框
            IMGUI_TOGGLE(renderBlackHole, true); // 控制是否启用黑洞渲染
            IMGUI_TOGGLE(mouseControl, true); // 控制鼠标控制

            IMGUI_SLIDER(cameraRoll, 0.0f, -180.0f, 180.0f); //  ImGui 的滑动条
            IMGUI_TOGGLE(frontView, false); // 控制是否启用前视图
            IMGUI_TOGGLE(topView, false); // 控制是否启用顶视图
            IMGUI_TOGGLE(adiskEnabled, true); // 控制是否启用陨石盘显示
            IMGUI_TOGGLE(adiskParticle, true); // 控制是否启用陨石盘粒子效果显示

            IMGUI_SLIDER(adiskDensityV, 2.0f, 0.0f, 10.0f); // 控制垂直密度
            IMGUI_SLIDER(adiskDensityH, 4.0f, 0.0f, 10.0f); // 控制水平密度
            IMGUI_SLIDER(adiskHeight, 0.55f, 0.0f, 1.0f); // 控制陨石盘高度
            IMGUI_SLIDER(adiskLit, 0.25f, 0.0f, 4.0f); // 控制陨石盘亮度
            IMGUI_SLIDER(adiskNoiseLOD, 5.0f, 1.0f, 12.0f); // 控制噪音细节级别
            IMGUI_SLIDER(adiskNoiseScale, 0.8f, 0.0f, 10.0f); // 控制噪音尺度
            IMGUI_SLIDER(adiskSpeed, 0.5f, 0.0f, 1.0f); // 控制陨石盘运动速度

            renderToTexture(rtti); // 渲染到纹理
        }
        // 创建用于亮度提取的纹理
        static GLuint texBrightness = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti; // 创建一个渲染到纹理的配置信息结构体
            rtti.fragShader = "./src/shader/brightness.frag";    // 亮度提取
            rtti.textureUniforms["texture0"] = texBlackhole;  // 与之前创建的 texBlackhole 绑定
            rtti.targetTexture = texBrightness;   // 设置渲染操作的目标纹理为 texBrightness
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT; // 设置渲染的目标纹理的宽度和高度与屏幕分辨率相同
            renderToTexture(rtti);    // 使用配置信息 rtti 执行渲染到纹理的操作，将结果存储在 texBrightness 中
        }

        const int MAX_BLOOM_ITER = 8; // 定义最大的辉光迭代次数，用于控制辉光效果的多层级别

        // 创建用于存储辉光效果的多层纹理数组，包括下采样和上采样阶段的纹理
        static GLuint texDownsampled[MAX_BLOOM_ITER];
        static GLuint texUpsampled[MAX_BLOOM_ITER];
        if (texDownsampled[0] == 0)
        {
            // 检查是否已经初始化这些纹理，避免重复创建
            for (int i = 0; i < MAX_BLOOM_ITER; i++)
            {
                // 循环迭代，创建多层纹理
                texDownsampled[i] = createColorTexture(SCR_WIDTH >> (i + 1), SCR_HEIGHT >> (i + 1));// 创建下采样纹理，尺寸逐级减小
                texUpsampled[i] = createColorTexture(SCR_WIDTH >> i, SCR_HEIGHT >> i);// 创建一个上采样纹理，尺寸逐级增大
            }
        }

        static int bloomIterations = MAX_BLOOM_ITER;  // 控制辉光效果的迭代次数
        ImGui::SliderInt("bloomIterations", &bloomIterations, 1, 8); // 创建并处理 ImGui 的整数滑动条
        for (int level = 0; level < bloomIterations; level++)
        {
            RenderToTextureInfo rtti; // 创建一个渲染到纹理的配置信息结构体。
            rtti.fragShader = "./src/shader/bloom_downsample.frag"; // 下采样操作。
            rtti.textureUniforms["texture0"] = level == 0 ? texBrightness : texDownsampled[level - 1];//与上一级的下采样结果或亮度纹理关联。
            rtti.targetTexture = texDownsampled[level];   // 设置渲染操作的目标纹理为当前级别的下采样纹理。
            rtti.width = SCR_WIDTH >> (level + 1);
            rtti.height = SCR_HEIGHT >> (level + 1);  // 设置渲染的目标纹理的宽度和高度，逐级递减。
            renderToTexture(rtti);    // 使用配置信息 rtti 执行下采样操作，将结果存储在当前级别的下采样纹理中。
        }

        // 针对每个辉光级别，逆序循环执行以下操作
        for (int level = bloomIterations - 1; level >= 0; level--)
        {
            RenderToTextureInfo rtti;
            rtti.fragShader = "./src/shader/bloom_upsample.frag"; // 设置上采样着色器程序
            rtti.textureUniforms["texture0"] = level == bloomIterations - 1
                ? texDownsampled[level]
                : texUpsampled[level + 1]; // 设置上采样所需的输入纹理
            rtti.textureUniforms["texture1"] =
                level == 0 ? texBrightness : texDownsampled[level - 1]; // 设置亮度纹理
            rtti.targetTexture = texUpsampled[level]; // 设置目标上采样纹理
            rtti.width = SCR_WIDTH >> level; // 设置上采样纹理的宽度
            rtti.height = SCR_HEIGHT >> level; // 设置上采样纹理的高度

            renderToTexture(rtti); // 渲染到上采样纹理
        }

        // 创建用于合成辉光效果的最终纹理
        static GLuint texBloomFinal = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti; // 创建一个渲染到纹理的配置信息结构体。
            rtti.fragShader = "./src/shader/bloom_composite.frag";  // 辉光效果的合成操作。
            rtti.textureUniforms["texture0"] = texBlackhole;// 与 texBlackhole 绑定，通常是黑洞渲染结果。
            rtti.textureUniforms["texture1"] = texUpsampled[0];   // 与 texUpsampled[0] 绑定，通常是上采样的辉光效果。
            rtti.targetTexture = texBloomFinal;   // 设置渲染操作的目标纹理。
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;// 设置渲染的目标纹理的宽度和高度与屏幕分辨率相同。

            IMGUI_SLIDER(bloomStrength, 0.1f, 0.0f, 1.0f); // 创建并处理 ImGui 的浮点滑动条

            renderToTexture(rtti);// 使用配置信息 rtti 执行辉光效果的合成操作，将结果存储在 texBloomFinal 中。
        }

        // 创建用于色调映射的最终纹理
        static GLuint texTonemapped = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti; // 创建一个渲染到纹理的配置信息结构体。
            rtti.fragShader = "./src/shader/tonemapping.frag";  // 色调映射。
            rtti.textureUniforms["texture0"] = texBloomFinal; //与 texBloomFinal 绑定，合成后的辉光效果图。
            rtti.targetTexture = texTonemapped;   // 设置渲染操作的目标纹理为 texTonemapped。
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;// 设置渲染的目标纹理的宽度和高度与屏幕分辨率相同。

            IMGUI_TOGGLE(tonemappingEnabled, true); // 创建并处理 ImGui 的复选框
            IMGUI_SLIDER(gamma, 2.5f, 1.0f, 4.0f); // 创建并处理 ImGui 的浮点滑动条

            renderToTexture(rtti);    // 使用配置信息 rtti 执行色调映射操作，将结果存储在 texTonemapped 中。
        }

        passthrough.render(texTonemapped); // 渲染后期处理效果到屏幕

        ImGui::End();

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

//视口调整
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//按键关闭
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouseCallback(GLFWwindow* window, double x, double y)
{
    static float lastX = 400.0f; // 上一次的鼠标X坐标
    static float lastY = 300.0f; // 上一次的鼠标Y坐标
    static float yaw = 0.0f; // 俯仰角
    static float pitch = 0.0f; // 偏航角
    static float firstMouse = true; // 是否是第一次移动鼠标

    mouseX = (float)x; // 更新全局鼠标X坐标
    mouseY = (float)y; // 更新全局鼠标Y坐标
}