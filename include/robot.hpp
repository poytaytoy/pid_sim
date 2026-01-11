#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class SwerveDrive {
private:
    GLuint VAO, VBO, EBO;
    float chassisSize = 0.25f;

public:
    float x, y, r;
    float x_back = 0.0f, y_back = 0.0f, r_back = 0.0f;

    SwerveDrive(float startX, float startY) : x(startX), y(startY), r(0.0f) {
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
        float friction = 0.95f;

        x = x + (x - x_back) * friction + x_a * dt * dt;
        y = y + (y - y_back) * friction + y_a * dt * dt;
        r = r + (r - r_back) * friction + r_a * dt * dt;

        x_back = x_store; 
        y_back = y_store; 
        r_back = r_store;
    }

    void draw(GLuint shaderProgram) {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        
        extern const float aspect_ratio;
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::ortho(-aspect_ratio, aspect_ratio, -1.0f, 1.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::rotate(model, r, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(chassisSize, chassisSize, 1.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform4f(colorLoc, 0.0f, 0.0f, 0.8f, 1.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glm::mat4 frontModel = glm::scale(model, glm::vec3(0.2f, 0.2f, 1.0f));
        frontModel = glm::translate(frontModel, glm::vec3(0.0f, 2.0f, 0.01f));
        
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(frontModel));
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
};