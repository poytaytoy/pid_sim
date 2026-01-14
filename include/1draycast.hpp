#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

class Raycaster {
private:
    GLuint VAO, VBO;
    int numRays;
    const float limitX = 1.0f;
    const float limitY = 1.0f;
    float focalLength = 0.1f; 
    float fovDegrees = 70.0f; 

public:
    Raycaster(int rays) : numRays(rays) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    void updateAndDraw(GLuint shader, float robotX, float robotY, float robotR) {
        std::vector<float> vertices;
        vertices.reserve(numRays * 6 * 6);

        float fovRad = glm::radians(fovDegrees);
        float startAngle = (robotR + 1.57079f) - (fovRad / 2.0f) ;

        for (int i = 0; i < numRays; i++) {
            float rayAngle = startAngle + ((float)i / (float)numRays) * fovRad;
            float dirX = cos(rayAngle);
            float dirY = sin(rayAngle);

            float tX = (dirX > 0) ? (limitX - robotX) / dirX : (-limitX - robotX) / dirX ;
            float tY = (dirY > 0) ? (limitY - robotY) / dirY : (-limitY - robotY) / dirY;

            float dist;
            glm::vec3 color;

            if (tX < tY) {
                dist = tX;
                color = glm::vec3(0.7f, 0.3f, 0.3f); 
            } else {
                dist = tY;
                color = glm::vec3(0.3f, 0.4f, 0.6f); 
            }

            dist *= cos(rayAngle - (robotR + 1.57079f) );

            float h = focalLength / (dist + 0.001f); 
            
            if (h > 1.0f) h = 1.0f;

            float xLeft = ((float)i / numRays) * 2.0f - 1.0f;
            float xRight = ((float)(i + 1) / numRays) * 2.0f - 1.0f;

            addQuad(vertices, xLeft, xRight, h, color);
        }

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glUseProgram(shader);
        glm::mat4 ident = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &ident[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, &ident[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &ident[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
    }


    void drawCursor(GLuint shader, float robotX, float robotY, float robotR, float mouseX, float mouseY) {
        float dx = mouseX - robotX;
        float dy = mouseY - robotY;
        float dist = sqrt(dx * dx + dy * dy);
        float angleToMouse = atan2(dy, dx);
        
        float relativeAngle = angleToMouse - (robotR + 1.5708f);
        
        while (relativeAngle > M_PI) relativeAngle -= 2.0f * M_PI;
        while (relativeAngle < -M_PI) relativeAngle += 2.0f * M_PI;

        float fovRad = glm::radians(fovDegrees);
        if (abs(relativeAngle) < fovRad / 2.0f && (dist >= 0.1)) {
            float screenX = relativeAngle / (fovRad / 2.0f);
            
            dist *= cos(relativeAngle); 
            float h = (focalLength / dist) * 0.5f; 
            float w = h; 

            std::vector<float> spriteVerts;
            glm::vec3 white(1.0f, 1.0f, 1.0f);
            addQuad(spriteVerts, screenX - w, screenX + w, h, white);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, spriteVerts.size() * sizeof(float), spriteVerts.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, spriteVerts.size() / 6);
        }
    }

private:
    void addQuad(std::vector<float>& buffer, float xL, float xR, float h, glm::vec3 c) {
        float quadVertices[] = {
            xL,  h, 0.0f, c.r, c.g, c.b,
            xL, -h, 0.0f, c.r, c.g, c.b,
            xR, -h, 0.0f, c.r, c.g, c.b,

            xL,  h, 0.0f, c.r, c.g, c.b,
            xR, -h, 0.0f, c.r, c.g, c.b,
            xR,  h, 0.0f, c.r, c.g, c.b
        };
        buffer.insert(buffer.end(), std::begin(quadVertices), std::end(quadVertices));
    }
};

