#include <../imgui/imgui.h>
#include <../imgui/imgui_impl_glfw.h>
#include <../imgui/imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
const float movementSpeed = 2.5f;

bool mouseCaptured = true;
bool firstPress = true;

unsigned int cubeShader1, cubeShader2, cubeShader3;
unsigned int sphereShader1, sphereShader2, sphereShader3;
unsigned int pyramidShader1, pyramidShader2;

struct Room1NoiseParams {
    // Cube 1 parameters
    float cubeNoiseScale = 2.0f;
    float cubeNoiseAmplitude = 0.5f;
    float cubeBaseColor[3] = {1.0f, 1.0f, 0.0f};  // Yellow

    // Sphere 1 parameters
    float sphereNoiseScale = 2.0f;
    float sphereNoiseOffset = 0.5f;
    float sphereNoiseIntensity = 0.5f;
    float sphereBaseColor[3] = {0.8f, 0.2f, 0.2f};  // Red

    // Pyramid 1 parameters
    float pyramidNoiseTurbulence = 3.0f;
    float pyramidNoiseGlow = 0.3f;
    float pyramidColorMix = 0.5f;
    float pyramidBaseColor1[3] = {1.0f, 0.3f, 0.0f};  // Orange
    float pyramidBaseColor2[3] = {1.0f, 0.8f, 0.0f};  // Yellow
} room1Params;

struct Room2NoiseParams {
    // Cube 2 parameters
    float cube2NoiseScale = 2.0f;
    float cube2NoiseIntensity = 0.4f;
    float cube2BaseColor[3] = {0.5f, 0.0f, 0.8f};  // Purple

    // Sphere 2 parameters
    float sphere2NoiseScale = 2.0f;
    float sphere2NoiseIntensity = 0.5f;
    float sphere2Lacunarity = 2.0f;
    int sphere2Octaves = 4;
    float sphere2BaseColor[3] = {0.2f, 0.5f, 0.8f};  // Light Blue

    // Pyramid 2 parameters
    float pyramid2NoiseScale = 3.0f;
    float pyramid2NoiseIntensity = 0.5f;
    float pyramid2EdgeThreshold = 0.1f;
    float pyramid2GlowStrength = 0.5f;
    float pyramid2BaseColor1[3] = {0.7f, 0.2f, 0.8f};  // Purple
    float pyramid2BaseColor2[3] = {0.2f, 0.8f, 0.7f};  // Turquoise    
} room2Params;

struct Room3NoiseParams {
    // Cube 3 parameters
    float cube3NoiseScale = 2.0f;
    float cube3NoiseIntensity = 0.5f;
    float cube3BaseColor[3] = {0.3f, 0.6f, 0.9f};  // Light Blue
    float cube3MinAlpha = 0.2f;
    float cube3MaxAlpha = 0.8f;

    // Sphere 3 parameters
    float sphere3NoiseScale = 3.0f;
    float sphere3NormalStrength = 0.5f;
    float sphere3BaseColor[3] = {0.8f, 0.2f, 0.2f};  // Red
    float sphere3Glossiness = 64.0f;
} room3Params;

std::string readShaderFile(const char* filePath) {
    std::string shaderCode;
    std::ifstream shaderFile;
    
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        shaderFile.open(filePath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filePath << std::endl;
    }
    
    return shaderCode;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!mouseCaptured) return; 
    
    if (firstMouse) {
        lastX = xposIn;
        lastY = yposIn;
        firstMouse = false;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float currentSpeed = movementSpeed * deltaTime;
    if (mouseCaptured) { 
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += currentSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= currentSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (firstPress) {
            mouseCaptured = !mouseCaptured;
            if (mouseCaptured) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            firstPress = false;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
        firstPress = true;
    }
}

unsigned int createShader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = readShaderFile(vertexPath);
    std::string fragmentCode = readShaderFile(fragmentPath);
    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void setupCorridors(unsigned int& corridorVAO, unsigned int& corridorVBO, unsigned int& corridorEBO, unsigned int& corridorShader) {
    float corridorVertices[] = {
        // First corridor (between rooms 1 and 2)
        // Front wall 
        5.0f, -5.0f,  0.8f,   0.0f, 0.0f, -1.0f,  
        10.0f, -5.0f,  0.8f,  0.0f, 0.0f, -1.0f,
        10.0f, -2.5f,  0.8f,  0.0f, 0.0f, -1.0f,
        5.0f, -2.5f,  0.8f,   0.0f, 0.0f, -1.0f,

        // Back wall 
        5.0f, -5.0f, -0.8f,   0.0f, 0.0f, 1.0f,  
        10.0f, -5.0f, -0.8f,  0.0f, 0.0f, 1.0f,
        10.0f, -2.5f, -0.8f,  0.0f, 0.0f, 1.0f,
        5.0f, -2.5f, -0.8f,   0.0f, 0.0f, 1.0f,

        // Ceiling
        5.0f, -2.5f, -0.8f,   0.0f, -1.0f, 0.0f,
        10.0f, -2.5f, -0.8f,  0.0f, -1.0f, 0.0f,
        10.0f, -2.5f,  0.8f,  0.0f, -1.0f, 0.0f,
        5.0f, -2.5f,  0.8f,   0.0f, -1.0f, 0.0f,

        // Floor
        5.0f, -5.0f, -0.8f,   0.0f, 1.0f, 0.0f,
        10.0f, -5.0f, -0.8f,  0.0f, 1.0f, 0.0f,
        10.0f, -5.0f,  0.8f,  0.0f, 1.0f, 0.0f,
        5.0f, -5.0f,  0.8f,   0.0f, 1.0f, 0.0f,

        // Second corridor (between rooms 2 and 3)
        // Front wall 
        20.0f, -5.0f,  0.8f,   0.0f, 0.0f, -1.0f,
        25.0f, -5.0f,  0.8f,   0.0f, 0.0f, -1.0f,
        25.0f, -2.5f,  0.8f,   0.0f, 0.0f, -1.0f,
        20.0f, -2.5f,  0.8f,   0.0f, 0.0f, -1.0f,

        // Back wall 
        20.0f, -5.0f, -0.8f,   0.0f, 0.0f, 1.0f,
        25.0f, -5.0f, -0.8f,   0.0f, 0.0f, 1.0f,
        25.0f, -2.5f, -0.8f,   0.0f, 0.0f, 1.0f,
        20.0f, -2.5f, -0.8f,   0.0f, 0.0f, 1.0f,

        // Ceiling
        20.0f, -2.5f, -0.8f,   0.0f, -1.0f, 0.0f,
        25.0f, -2.5f, -0.8f,   0.0f, -1.0f, 0.0f,
        25.0f, -2.5f,  0.8f,   0.0f, -1.0f, 0.0f,
        20.0f, -2.5f,  0.8f,   0.0f, -1.0f, 0.0f,

        // Floor
        20.0f, -5.0f, -0.8f,   0.0f, 1.0f, 0.0f,
        25.0f, -5.0f, -0.8f,   0.0f, 1.0f, 0.0f,
        25.0f, -5.0f,  0.8f,   0.0f, 1.0f, 0.0f,
        20.0f, -5.0f,  0.8f,   0.0f, 1.0f, 0.0f
    };

    unsigned int corridorIndices[] = {
        // First corridor
        0, 1, 2,    0, 2, 3,    
        4, 5, 6,    4, 6, 7,    
        8, 9, 10,   8, 10, 11,  
        12, 13, 14, 12, 14, 15, 
        
        // Second corridor
        16, 17, 18, 16, 18, 19, 
        20, 21, 22, 20, 22, 23, 
        24, 25, 26, 24, 26, 27, 
        28, 29, 30, 28, 30, 31  
    };

    glGenVertexArrays(1, &corridorVAO);
    glGenBuffers(1, &corridorVBO);
    glGenBuffers(1, &corridorEBO);

    glBindVertexArray(corridorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, corridorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(corridorVertices), corridorVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, corridorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(corridorIndices), corridorIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    corridorShader = createShader("../shaders/vertex_corridor.glsl", "../shaders/fragment_corridor.glsl");
}

void renderCorridors(unsigned int corridorVAO, unsigned int corridorShader, 
                    const glm::mat4& view, const glm::mat4& projection, 
                    const glm::vec3& cameraPos) {
    glUseProgram(corridorShader);
    
    glUniform3f(glGetUniformLocation(corridorShader, "lightPos"), 0.0f, 10.0f, 0.0f);
    glUniform3f(glGetUniformLocation(corridorShader, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
    
    glm::mat4 corridorModel = glm::mat4(1.0f);
    
    glUniformMatrix4fv(glGetUniformLocation(corridorShader, "model"), 1, GL_FALSE, glm::value_ptr(corridorModel));
    glUniformMatrix4fv(glGetUniformLocation(corridorShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(corridorShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(corridorVAO);
    glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
}

void cleanupCorridors(unsigned int corridorVAO, unsigned int corridorVBO, 
                     unsigned int corridorEBO, unsigned int corridorShader) {
    glDeleteVertexArrays(1, &corridorVAO);
    glDeleteBuffers(1, &corridorVBO);
    glDeleteBuffers(1, &corridorEBO);
    glDeleteProgram(corridorShader);
}

// Cube functions
void setupCube(unsigned int& cubeVAO, unsigned int& cubeVBO, unsigned int& cubeEBO) {
    float cubeVertices[] = {
        
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    unsigned int cubeIndices[] = {
        0, 1, 2,    0, 2, 3,   
        4, 5, 6,    4, 6, 7,   
        8, 9, 10,   8, 10, 11, 
        12,13, 14,  12,14, 15, 
        16,17, 18,  16,18, 19, 
        20,21, 22,  20,22, 23  
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    cubeShader1 = createShader("../shaders/vertex_cube1.glsl", "../shaders/fragment_cube1.glsl");
    cubeShader2 = createShader("../shaders/vertex_cube2.glsl", "../shaders/fragment_cube2.glsl");
    cubeShader3 = createShader("../shaders/vertex_cube3.glsl", "../shaders/fragment_cube3.glsl");

}

void renderCube(unsigned int cubeVAO, int room, const glm::mat4& view, const glm::mat4& projection, 
                const glm::vec3& cameraPos, const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f)) {
    unsigned int shader;
    if (room == 1) {
        shader = cubeShader1;
        glUseProgram(cubeShader1);
        glUniform1f(glGetUniformLocation(cubeShader1, "noiseScale"), room1Params.cubeNoiseScale);
        glUniform1f(glGetUniformLocation(cubeShader1, "noiseAmplitude"), room1Params.cubeNoiseAmplitude);
        glUniform3fv(glGetUniformLocation(cubeShader1, "baseColor"), 1, room1Params.cubeBaseColor);
    } else if (room == 2) {
        shader = cubeShader2;
        glUseProgram(cubeShader2);
        glUniform1f(glGetUniformLocation(cubeShader2, "noiseScale"), room2Params.cube2NoiseScale);
        glUniform1f(glGetUniformLocation(cubeShader2, "noiseIntensity"), room2Params.cube2NoiseIntensity);
        glUniform3fv(glGetUniformLocation(cubeShader2, "baseColor"), 1, room2Params.cube2BaseColor);
    }else {
        shader = cubeShader3;
        glUseProgram(cubeShader3);
        glUniform1f(glGetUniformLocation(cubeShader3, "noiseScale"), room3Params.cube3NoiseScale);
        glUniform1f(glGetUniformLocation(cubeShader3, "noiseIntensity"), room3Params.cube3NoiseIntensity);
        glUniform3fv(glGetUniformLocation(cubeShader3, "baseColor"), 1, room3Params.cube3BaseColor);
        glUniform1f(glGetUniformLocation(cubeShader3, "minAlpha"), room3Params.cube3MinAlpha);
        glUniform1f(glGetUniformLocation(cubeShader3, "maxAlpha"), room3Params.cube3MaxAlpha);
    }

    glUseProgram(shader);
    
    glUniform3f(glGetUniformLocation(shader, "lightPos"), 0.0f, 10.0f, 0.0f);
    glUniform3f(glGetUniformLocation(shader, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);  
    
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void cleanupCube(unsigned int cubeVAO, unsigned int cubeVBO, unsigned int cubeEBO) {
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);
    glDeleteProgram(cubeShader1);
    glDeleteProgram(cubeShader2);
    glDeleteProgram(cubeShader3);
}

// Sphere functions
void setupSphere(unsigned int& sphereVAO, unsigned int& sphereVBO, unsigned int& sphereEBO, std::vector<unsigned int>& sphereIndices) {
    std::vector<float> sphereVertices;
    const unsigned int X_SEGMENTS = 32;
    const unsigned int Y_SEGMENTS = 32;
    const float PI = 3.14159265359f;
    
    for(unsigned int y = 0; y <= Y_SEGMENTS; y++) {
        for(unsigned int x = 0; x <= X_SEGMENTS; x++) {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            
            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);
            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);
        }
    }
    
    // Generate indices
    for(unsigned int y = 0; y < Y_SEGMENTS; y++) {
        for(unsigned int x = 0; x < X_SEGMENTS; x++) {
            sphereIndices.push_back(y * (X_SEGMENTS + 1) + x);
            sphereIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            sphereIndices.push_back(y * (X_SEGMENTS + 1) + x + 1);
            sphereIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            sphereIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
            sphereIndices.push_back(y * (X_SEGMENTS + 1) + x + 1);
        }
    }

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    sphereShader1 = createShader("../shaders/vertex_sphere1.glsl", "../shaders/fragment_sphere1.glsl");
    sphereShader2 = createShader("../shaders/vertex_sphere2.glsl", "../shaders/fragment_sphere2.glsl");
    sphereShader3 = createShader("../shaders/vertex_sphere3.glsl", "../shaders/fragment_sphere3.glsl"); 

}

void renderSphere(unsigned int sphereVAO, int room, const glm::mat4& view, const glm::mat4& projection,
                 const glm::vec3& cameraPos, const std::vector<unsigned int>& indices,
                 const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f)) {
    unsigned int shader;
    if (room == 1) {
        shader = sphereShader1;
        glUseProgram(sphereShader1);
        glUniform1f(glGetUniformLocation(sphereShader1, "noiseScale"), room1Params.sphereNoiseScale);
        glUniform1f(glGetUniformLocation(sphereShader1, "noiseOffset"), room1Params.sphereNoiseOffset);
        glUniform1f(glGetUniformLocation(sphereShader1, "noiseIntensity"), room1Params.sphereNoiseIntensity);
        glUniform3fv(glGetUniformLocation(sphereShader1, "baseColor"), 1, room1Params.sphereBaseColor);
    }else if (room == 2) {
        shader = sphereShader2;
        glUseProgram(sphereShader2);
        glUniform1f(glGetUniformLocation(sphereShader2, "noiseScale"), room2Params.sphere2NoiseScale);
        glUniform1f(glGetUniformLocation(sphereShader2, "noiseIntensity"), room2Params.sphere2NoiseIntensity);
        glUniform1f(glGetUniformLocation(sphereShader2, "lacunarity"), room2Params.sphere2Lacunarity);
        glUniform1i(glGetUniformLocation(sphereShader2, "octaves"), room2Params.sphere2Octaves);
        glUniform3fv(glGetUniformLocation(sphereShader2, "baseColor"), 1, room2Params.sphere2BaseColor);
    }else {
        shader = sphereShader3;
        glUseProgram(sphereShader3);
        glUniform1f(glGetUniformLocation(sphereShader3, "noiseScale"), room3Params.sphere3NoiseScale);
        glUniform1f(glGetUniformLocation(sphereShader3, "normalStrength"), room3Params.sphere3NormalStrength);
        glUniform3fv(glGetUniformLocation(sphereShader3, "baseColor"), 1, room3Params.sphere3BaseColor);
        glUniform1f(glGetUniformLocation(sphereShader3, "glossiness"), room3Params.sphere3Glossiness);
    }

    glUseProgram(shader);
    
    glUniform3f(glGetUniformLocation(shader, "lightPos"), 0.0f, 10.0f, 0.0f);
    glUniform3f(glGetUniformLocation(shader, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);  
    
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void cleanupSphere(unsigned int sphereVAO, unsigned int sphereVBO, unsigned int sphereEBO) {
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteProgram(sphereShader1);
    glDeleteProgram(sphereShader2);
    glDeleteProgram(sphereShader3);
}

// Pyramid functions
void setupPyramid(unsigned int& pyramidVAO, unsigned int& pyramidVBO, unsigned int& pyramidEBO) {
    float pyramidVertices[] = {

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        
        -0.5f, -0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,
         0.0f,  0.5f,  0.0f,  -0.5f, 0.5f,  0.5f,
        
         0.5f, -0.5f,  0.5f,   0.5f, 0.5f,  0.0f,
         0.5f, -0.5f, -0.5f,   0.5f, 0.5f,  0.0f,
         0.0f,  0.5f,  0.0f,   0.5f, 0.5f,  0.0f,
        
         0.5f, -0.5f, -0.5f,   0.0f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,   0.0f, 0.5f, -0.5f,
         0.0f,  0.5f,  0.0f,   0.0f, 0.5f, -0.5f,
        
        -0.5f, -0.5f, -0.5f,  -0.5f, 0.5f,  0.0f,
        -0.5f, -0.5f,  0.5f,  -0.5f, 0.5f,  0.0f,
         0.0f,  0.5f,  0.0f,  -0.5f, 0.5f,  0.0f,
    };

    unsigned int pyramidIndices[] = {
        0, 1, 2,    
        0, 2, 3,
        4, 5, 6,    
        7, 8, 9,    
        10, 11, 12, 
        13, 14, 15  
    };

    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glGenBuffers(1, &pyramidEBO);

    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    pyramidShader1 = createShader("../shaders/vertex_pyramid1.glsl", "../shaders/fragment_pyramid1.glsl");
    pyramidShader2 = createShader("../shaders/vertex_pyramid2.glsl", "../shaders/fragment_pyramid2.glsl");

}

void renderPyramid(unsigned int pyramidVAO, int room, const glm::mat4& view, const glm::mat4& projection,
                  const glm::vec3& cameraPos, const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f)) {
    unsigned int shader;
    if (room == 1) {
        shader = pyramidShader1;
        glUseProgram(pyramidShader1);
        glUniform1f(glGetUniformLocation(pyramidShader1, "noiseTurbulence"), room1Params.pyramidNoiseTurbulence);
        glUniform1f(glGetUniformLocation(pyramidShader1, "noiseGlow"), room1Params.pyramidNoiseGlow);
        glUniform1f(glGetUniformLocation(pyramidShader1, "colorMix"), room1Params.pyramidColorMix);
        glUniform3fv(glGetUniformLocation(pyramidShader1, "baseColor1"), 1, room1Params.pyramidBaseColor1);
        glUniform3fv(glGetUniformLocation(pyramidShader1, "baseColor2"), 1, room1Params.pyramidBaseColor2);
    }else if (room == 2) {
        shader = pyramidShader2;
        glUseProgram(pyramidShader2);
        glUniform1f(glGetUniformLocation(pyramidShader2, "noiseScale"), room2Params.pyramid2NoiseScale);
        glUniform1f(glGetUniformLocation(pyramidShader2, "noiseIntensity"), room2Params.pyramid2NoiseIntensity);
        glUniform1f(glGetUniformLocation(pyramidShader2, "edgeThreshold"), room2Params.pyramid2EdgeThreshold);
        glUniform1f(glGetUniformLocation(pyramidShader2, "glowStrength"), room2Params.pyramid2GlowStrength);
        glUniform3fv(glGetUniformLocation(pyramidShader2, "baseColor1"), 1, room2Params.pyramid2BaseColor1);
        glUniform3fv(glGetUniformLocation(pyramidShader2, "baseColor2"), 1, room2Params.pyramid2BaseColor2);
    }

    glUseProgram(shader);
    
    glUniform3f(glGetUniformLocation(shader, "lightPos"), 0.0f, 10.0f, 0.0f);
    glUniform3f(glGetUniformLocation(shader, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale); 
    
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(pyramidVAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
}

void cleanupPyramid(unsigned int pyramidVAO, unsigned int pyramidVBO, unsigned int pyramidEBO) {
    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteBuffers(1, &pyramidVBO);
    glDeleteBuffers(1, &pyramidEBO);
    glDeleteProgram(pyramidShader1);
    glDeleteProgram(pyramidShader2);
}

void setupDoorFrames(unsigned int& doorVAO, unsigned int& doorVBO, unsigned int& doorEBO, unsigned int& doorShader) {
    float vertices[] = {
        // Above door section for Room 1 right wall
        5.0f, -2.5f, -0.8f,     
        5.0f, -2.5f,  0.8f,     
        5.0f,  5.0f, -0.8f,     
        5.0f,  5.0f,  0.8f,     

        // Above door section for Room 2 left wall
        10.0f, -2.5f, -0.8f,    
        10.0f, -2.5f,  0.8f,   
        10.0f,  5.0f, -0.8f,    
        10.0f,  5.0f,  0.8f,   

        // Above door section for Room 2 right wall
        20.0f, -2.5f, -0.8f,   
        20.0f, -2.5f,  0.8f,    
        20.0f,  5.0f, -0.8f,    
        20.0f,  5.0f,  0.8f,    

        // Above door section for Room 3 left wall
        25.0f, -2.5f, -0.8f,    
        25.0f, -2.5f,  0.8f,    
        25.0f,  5.0f, -0.8f,   
        25.0f,  5.0f,  0.8f     
    };

    unsigned int indices[] = {
        // Room 1 right wall
        0, 1, 2,
        1, 3, 2,

        // Room 2 left wall
        4, 5, 6,
        5, 7, 6,

        // Room 2 right wall
        8, 9, 10,
        9, 11, 10,

        // Room 3 left wall
        12, 13, 14,
        13, 15, 14
    };

    glGenVertexArrays(1, &doorVAO);
    glGenBuffers(1, &doorVBO);
    glGenBuffers(1, &doorEBO);

    glBindVertexArray(doorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, doorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, doorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    doorShader = createShader("../shaders/vertex_left.glsl", "../shaders/fragment_left.glsl");
}

void renderDoorFrames(unsigned int doorVAO, unsigned int doorShader, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(doorShader);
    glUniformMatrix4fv(glGetUniformLocation(doorShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(doorShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(doorShader, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(doorVAO);
    
    glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
}

void cleanupDoorFrames(unsigned int doorVAO, unsigned int doorVBO, unsigned int doorEBO, unsigned int doorShader) {
    glDeleteVertexArrays(1, &doorVAO);
    glDeleteBuffers(1, &doorVBO);
    glDeleteBuffers(1, &doorEBO);
    glDeleteProgram(doorShader);
}

void setupRooms(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO, unsigned int shaderPrograms[]) {
    float vertices[] = {
        // Room 1
        // Front face
        -5.0f, -5.0f,  5.0f,    
        5.0f, -5.0f,  5.0f,     
        5.0f,  5.0f,  5.0f,    
        -5.0f,  5.0f,  5.0f,   

        // Back face
        -5.0f, -5.0f, -5.0f,    
        5.0f, -5.0f, -5.0f,     
        5.0f,  5.0f, -5.0f,     
        -5.0f,  5.0f, -5.0f,   

        // Left face
        -5.0f, -5.0f, -5.0f,   
        -5.0f, -5.0f,  5.0f,    
        -5.0f,  5.0f,  5.0f,    
        -5.0f,  5.0f, -5.0f,    

        // Right face (with door)
        5.0f, -5.0f, -5.0f,     
        5.0f, -5.0f, -0.8f,     
        5.0f, -5.0f,  0.8f,     
        5.0f, -5.0f,  5.0f,     
        5.0f, -2.5f, -0.8f,     
        5.0f, -2.5f,  0.8f,     
        5.0f,  5.0f, -5.0f,     
        5.0f,  5.0f, -0.8f,    
        5.0f,  5.0f,  0.8f,    
        5.0f,  5.0f,  5.0f,     

        // Room 2
        // Front face 
        10.0f, -5.0f,  5.0f,    
        20.0f, -5.0f,  5.0f,   
        20.0f,  5.0f,  5.0f,    
        10.0f,  5.0f,  5.0f,   

        // Back face
        10.0f, -5.0f, -5.0f,    
        20.0f, -5.0f, -5.0f,    
        20.0f,  5.0f, -5.0f,    
        10.0f,  5.0f, -5.0f,    

        // Left face (with door)
        10.0f, -5.0f, -5.0f,    
        10.0f, -5.0f, -0.8f,    
        10.0f, -5.0f,  0.8f,    
        10.0f, -5.0f,  5.0f,    
        10.0f, -2.5f, -0.8f,    
        10.0f, -2.5f,  0.8f,    
        10.0f,  5.0f, -5.0f,    
        10.0f,  5.0f, -0.8f,    
        10.0f,  5.0f,  0.8f,    
        10.0f,  5.0f,  5.0f,    

        // Right face (with door)
        20.0f, -5.0f, -5.0f,    
        20.0f, -5.0f, -0.8f,    
        20.0f, -5.0f,  0.8f,    
        20.0f, -5.0f,  5.0f,    
        20.0f, -2.5f, -0.8f,    
        20.0f, -2.5f,  0.8f,    
        20.0f,  5.0f, -5.0f,    
        20.0f,  5.0f, -0.8f,    
        20.0f,  5.0f,  0.8f,    
        20.0f,  5.0f,  5.0f,   

        // Room 3
        // Front face 
        25.0f, -5.0f,  5.0f,    
        35.0f, -5.0f,  5.0f,    
        35.0f,  5.0f,  5.0f,    
        25.0f,  5.0f,  5.0f,    

        // Back face
        25.0f, -5.0f, -5.0f,   
        35.0f, -5.0f, -5.0f,    
        35.0f,  5.0f, -5.0f,    
        25.0f,  5.0f, -5.0f,    

        // Left face (with door)
        25.0f, -5.0f, -5.0f,    
        25.0f, -5.0f, -0.8f,    
        25.0f, -5.0f,  0.8f,    
        25.0f, -5.0f,  5.0f,    
        25.0f, -2.5f, -0.8f,    
        25.0f, -2.5f,  0.8f,    
        25.0f,  5.0f, -5.0f,    
        25.0f,  5.0f, -0.8f,    
        25.0f,  5.0f,  0.8f,    
        25.0f,  5.0f,  5.0f,    

        // Right face 
        35.0f, -5.0f, -5.0f,    
        35.0f, -5.0f,  5.0f,    
        35.0f,  5.0f,  5.0f,    
        35.0f,  5.0f, -5.0f,    

        // Ceilings
        // Room 1 ceiling
        -5.0f,  5.0f, -5.0f,     
        5.0f,  5.0f, -5.0f,     
        5.0f,  5.0f,  5.0f,     
        -5.0f,  5.0f,  5.0f,    

        // Room 2 ceiling
        10.0f,  5.0f, -5.0f,    
        20.0f,  5.0f, -5.0f,    
        20.0f,  5.0f,  5.0f,    
        10.0f,  5.0f,  5.0f,            

        // Room 3 ceiling
        25.0f,  5.0f, -5.0f,    
        35.0f,  5.0f, -5.0f,    
        35.0f,  5.0f,  5.0f,    
        25.0f,  5.0f,  5.0f,    

        // Floors
        // Room 1 floor
        -5.0f, -5.0f, -5.0f,    
        5.0f, -5.0f, -5.0f,     
        5.0f, -5.0f,  5.0f,     
        -5.0f, -5.0f,  5.0f,    

        // Room 2 floor
        10.0f, -5.0f, -5.0f,   
        20.0f, -5.0f, -5.0f,    
        20.0f, -5.0f,  5.0f,    
        10.0f, -5.0f,  5.0f,    

        // Room 3 floor
        25.0f, -5.0f, -5.0f,    
        35.0f, -5.0f, -5.0f,    
        35.0f, -5.0f,  5.0f,    
        25.0f, -5.0f,  5.0f     

    };

    unsigned int indices[] = {
        // Room 1
        // Front face 
        0, 1, 2,
        0, 2, 3,

        // Back face
        4, 5, 6,
        4, 6, 7,

        // Left face
        8, 9, 10,
        8, 10, 11,

        // Right face (with door)
        12, 13, 16,    // Bottom left section
        12, 16, 18,    
        14, 15, 21,    // Bottom right section
        14, 21, 20,    
        16, 19, 18,    // Top section
        17, 20, 21,    

        // Room 2
        // Front face 
        22, 23, 24,
        22, 24, 25,

        // Back face
        26, 27, 28,
        26, 28, 29,

        // Left face (with door)
        30, 31, 34,    // Bottom left section
        30, 34, 36,    
        32, 33, 39,    // Bottom right section
        32, 39, 38,    
        34, 37, 36,    // Top section
        35, 38, 39,    

        // Right face (with door)
        40, 41, 44,    // Bottom left section
        40, 44, 46,    
        42, 43, 49,    // Bottom right section
        42, 49, 48,    
        44, 47, 46,    // Top section
        45, 48, 49,    

         // Room 3
        // Front face 
        50, 51, 52,
        50, 52, 53,

        // Back face
        54, 55, 56,
        54, 56, 57,

        // Left face (with door)
        58, 59, 62,    // Bottom left section
        58, 62, 64,    
        60, 61, 67,    // Bottom right section
        60, 67, 66,    
        62, 65, 64,    // Top section
        63, 66, 67,    

        // Right face 
        68, 69, 70,
        68, 70, 71,

        // Ceilings
        // Room 1
        72, 73, 74,
        72, 74, 75,

        // Room 2
        76, 77, 78,
        76, 78, 79,

        // Room 3
        80, 81, 82,
        80, 82, 83,

        // Floors
        // Room 1
        84, 85, 86,
        84, 86, 87,

        // Room 2
        88, 89, 90,
        88, 90, 91,

        // Room 3
        92, 93, 94,
        92, 94, 95
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    shaderPrograms[0] = createShader("../shaders/vertex_front.glsl", "../shaders/fragment_front.glsl");
    shaderPrograms[1] = createShader("../shaders/vertex_back.glsl", "../shaders/fragment_back.glsl");
    shaderPrograms[2] = createShader("../shaders/vertex_left.glsl", "../shaders/fragment_left.glsl");
    shaderPrograms[3] = createShader("../shaders/vertex_right.glsl", "../shaders/fragment_right.glsl");
    shaderPrograms[4] = createShader("../shaders/vertex_top.glsl", "../shaders/fragment_top.glsl");
    shaderPrograms[5] = createShader("../shaders/vertex_bottom.glsl", "../shaders/fragment_bottom.glsl");
}

void renderRooms(unsigned int VAO, unsigned int shaderPrograms[], const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 model = glm::mat4(1.0f);

    // Room 1
    // Front face 
    glUseProgram(shaderPrograms[0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[0], "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[0], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[0], "model"), 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Back face
    glUseProgram(shaderPrograms[1]);
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[1], "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[1], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[1], "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int)));

    // Left face
    glUseProgram(shaderPrograms[2]);
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[2], "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[2], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[2], "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int)));

    // Right face (with door)
    glUseProgram(shaderPrograms[3]);
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[3], "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[3], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[3], "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, (void*)(18 * sizeof(unsigned int)));

    // Room 2
    // Front face 
    glUseProgram(shaderPrograms[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(36 * sizeof(unsigned int)));

    // Back face
    glUseProgram(shaderPrograms[1]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(42 * sizeof(unsigned int)));

    // Left face (with door)
    glUseProgram(shaderPrograms[2]);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, (void*)(48 * sizeof(unsigned int)));

    // Right face (with door)
    glUseProgram(shaderPrograms[3]);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, (void*)(66 * sizeof(unsigned int)));

    // Room 3
    // Front face 
    glUseProgram(shaderPrograms[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(84 * sizeof(unsigned int)));

    // Back face
    glUseProgram(shaderPrograms[1]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(90 * sizeof(unsigned int)));

    // Left face (with door)
    glUseProgram(shaderPrograms[2]);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, (void*)(96 * sizeof(unsigned int)));

    // Right face 
    glUseProgram(shaderPrograms[3]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(114 * sizeof(unsigned int)));

    // Ceilings
    glUseProgram(shaderPrograms[4]);
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[4], "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[4], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[4], "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    // Room 1 ceiling
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(120 * sizeof(unsigned int)));
    // Room 2 ceiling
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(126 * sizeof(unsigned int)));
    // Room 3 ceiling
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(132 * sizeof(unsigned int)));

    // Floors
    glUseProgram(shaderPrograms[5]);
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[5], "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[5], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[5], "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    // Room 1 floor
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(138 * sizeof(unsigned int)));
    // Room 2 floor
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(144 * sizeof(unsigned int)));
    // Room 3 floor
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(150 * sizeof(unsigned int)));
}

void cleanupRooms(unsigned int VAO, unsigned int VBO, unsigned int EBO, unsigned int shaderPrograms[]) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    for (int i = 0; i < 6; i++) {
        glDeleteProgram(shaderPrograms[i]);
    }
}

void renderObjects(unsigned int cubeVAO, unsigned int sphereVAO, unsigned int pyramidVAO,
                  const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos,
                  const std::vector<unsigned int>& sphereIndices) {
    
    // Room 1 objects 
    renderCube(cubeVAO, 1, view, projection, cameraPos, glm::vec3(-3.0f, -3.0f, 3.0f));
    renderSphere(sphereVAO, 1, view, projection, cameraPos, sphereIndices, glm::vec3(0.0f, -3.0f, 0.0f));
    renderPyramid(pyramidVAO, 1, view, projection, cameraPos, glm::vec3(-3.0f, -3.0f, -3.0f));

    // Room 2 objects 
    renderCube(cubeVAO, 2, view, projection, cameraPos, glm::vec3(12.0f, -3.0f, 3.0f));
    renderSphere(sphereVAO, 2, view, projection, cameraPos, sphereIndices, glm::vec3(15.0f, -3.0f, 0.0f));
    renderPyramid(pyramidVAO, 2, view, projection, cameraPos, glm::vec3(18.0f, -3.0f, -3.0f));

    // Room 3 objects 
    renderCube(cubeVAO, 3, view, projection, cameraPos, 
              glm::vec3(32.0f, -3.0f, 2.5f),    
              glm::vec3(2.5f));

    renderSphere(sphereVAO, 3, view, projection, cameraPos, sphereIndices, 
                glm::vec3(32.0f, -3.0f, -2.5f), 
                glm::vec3(1.75f));
}

void renderNoiseControls() {

    ImGui::Begin("Noise Controls");
    
    if (ImGui::CollapsingHeader("Room 1")) {
        if (ImGui::CollapsingHeader("Cube Parameters (Perlin Noise with Octaves)")) {
            ImGui::SliderFloat("Cube Noise Scale", &room1Params.cubeNoiseScale, 0.1f, 5.0f);
            ImGui::SliderFloat("Cube Noise Amplitude", &room1Params.cubeNoiseAmplitude, 0.0f, 1.0f);
            ImGui::ColorEdit3("Cube Color", room1Params.cubeBaseColor);
        }
        
        if (ImGui::CollapsingHeader("Sphere Parameters (simple Perlin Noise)")) {
            ImGui::SliderFloat("Sphere Noise Scale", &room1Params.sphereNoiseScale, 0.1f, 5.0f);
            ImGui::SliderFloat("Sphere Noise Offset", &room1Params.sphereNoiseOffset, 0.0f, 1.0f);
            ImGui::SliderFloat("Sphere Noise Intensity", &room1Params.sphereNoiseIntensity, 0.0f, 1.0f);
            ImGui::ColorEdit3("Sphere Color", room1Params.sphereBaseColor);
        }
        
        if (ImGui::CollapsingHeader("Pyramid Parameters (Perlin Noise with Turbulence)")) {
            ImGui::SliderFloat("Pyramid Turbulence", &room1Params.pyramidNoiseTurbulence, 0.1f, 10.0f);
            ImGui::SliderFloat("Pyramid Glow", &room1Params.pyramidNoiseGlow, 0.0f, 1.0f);
            ImGui::SliderFloat("Pyramid Color Mix", &room1Params.pyramidColorMix, 0.0f, 1.0f);
            ImGui::ColorEdit3("Pyramid Color 1", room1Params.pyramidBaseColor1);
            ImGui::ColorEdit3("Pyramid Color 2", room1Params.pyramidBaseColor2);
        }
    }

    if (ImGui::CollapsingHeader("Room 2")) {
        if (ImGui::CollapsingHeader("Cube Parameters (Simplex Noise)")) {
            ImGui::SliderFloat("Cube Noise Scale", &room2Params.cube2NoiseScale, 0.1f, 5.0f);
            ImGui::SliderFloat("Cube Noise Intensity", &room2Params.cube2NoiseIntensity, 0.0f, 1.0f);
            ImGui::ColorEdit3("Cube Color", room2Params.cube2BaseColor);
        }

        if (ImGui::CollapsingHeader("Sphere Parameters (Multifractal Noise)")) {
            ImGui::SliderFloat("Sphere Noise Scale", &room2Params.sphere2NoiseScale, 0.1f, 5.0f);
            ImGui::SliderFloat("Sphere Noise Intensity", &room2Params.sphere2NoiseIntensity, 0.0f, 1.0f);
            ImGui::SliderFloat("Sphere Lacunarity", &room2Params.sphere2Lacunarity, 1.0f, 4.0f);
            ImGui::SliderInt("Sphere Octaves", &room2Params.sphere2Octaves, 1, 8);
            ImGui::ColorEdit3("Sphere Color", room2Params.sphere2BaseColor);
        }

        if (ImGui::CollapsingHeader("Pyramid Parameters (Cellular Noise)")) {
            ImGui::SliderFloat("Pyramid Noise Scale", &room2Params.pyramid2NoiseScale, 1.0f, 10.0f);
            ImGui::SliderFloat("Pyramid Noise Intensity", &room2Params.pyramid2NoiseIntensity, 0.0f, 1.0f);
            ImGui::SliderFloat("Pyramid Edge Threshold", &room2Params.pyramid2EdgeThreshold, 0.01f, 0.2f);
            ImGui::SliderFloat("Pyramid Glow Strength", &room2Params.pyramid2GlowStrength, 0.0f, 1.0f);
            ImGui::ColorEdit3("Pyramid Color 1", room2Params.pyramid2BaseColor1);
            ImGui::ColorEdit3("Pyramid Color 2", room2Params.pyramid2BaseColor2);
        }
    }

    if (ImGui::CollapsingHeader("Room 3")) {
        if (ImGui::CollapsingHeader("Cube Parameters (Perlin Noise on Transparency)")) {
            ImGui::SliderFloat("Cube Noise Scale", &room3Params.cube3NoiseScale, 0.1f, 5.0f);
            ImGui::SliderFloat("Cube Noise Intensity", &room3Params.cube3NoiseIntensity, 0.0f, 1.0f);
            ImGui::ColorEdit3("Cube Color", room3Params.cube3BaseColor);
            ImGui::SliderFloat("Cube Min Alpha", &room3Params.cube3MinAlpha, 0.0f, 1.0f);
            ImGui::SliderFloat("Cube Max Alpha", &room3Params.cube3MaxAlpha, 0.0f, 1.0f);
        }

        if (ImGui::CollapsingHeader("Sphere Parameters (Perlin Noise on Normal Mapping)")) {
            ImGui::SliderFloat("Sphere Noise Scale", &room3Params.sphere3NoiseScale, 0.1f, 10.0f);
            ImGui::SliderFloat("Sphere Normal Strength", &room3Params.sphere3NormalStrength, 0.0f, 2.0f);
            ImGui::ColorEdit3("Sphere Color", room3Params.sphere3BaseColor);
            ImGui::SliderFloat("Sphere Glossiness", &room3Params.sphere3Glossiness, 1.0f, 128.0f);
        }
    }
    ImGui::End();
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Room", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure window and callbacks
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glEnable(GL_DEPTH_TEST);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int roomVAO, roomVBO, roomEBO;
    unsigned int roomShaders[6];
    setupRooms(roomVAO, roomVBO, roomEBO, roomShaders);

    unsigned int sphereVAO, sphereVBO, sphereEBO;
    std::vector<unsigned int> sphereIndices;
    setupSphere(sphereVAO, sphereVBO, sphereEBO, sphereIndices);

    unsigned int pyramidVAO, pyramidVBO, pyramidEBO;
    setupPyramid(pyramidVAO, pyramidVBO, pyramidEBO);    

    unsigned int cubeVAO, cubeVBO, cubeEBO;
    setupCube(cubeVAO, cubeVBO, cubeEBO);

    unsigned int corridorVAO, corridorVBO, corridorEBO, corridorShader;
    setupCorridors(corridorVAO, corridorVBO, corridorEBO, corridorShader);

    unsigned int doorVAO, doorVBO, doorEBO, doorShader;
    setupDoorFrames(doorVAO, doorVBO, doorEBO, doorShader);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render GUI
        renderNoiseControls();

        // Render scene
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

        // Render rooms, corridors, door frames, and objects
        renderRooms(roomVAO, roomShaders, view, projection);
        renderCorridors(corridorVAO, corridorShader, view, projection, cameraPos);
        renderDoorFrames(doorVAO, doorShader, view, projection);
        renderObjects(cubeVAO, sphereVAO, pyramidVAO, view, projection, cameraPos, sphereIndices);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup
    cleanupDoorFrames(doorVAO, doorVBO, doorEBO, doorShader);
    cleanupCorridors(corridorVAO, corridorVBO, corridorEBO, corridorShader);
    cleanupPyramid(pyramidVAO, pyramidVBO, pyramidEBO);
    cleanupSphere(sphereVAO, sphereVBO, sphereEBO);
    cleanupCube(cubeVAO, cubeVBO, cubeEBO);
    cleanupRooms(roomVAO, roomVBO, roomEBO, roomShaders);
    
    glfwTerminate();
    return 0;
}

