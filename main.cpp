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

// Backends (These connect ImGui to your window and graphics API)
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

class SwerveDrive {
private:
    GLuint VAO, VBO, EBO;
    float chassisSize = 0.25f; 

public:
    float x, y, r;
    float x_back = 0, y_back = 0, r_back = 0; 

    SwerveDrive(float startX, float startY) : x(startX), y(startY), r(0.0f) {
        // Simple unit square for the chassis
        std::vector<float> vertices = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f 
        };
        std::vector<unsigned int> indices = { 0, 1, 2, 2, 3, 0 };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void updatePose(float x_a, float y_a, float r_a, float dt) {
        float x_store = x, y_store = y, r_store = r; 
        float friction = 0.95f; // Slightly more friction for control

        x = x + (x - x_back) * friction + x_a * dt * dt; 
        y = y + (y - y_back) * friction + y_a * dt * dt;
        r = r + (r - r_back) * friction + r_a * dt * dt;

        x_back = x_store; y_back = y_store; r_back = r_store; 
    }

    void draw(GLuint shaderProgram) {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::ortho(-aspect_ratio, aspect_ratio, -1.0f, 1.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // --- DRAW BLUE BUMPERS ---
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::rotate(model, r, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(chassisSize, chassisSize, 1.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform4f(colorLoc, 0.0f, 0.0f, 0.8f, 1.0f); // FRC Blue
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // --- DRAW FRONT INDICATOR (Optional 'front' square) ---
        glm::mat4 frontModel = glm::scale(model, glm::vec3(0.2f, 0.2f, 1.0f));
        frontModel = glm::translate(frontModel, glm::vec3(0.0f, 2.0f, 0.01f)); // Shift to front edge
        
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(frontModel));
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // White indicator
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
};

class CircleIndicator {
private:
    GLuint VAO, VBO, EBO;
    int vertexCount;
    // ADJUSTED: Changed from 0.15f to 0.05f for a smaller dot
    float radius = 0.02f; 

public:
    float x, y;

    CircleIndicator(float startX, float startY) : x(startX), y(startY) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        int segments = 50; 

        // Center point
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);

        for (int i = 0; i <= segments; i++) {
            // Note: Fixed the '2f' syntax to '2.0f' for standard C++
            float angle = 2.0f * 3.14159265359f * (float)i / (float)segments;
            vertices.push_back(cosf(angle));
            vertices.push_back(sinf(angle));
            vertices.push_back(0.0f);

            if (i > 0) {
                indices.push_back(0);     
                indices.push_back(i);     
                indices.push_back(i + 1); 
            }
        }
        vertexCount = indices.size();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void draw(GLuint shaderProgram) {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc  = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc  = glGetUniformLocation(shaderProgram, "projection");

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::ortho(-aspect_ratio, aspect_ratio, -1.0f, 1.0f, -1.0f, 1.0f);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        // This scale call uses the smaller radius defined above
        model = glm::scale(model, glm::vec3(radius, radius, 1.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

class PID{

public: 

    float P, I, D; 
    float e_accum = 0; 
    float e_back = 0; 

    PID(float P, float I, float D): P(P), I(I), D(I) {
    }

    //TODO Make bound for the integral 
    //TODO Make dt an input 


    float calculate_error(float e, float dt) {
        float return_e = P * e + I * e_accum + D * (e - e_back)/dt ; 

        e_accum += e * dt; 
        e_back = e; 

        return return_e; 
    }
};

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

        // 4. ImGui New Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);

        // 5. TUNING UI
        ImGui::Begin("PID Controller Tuning");

        ImGui::SetWindowFontScale(1.2f);
        
        if (ImGui::CollapsingHeader("Translation PID")) {
            ImGui::SliderFloat("Move P", &moveGains[0], 0.0f, 10.0f);
            ImGui::SliderFloat("Move I", &moveGains[1], 0.0f, 2.0f);
            ImGui::SliderFloat("Move D", &moveGains[2], 0.0f, 10.0f);
            // Sync gains back to PID objects
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
            // Sync gains back to PID object
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

        // 6. LOGIC & MATH
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        float ndcX = ((2.0f * (float)mouseX) / width - 1.0f) * aspect_ratio;
        float ndcY = (1.0f - (2.0f * (float)mouseY) / height);

        mouseIndicator.x = ndcX;
        mouseIndicator.y = ndcY;
        
        float dx = ndcX - robot.x;
        float dy = ndcY - robot.y;
        
        // Shortest-path rotation logic
        float targetAngle = atan2(dy, dx) - 1.5708f;
        float dr = targetAngle - robot.r; 
        while (dr >  M_PI) dr -= 2.0f * M_PI;
        while (dr < -M_PI) dr += 2.0f * M_PI;

        // Apply PID outputs as accelerations in Verlet
        robot.updatePose(
            pid_x.calculate_error(dx, time), 
            pid_y.calculate_error(dy, time), 
            pid_r.calculate_error(dr, time),
            time
        ); 

        // 7. DRAWING
        robot.draw(shaderProgram);
        mouseIndicator.draw(shaderProgram);

        // 8. Render ImGui on top
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