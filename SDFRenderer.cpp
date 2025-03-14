#include "SDFRenderer.h"
#include "ShaderSources.h"
#include <iostream>

SDFRenderer::SDFRenderer() : VAO(0), VBO(0), EBO(0), width(800), height(600), mouseX(0.0f), mouseY(0.0f),
    mouseLeftPressed(false), dragStartX(0.0f), dragStartY(0.0f), currentDragX(0.0f), currentDragY(0.0f),
    savedDragX(0.0f), savedDragY(0.0f), cameraX(0.0f), cameraY(0.0f), cameraZ(2.0f) {
    // Initialize global camera position
    ::cameraX = 0.0f;
    ::cameraY = 0.0f;
    ::cameraZ = 2.0f;
    ::cameraSpeed = 0.1f;
}

SDFRenderer::~SDFRenderer() {
    cleanup();
}

bool SDFRenderer::initialize() {
    // Set up full-screen quad (two triangles forming a rectangle)
    float vertices[] = {
        -1.0f, -1.0f, // Bottom-left
         1.0f, -1.0f, // Bottom-right
        -1.0f,  1.0f, // Top-left
         1.0f,  1.0f  // Top-right
    };
    unsigned int indices[] = {0, 1, 2, 1, 3, 2}; // Two triangles
    
    // Create OpenGL buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    // Bind and set vertex array
    glBindVertexArray(VAO);
    
    // Bind and set vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Configure vertex attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Compile shaders
    if (!shader.compile(vertexShaderSource, fragmentShaderSource)) {
        std::cerr << "Failed to compile shaders!" << std::endl;
        return false;
    }
    
    return true;
}

void SDFRenderer::render(float time) {
    // Use shader
    shader.use();
    
    // Calculate drag offset if mouse is pressed
    if (mouseLeftPressed) {
        currentDragX = mouseX - dragStartX;
        currentDragY = mouseY - dragStartY;
    }
    
    // Set uniforms
    shader.setVec2("u_resolution", static_cast<float>(width), static_cast<float>(height));
    shader.setFloat("u_time", time);
    shader.setVec2("u_mouse", mouseX, mouseY);
    shader.setVec2("u_dragOffset", savedDragX + currentDragX, savedDragY + currentDragY);
    shader.setFloat("u_isDragging", mouseLeftPressed ? 1.0f : 0.0f);
    
    // Set camera position uniform
    shader.setVec3("u_cameraPos", cameraX, cameraY, cameraZ);
    
    // Draw quad
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void SDFRenderer::cleanup() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    
    // Shader cleanup is handled by the Shader class destructor
    
    // Reset IDs
    VAO = VBO = EBO = 0;
}

void SDFRenderer::setMousePosition(float x, float y) {
    mouseX = x;
    mouseY = y;
}

void SDFRenderer::setWindowSize(int w, int h) {
    width = w;
    height = h;
}

void SDFRenderer::setMouseButtonState(bool pressed) {
    mouseLeftPressed = pressed;
    if (!pressed) {
        currentDragX = currentDragY = 0.0f;
    }
}

void SDFRenderer::setMouseDragStart(float x, float y) {
    dragStartX = x;
    dragStartY = y;
    currentDragX = currentDragY = 0.0f;
}

void SDFRenderer::storeDragOffset() {
    // Save current drag as permanent offset
    savedDragX += currentDragX;
    savedDragY += currentDragY;
    currentDragX = currentDragY = 0.0f;
}

void SDFRenderer::moveCamera(float dx, float dy, float dz) {
    cameraX += dx;
    cameraY += dy;
    cameraZ += dz;
    
    // Update global variables
    ::cameraX = cameraX;
    ::cameraY = cameraY;
    ::cameraZ = cameraZ;
}

void SDFRenderer::setCameraPosition(float x, float y, float z) {
    cameraX = x;
    cameraY = y;
    cameraZ = z;
    
    // Update global variables
    ::cameraX = cameraX;
    ::cameraY = cameraY;
    ::cameraZ = cameraZ;
}
