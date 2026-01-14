#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "pid.hpp"
#include "robot.hpp"
#include "1draycast.hpp"
#include "circle.hpp"

// Window Constants
const unsigned int WIDTH = 750; 
const unsigned int HEIGHT = 750; 
const float aspect_ratio = (float)WIDTH/(float)HEIGHT; 

const unsigned int WIDTH2 = 750;
const unsigned int HEIGHT2 = 750;

// Structure to hold PID gains for cleaner parameter passing
struct TuningState {
    float moveGains[3] = {1.0f, 0.0f, 0.00f};
    float turnGains[3] = {1.0f, 0.0f, 0.00f};
    float time = 0.01f;
};

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
        uniform vec4 uColor;
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

GLuint CreateRenderRayProgram() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;   // Ray position (x, y, z)
        layout (location = 1) in vec3 aColor; // Per-vertex color (r, g, b)

        out vec3 ourColor; 

        uniform mat4 model;
        uniform mat4 view; 
        uniform mat4 projection; 

        void main() {
            // Standard transformation
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            
            // Pass the vertex color to the fragment shader
            ourColor = aColor;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        in vec3 ourColor; 

        void main() {
            // Output the color received from the vertex shader
            FragColor = vec4(ourColor, 1.0);
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


void RenderUI(TuningState& state, PID& px, PID& py, PID& pr, const SwerveDrive& robot) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("PID Controller Tuning");
    ImGui::SetWindowFontScale(1.2f);

    if (ImGui::CollapsingHeader("Translation PID")) {
        ImGui::SliderFloat("Move P", &state.moveGains[0], 0.0f, 10.0f);
        ImGui::SliderFloat("Move I", &state.moveGains[1], 0.0f, 2.0f);
        ImGui::SliderFloat("Move D", &state.moveGains[2], 0.0f, 10.0f);
        px.P = py.P = state.moveGains[0];
        px.I = py.I = state.moveGains[1];
        px.D = py.D = state.moveGains[2];
    }

    if (ImGui::CollapsingHeader("Rotation PID")) {
        ImGui::SliderFloat("Turn P", &state.turnGains[0], 0.0f, 10.0f);
        ImGui::SliderFloat("Turn I", &state.turnGains[1], 0.0f, 2.0f);
        ImGui::SliderFloat("Turn D", &state.turnGains[2], 0.0f, 10.0f);
        pr.P = state.turnGains[0];
        pr.I = state.turnGains[1];
        pr.D = state.turnGains[2];
    }

    ImGui::Separator();
    ImGui::SliderFloat("Step Time", &state.time, 0.01f, 0.1f);
    ImGui::Text("Robot X: %.3f, Y: %.3f", robot.x, robot.y);
    ImGui::Text("Heading: %.3f rad", robot.r);
    
    ImGui::End();
    ImGui::Render();
}

void RenderMapWindow(GLFWwindow* window, GLuint shader, SwerveDrive& robot, CircleIndicator& indicator) {
    
    glfwMakeContextCurrent(window);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::ortho(-aspect_ratio, aspect_ratio, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));

    robot.draw(shader);
    indicator.draw(shader);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}



int main(void)
{
    if (!glfwInit()) return -1;
    
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "PID Tuning Sim (Map)", NULL, NULL);
    GLFWwindow* window2 = glfwCreateWindow(WIDTH2, HEIGHT2, "Robot View", NULL, window);
    
    if (!window || !window2) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glDisable(GL_DEPTH_TEST);
    GLuint shaderProgram = CreateShaderProgram();
    GLuint rayProgram = CreateRenderRayProgram();

    CircleIndicator mouseIndicator(0.0f, 0.0f);
    SwerveDrive robot(0.0f, 0.0f);
    TuningState state;
    // Initialize in main
    

    PID pid_x(state.moveGains[0], state.moveGains[1], state.moveGains[2]);
    PID pid_y(state.moveGains[0], state.moveGains[1], state.moveGains[2]);
    PID pid_r(state.turnGains[0], state.turnGains[1], state.turnGains[2]);

    while (!glfwWindowShouldClose(window) && !glfwWindowShouldClose(window2)) {
        glfwPollEvents();

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        float ndcX = ((2.0f * (float)mouseX) / WIDTH - 1.0f) * aspect_ratio;
        float ndcY = (1.0f - (2.0f * (float)mouseY) / HEIGHT);

        mouseIndicator.x = ndcX;
        mouseIndicator.y = ndcY;
        
        float dx = ndcX - robot.x;
        float dy = ndcY - robot.y;
        float targetAngle = atan2(dy, dx) - 1.5708f;
        float dr = targetAngle - robot.r; 
        while (dr >  M_PI) dr -= 2.0f * M_PI;
        while (dr < -M_PI) dr += 2.0f * M_PI;

        robot.updatePose(
            pid_x.calculate_error(dx, state.time), 
            pid_y.calculate_error(dy, state.time), 
            pid_r.calculate_error(dr, state.time),
            state.time
        ); 

        RenderUI(state, pid_x, pid_y, pid_r, robot);
        RenderMapWindow(window, shaderProgram, robot, mouseIndicator);

        glfwMakeContextCurrent(window2);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Raycaster raycaster(750);
        
        raycaster.updateAndDraw(rayProgram, robot.x, robot.y, robot.r);
        raycaster.drawCursor(rayProgram, robot.x, robot.y, robot.r, ndcX, ndcY);
        glfwSwapBuffers(window2);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}