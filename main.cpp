#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "pid.hpp"
#include "robot.hpp"
#include "circle.hpp"

const unsigned int WIDTH = 1250; 
const unsigned int HEIGHT = 750; 
const float aspect_ratio = (float)WIDTH/(float)HEIGHT; 

GLuint CreateShaderProgram() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view; 
        uniform mat4 projection; 

        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 uColor; // Added for customizable color

        void main() {
            FragColor = uColor;
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


int main(void)
{
    if (!glfwInit()) return -1;
    
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    unsigned int width = WIDTH, height = HEIGHT;
    GLFWwindow* window = glfwCreateWindow(width, height, "PID Tuning Sim", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    GLuint shaderProgram = CreateShaderProgram();

    CircleIndicator mouseIndicator(0.0f, 0.0f);
    SwerveDrive robot(0.0f, 0.0f);

    float moveGains[3] = {1.0f, 0.0f, 0.00f}; // P, I, D
    float turnGains[3] = {1.0f, 0.0f, 0.00f}; // P, I, D

    PID pid_x(moveGains[0], moveGains[1], moveGains[2]);
    PID pid_y(moveGains[0], moveGains[1], moveGains[2]);
    PID pid_r(turnGains[0], turnGains[1], turnGains[2]);

    float time = 0.01; 

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);

        ImGui::Begin("PID Controller Tuning");

        ImGui::SetWindowFontScale(1.2f);
        
        if (ImGui::CollapsingHeader("Translation PID")) {
            ImGui::SliderFloat("Move P", &moveGains[0], 0.0f, 10.0f);
            ImGui::SliderFloat("Move I", &moveGains[1], 0.0f, 2.0f);
            ImGui::SliderFloat("Move D", &moveGains[2], 0.0f, 10.0f);
            pid_x.P = pid_y.P = moveGains[0];
            pid_x.I = pid_y.I = moveGains[1];
            pid_x.D = pid_y.D = moveGains[2];
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Rotation PID")) {
            ImGui::SliderFloat("Turn P", &turnGains[0], 0.0f, 10.0f);
            ImGui::SliderFloat("Turn I", &turnGains[1], 0.0f, 2.0f);
            ImGui::SliderFloat("Turn D", &turnGains[2], 0.0f, 10.0f);
            pid_r.P = turnGains[0];
            pid_r.I = turnGains[1];
            pid_r.D = turnGains[2];
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::SliderFloat("Time", &time, 0.01f, 0.1f);
        ImGui::Text("Position: %.4f, %.4f", robot.x, robot.y);
        ImGui::Text("Rotation:  %.4f", robot.r);
        ImGui::End();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        float ndcX = ((2.0f * (float)mouseX) / width - 1.0f) * aspect_ratio;
        float ndcY = (1.0f - (2.0f * (float)mouseY) / height);

        mouseIndicator.x = ndcX;
        mouseIndicator.y = ndcY;
        
        float dx = ndcX - robot.x;
        float dy = ndcY - robot.y;
        
        float targetAngle = atan2(dy, dx) - 1.5708f;
        float dr = targetAngle - robot.r; 
        while (dr >  M_PI) dr -= 2.0f * M_PI;
        while (dr < -M_PI) dr += 2.0f * M_PI;

        robot.updatePose(
            pid_x.calculate_error(dx, time), 
            pid_y.calculate_error(dy, time), 
            pid_r.calculate_error(dr, time),
            time
        ); 

        robot.draw(shaderProgram);
        mouseIndicator.draw(shaderProgram);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}