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

// ����һ���꣬���ڴ���ImGui�ĸ�ѡ��
#define IMGUI_TOGGLE(NAME, DEFAULT)                                            \
  static bool NAME = DEFAULT;                                                  \
  ImGui::Checkbox(#NAME, &NAME);                                               \
  rtti.floatUniforms[#NAME] = NAME ? 1.0f : 0.0f;

// ����һ���꣬���ڴ���ImGui�Ļ�����
#define IMGUI_SLIDER(NAME, DEFAULT, MIN, MAX)                                  \
  static float NAME = DEFAULT;                                                 \
  ImGui::SliderFloat(#NAME, &NAME, MIN, MAX);                                  \
  rtti.floatUniforms[#NAME] = NAME;

unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
const char* glsl_version = "#version 460";

static float mouseX, mouseY; // �洢��������ȫ�ֱ���

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

class PostProcessPass
{
private:
    GLuint program; // ��ɫ������ID

public:
    PostProcessPass(const std::string& fragShader)
    {
        this->program = createShaderProgram("./src/shader/simple.vert", fragShader);

        glUseProgram(this->program);
        glUniform1i(glGetUniformLocation(program, "texture0"), 0);
        glUseProgram(0);
    }

    // ��Ⱦ����������������ɫ�����Ŀ��֡����
    void render(GLuint inputColorTexture, GLuint destFramebuffer = 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, destFramebuffer);

        glDisable(GL_DEPTH_TEST); // ������Ȳ���

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // ���������ɫ
        glClear(GL_COLOR_BUFFER_BIT); // �����ɫ����

        glUseProgram(this->program); // ʹ�ú��ڴ�����ɫ������

        glUniform2f(glGetUniformLocation(this->program, "resolution"), (float)SCR_WIDTH, (float)SCR_HEIGHT); // ���ݷֱ��ʸ���ɫ��

        glUniform1f(glGetUniformLocation(this->program, "time"), (float)glfwGetTime()); // ����ʱ�����ɫ��

        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, inputColorTexture); // ��������ɫ����

        glDrawArrays(GL_TRIANGLES, 0, 6); // ����һ������

        glUseProgram(0); // ͣ����ɫ������
    }
};

int main()
{
    glfwInit(); //��ʼ��GLFW

    //������Ҫ�ʹ�Ҫ�汾
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//ʹ�õ��Ǻ���ģʽ(Core-profile)��ֻ��ʹ��OpenGL���ܵ�һ���Ӽ�

    //�������ڶ���
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // ���ô��ڲ���������ʾ���ڱ߿�
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "BlackHole", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();//�����߳�
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//ע�ᴰ�ڱ任��������
    glfwSetCursorPosCallback(window, mouseCallback);//����ƶ�
    glfwSwapInterval(1); // ���ô�ֱͬ��������֡��
    glfwSetWindowPos(window, 0, 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);// ���ع��

    /*�ڵ����κ�OpenGL�ĺ���֮ǰ������Ҫ��ʼ��GLAD��*/
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

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

    // ����֡����������ɫ����
    GLuint fboBlackhole, texBlackhole;
    texBlackhole = createColorTexture(SCR_WIDTH, SCR_HEIGHT);

    // ����֡��������������Ϣ
    FramebufferCreateInfo info = {};
    info.colorTexture = texBlackhole;

    // ����֡�������
    if (!(fboBlackhole = createFramebuffer(info)))
    {
        assert(false); // ���֡��������Ƿ񴴽��ɹ���������ֹ����
    }

    // �ı���
    GLuint quadVAO = createQuadVAO();
    glBindVertexArray(quadVAO);

    // �������ڴ���Ч������ѭ��
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
        // ������ɫ����
        static GLuint texBlackhole = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti;
            rtti.fragShader = "./src/shader/blackhole.frag"; // ������ɫ������
            rtti.cubemapUniforms["galaxy"] = galaxy; // ����������ͼ�� uniform
            rtti.textureUniforms["colorMap"] = colorMap; // ����2D����� uniform
            rtti.floatUniforms["mouseX"] = mouseX; // �������X����� uniform
            rtti.floatUniforms["mouseY"] = mouseY; // �������Y����� uniform
            rtti.targetTexture = texBlackhole; // ����Ŀ������
            rtti.width = SCR_WIDTH; // ����������
            rtti.height = SCR_HEIGHT; // ��������߶�

            IMGUI_TOGGLE(gravatationalLensing, true); // ���������� ImGui �ĸ�ѡ��
            IMGUI_TOGGLE(renderBlackHole, true); // �����Ƿ����úڶ���Ⱦ
            IMGUI_TOGGLE(mouseControl, true); // ����������

            IMGUI_SLIDER(cameraRoll, 0.0f, -180.0f, 180.0f); //  ImGui �Ļ�����
            IMGUI_TOGGLE(frontView, false); // �����Ƿ�����ǰ��ͼ
            IMGUI_TOGGLE(topView, false); // �����Ƿ����ö���ͼ
            IMGUI_TOGGLE(adiskEnabled, true); // �����Ƿ�������ʯ����ʾ
            IMGUI_TOGGLE(adiskParticle, true); // �����Ƿ�������ʯ������Ч����ʾ

            IMGUI_SLIDER(adiskDensityV, 2.0f, 0.0f, 10.0f); // ���ƴ�ֱ�ܶ�
            IMGUI_SLIDER(adiskDensityH, 4.0f, 0.0f, 10.0f); // ����ˮƽ�ܶ�
            IMGUI_SLIDER(adiskHeight, 0.55f, 0.0f, 1.0f); // ������ʯ�̸߶�
            IMGUI_SLIDER(adiskLit, 0.25f, 0.0f, 4.0f); // ������ʯ������
            IMGUI_SLIDER(adiskNoiseLOD, 5.0f, 1.0f, 12.0f); // ��������ϸ�ڼ���
            IMGUI_SLIDER(adiskNoiseScale, 0.8f, 0.0f, 10.0f); // ���������߶�
            IMGUI_SLIDER(adiskSpeed, 0.5f, 0.0f, 1.0f); // ������ʯ���˶��ٶ�

            renderToTexture(rtti); // ��Ⱦ������
        }
        // ��������������ȡ������
        static GLuint texBrightness = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti; // ����һ����Ⱦ�������������Ϣ�ṹ��
            rtti.fragShader = "./src/shader/brightness.frag";    // ������ȡ
            rtti.textureUniforms["texture0"] = texBlackhole;  // ��֮ǰ������ texBlackhole ��
            rtti.targetTexture = texBrightness;   // ������Ⱦ������Ŀ������Ϊ texBrightness
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT; // ������Ⱦ��Ŀ������Ŀ�Ⱥ͸߶�����Ļ�ֱ�����ͬ
            renderToTexture(rtti);    // ʹ��������Ϣ rtti ִ����Ⱦ������Ĳ�����������洢�� texBrightness ��
        }

        const int MAX_BLOOM_ITER = 8; // �������ĻԹ�������������ڿ��ƻԹ�Ч���Ķ�㼶��

        // �������ڴ洢�Թ�Ч���Ķ���������飬�����²������ϲ����׶ε�����
        static GLuint texDownsampled[MAX_BLOOM_ITER];
        static GLuint texUpsampled[MAX_BLOOM_ITER];
        if (texDownsampled[0] == 0)
        {
            // ����Ƿ��Ѿ���ʼ����Щ���������ظ�����
            for (int i = 0; i < MAX_BLOOM_ITER; i++)
            {
                // ѭ�������������������
                texDownsampled[i] = createColorTexture(SCR_WIDTH >> (i + 1), SCR_HEIGHT >> (i + 1));// �����²��������ߴ��𼶼�С
                texUpsampled[i] = createColorTexture(SCR_WIDTH >> i, SCR_HEIGHT >> i);// ����һ���ϲ��������ߴ�������
            }
        }

        static int bloomIterations = MAX_BLOOM_ITER;  // ���ƻԹ�Ч���ĵ�������
        ImGui::SliderInt("bloomIterations", &bloomIterations, 1, 8); // ���������� ImGui ������������
        for (int level = 0; level < bloomIterations; level++)
        {
            RenderToTextureInfo rtti; // ����һ����Ⱦ�������������Ϣ�ṹ�塣
            rtti.fragShader = "./src/shader/bloom_downsample.frag"; // �²���������
            rtti.textureUniforms["texture0"] = level == 0 ? texBrightness : texDownsampled[level - 1];//����һ�����²���������������������
            rtti.targetTexture = texDownsampled[level];   // ������Ⱦ������Ŀ������Ϊ��ǰ������²�������
            rtti.width = SCR_WIDTH >> (level + 1);
            rtti.height = SCR_HEIGHT >> (level + 1);  // ������Ⱦ��Ŀ������Ŀ�Ⱥ͸߶ȣ��𼶵ݼ���
            renderToTexture(rtti);    // ʹ��������Ϣ rtti ִ���²���������������洢�ڵ�ǰ������²��������С�
        }

        // ���ÿ���Թ⼶������ѭ��ִ�����²���
        for (int level = bloomIterations - 1; level >= 0; level--)
        {
            RenderToTextureInfo rtti;
            rtti.fragShader = "./src/shader/bloom_upsample.frag"; // �����ϲ�����ɫ������
            rtti.textureUniforms["texture0"] = level == bloomIterations - 1
                ? texDownsampled[level]
                : texUpsampled[level + 1]; // �����ϲ����������������
            rtti.textureUniforms["texture1"] =
                level == 0 ? texBrightness : texDownsampled[level - 1]; // ������������
            rtti.targetTexture = texUpsampled[level]; // ����Ŀ���ϲ�������
            rtti.width = SCR_WIDTH >> level; // �����ϲ�������Ŀ��
            rtti.height = SCR_HEIGHT >> level; // �����ϲ�������ĸ߶�

            renderToTexture(rtti); // ��Ⱦ���ϲ�������
        }

        // �������ںϳɻԹ�Ч������������
        static GLuint texBloomFinal = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti; // ����һ����Ⱦ�������������Ϣ�ṹ�塣
            rtti.fragShader = "./src/shader/bloom_composite.frag";  // �Թ�Ч���ĺϳɲ�����
            rtti.textureUniforms["texture0"] = texBlackhole;// �� texBlackhole �󶨣�ͨ���Ǻڶ���Ⱦ�����
            rtti.textureUniforms["texture1"] = texUpsampled[0];   // �� texUpsampled[0] �󶨣�ͨ�����ϲ����ĻԹ�Ч����
            rtti.targetTexture = texBloomFinal;   // ������Ⱦ������Ŀ������
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;// ������Ⱦ��Ŀ������Ŀ�Ⱥ͸߶�����Ļ�ֱ�����ͬ��

            IMGUI_SLIDER(bloomStrength, 0.1f, 0.0f, 1.0f); // ���������� ImGui �ĸ��㻬����

            renderToTexture(rtti);// ʹ��������Ϣ rtti ִ�лԹ�Ч���ĺϳɲ�����������洢�� texBloomFinal �С�
        }

        // ��������ɫ��ӳ�����������
        static GLuint texTonemapped = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti; // ����һ����Ⱦ�������������Ϣ�ṹ�塣
            rtti.fragShader = "./src/shader/tonemapping.frag";  // ɫ��ӳ�䡣
            rtti.textureUniforms["texture0"] = texBloomFinal; //�� texBloomFinal �󶨣��ϳɺ�ĻԹ�Ч��ͼ��
            rtti.targetTexture = texTonemapped;   // ������Ⱦ������Ŀ������Ϊ texTonemapped��
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;// ������Ⱦ��Ŀ������Ŀ�Ⱥ͸߶�����Ļ�ֱ�����ͬ��

            IMGUI_TOGGLE(tonemappingEnabled, true); // ���������� ImGui �ĸ�ѡ��
            IMGUI_SLIDER(gamma, 2.5f, 1.0f, 4.0f); // ���������� ImGui �ĸ��㻬����

            renderToTexture(rtti);    // ʹ��������Ϣ rtti ִ��ɫ��ӳ�������������洢�� texTonemapped �С�
        }

        passthrough.render(texTonemapped); // ��Ⱦ���ڴ���Ч������Ļ

        ImGui::End();

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

//�ӿڵ���
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

void mouseCallback(GLFWwindow* window, double x, double y)
{
    static float lastX = 400.0f; // ��һ�ε����X����
    static float lastY = 300.0f; // ��һ�ε����Y����
    static float yaw = 0.0f; // ������
    static float pitch = 0.0f; // ƫ����
    static float firstMouse = true; // �Ƿ��ǵ�һ���ƶ����

    mouseX = (float)x; // ����ȫ�����X����
    mouseY = (float)y; // ����ȫ�����Y����
}