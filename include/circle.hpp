#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

class CircleIndicator {
private:
    GLuint VAO, VBO, EBO;
    int vertexCount;
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

        extern const float aspect_ratio; // Required to access the global constant in main.cpp
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::ortho(-aspect_ratio, aspect_ratio, -1.0f, 1.0f, -1.0f, 1.0f);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::scale(model, glm::vec3(radius, radius, 1.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};