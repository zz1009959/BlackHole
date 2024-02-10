#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include<iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const char* vertexShaderSource = "#version 460 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
"}\n";

const char* fragmentShaderSource = "#version 460 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n";

int main()
{
    glfwInit(); //初始化GLFW

    //设置主要和次要版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用的是核心模式(Core-profile)，只能使用OpenGL功能的一个子集

    //创建窗口对象
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();//结束线程
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//注册窗口变换监听函数

    /*在调用任何OpenGL的函数之前我们需要初始化GLAD。*/
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //创建顶点着色器和片段着色器
    unsigned int vertexShader, fragmentShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //附加着色器源码
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    //检测是否编译成功
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR:SHADER::VERTEX::COMPICATION_FAILED\n" << infoLog << std::endl;
    }
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR:SHADER::FRAGMENT::COMPICATION_FAILED\n" << infoLog << std::endl;
    }

    //创建程序对象
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    //将着色器附加到程序对象上
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    //链接着色器
    glLinkProgram(shaderProgram);

    //检测链接着色器程序是否成功
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR:SHADER::SHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    }

    //删除着色器（着色器对象链接到程序对象以后）
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //顶点数组
    //在实际开发中用GLfloat
    float vertices[] = {
        0.5f, 0.5f, 0.0f,   // 右上角
        0.5f, -0.5f, 0.0f,  // 右下角
        -0.5f, -0.5f, 0.0f, // 左下角
        -0.5f, 0.5f, 0.0f   // 左上角
    };

    unsigned int indices[] = {
        // 注意索引从0开始!
        // 此例的索引(0,1,2,3)就是顶点数组vertices的下标，
        // 这样可以由下标代表顶点组合成矩形

        0, 1, 3, // 第一个三角形
        1, 2, 3  // 第二个三角形
    };
    //创建VBO,VAO,EBO
    unsigned int VBO, VAO, EBO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);

    //绑定VAO对象
    glBindVertexArray(VAO);

    //绑定缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 填充VBO数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //把索引复制到缓冲里
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //填充EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //解析顶点数据（设置顶点属性指针）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //解绑
    glBindVertexArray(0);

    //设置线框绘制模式
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //GL_FILL为填充
    while (!glfwWindowShouldClose(window))
    {
        //输入
        processInput(window);

        //渲染指令
        /*当调用glClear函数，清除颜色缓冲之后，整个颜色缓冲都会被填充为glClearColor里所设置的颜色。*/
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //绘制三角形
        glUseProgram(shaderProgram);//使用着色器对象

        //绑定VAO
        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 3);--->对应VBO
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);//--->EBO

        //解绑
        glBindVertexArray(0);

        //交换缓冲
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /*可选:一旦资源超出其用途，就取消分配所有资源:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);*/

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
}