#include <cmath>
#include "SDFRenderer.h"
#include "ShaderSources.h"
#include "CoordSystem.h"
#include <iostream>
#include <glm/glm.hpp>

SDFRenderer::SDFRenderer() : VAO(0), VBO(0), EBO(0), width(800), height(600), mouseX(0.0f), mouseY(0.0f),
    mouseLeftPressed(false), dragStartX(0.0f), dragStartY(0.0f), currentDragX(0.0f), currentDragY(0.0f),
    savedDragX(0.0f), savedDragY(0.0f), cameraX(0.0f), cameraY(0.0f), cameraZ(2.0f), cameraW(7.0f),
    draggingShape(false), selectedShape(0), draggedObjectIndex(-1), objectUnderCursor(-1), shiftKeyPressed(false) {
    // Initialize global camera position
    ::cameraX = 0.0f;
    ::cameraY = 0.0f;
    ::cameraZ = 2.0f;
    ::cameraSpeed = 0.1f;
    
    // Generate some random objects (5 spheres, 5 cubes)
    objectManager.generateRandomObjects(5, 5);
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
    
    // Reset the object under cursor and update it based on hover
    objectUnderCursor = -1;
    
    // First update the object under cursor
    updateObjectUnderCursor();
    
    // Auto-select the object under cursor (hover selection)
    objectManager.clearSelections();
    if (objectUnderCursor >= 0) {
        objectManager.selectObject(objectUnderCursor);
        
        // If mouse is pressed, this becomes the dragged object
        if (mouseLeftPressed && draggedObjectIndex == -1) {
            draggedObjectIndex = objectUnderCursor;
            
            // Store initial position and distance from camera
            draggedObjectInitialPos = objectManager.getObjectPosition(draggedObjectIndex);
            // Calculate distance using the 3D mapped positions
            glm::vec3 mappedObjectPos = getmapcoord(draggedObjectInitialPos);
            glm::vec3 mappedCameraPos = getmapcoord(glm::vec4(cameraX, cameraY, cameraZ, cameraW));
            draggedObjectDistance = glm::length(mappedObjectPos - mappedCameraPos);
        }
    }
    
    // Handle dragging - move object in a sphere around the camera
    if (mouseLeftPressed && draggedObjectIndex >= 0) {
        // Calculate view direction based on mouse position
        float horizontalAngle = -(mouseX / static_cast<float>(width)) * 2.0f * 3.14159f;
        float verticalAngle = ((1.0f - mouseY / static_cast<float>(height)) - 0.5f) * 3.14159f * 0.5f;
        
        // Calculate the direction vector from the camera in 3D space
        glm::vec3 direction(
            sin(horizontalAngle) * cos(verticalAngle),
            sin(verticalAngle),
            cos(horizontalAngle) * cos(verticalAngle)
        );
        
        // Scale the direction vector by the distance to keep object at constant distance
        glm::vec3 mappedCameraPos = getmapcoord(glm::vec4(cameraX, cameraY, cameraZ, cameraW));
        glm::vec3 newPosition3D = mappedCameraPos + direction * draggedObjectDistance;
        
        // Update the object's position - convert 3D position to 4D
        objectManager.setObject3DPosition(draggedObjectIndex, newPosition3D);
    }
    
    // Set basic uniforms
    shader.setVec2("u_resolution", static_cast<float>(width), static_cast<float>(height));
    shader.setFloat("u_time", time);
    shader.setVec2("u_mouse", mouseX, mouseY);
    shader.setFloat("u_isDragging", mouseLeftPressed ? 1.0f : 0.0f);
    // Send the mapped (3D) camera position to the shader
    glm::vec3 mappedCameraPos = getmapcoord(glm::vec4(cameraX, cameraY, cameraZ, cameraW));
    shader.setVec3("u_cameraPos", mappedCameraPos.x, mappedCameraPos.y, mappedCameraPos.z);
    
    // Set object data uniforms
    int objectCount = objectManager.getObjectCount();
    shader.setInt("u_objectCount", objectCount);
    
    // Update object types and positions in shader
    for (int i = 0; i < objectCount; i++) {
        std::string typeIndex = "u_objectTypes[" + std::to_string(i) + "]";
        shader.setInt(typeIndex, objectManager.getObjectType(i));
        
        std::string posIndex = "u_objectPositions[" + std::to_string(i) + "]";
        glm::vec3 pos = objectManager.getObject3DPosition(i); // Get mapped 3D position
        shader.setVec3(posIndex, pos.x, pos.y, pos.z);
        
        std::string selIndex = "u_objectSelected[" + std::to_string(i) + "]";
        shader.setInt(selIndex, objectManager.isObjectSelected(i) ? 1 : 0);
    }
    
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
    
    // Update the object under cursor whenever the mouse moves
    updateObjectUnderCursor();
}

// SDF functions that match those in the fragment shader
float SDFRenderer::sdfSphere(const glm::vec3& p) {
    return glm::length(p) - 0.5f;
}

float SDFRenderer::sdfCube(const glm::vec3& p) {
    glm::vec3 d = glm::abs(p) - glm::vec3(0.5f); // Half-size of cube is 0.5
    return glm::length(glm::max(d, glm::vec3(0.0f))) + 
           std::min(std::max(d.x, std::max(d.y, d.z)), 0.0f);
}

// Object-specific SDF with world position
float SDFRenderer::sdfObject(const glm::vec3& p, int objIndex) {
    // Get object data
    int type = objectManager.getObjectType(objIndex);
    glm::vec3 position = objectManager.getObject3DPosition(objIndex); // Use mapped 3D position
    
    // Calculate distance based on object type
    if (type == 0) {
        // Sphere
        return sdfSphere(p - position);
    } else if (type == 1) {
        // Cube
        return sdfCube(p - position);
    }
    
    return 1000.0f; // Default large distance for unknown types
}

// Combined SDF: finds minimum distance to any object in the scene
float SDFRenderer::sdfScene(const glm::vec3& p) {
    float minDist = 1000.0f;
    
    for (int i = 0; i < objectManager.getObjectCount(); i++) {
        float dist = sdfObject(p, i);
        if (dist < minDist) {
            minDist = dist;
        }
    }
    
    return minDist;
}

// Find closest object hit by ray
int SDFRenderer::getHitObjectIndex(const glm::vec3& p) {
    float minDist = 1000.0f;
    int closestIndex = -1;
    
    for (int i = 0; i < objectManager.getObjectCount(); i++) {
        float dist = sdfObject(p, i);
        if (dist < minDist && dist < 0.01f) {
            minDist = dist;
            closestIndex = i;
        }
    }
    
    return closestIndex;
}

// Raymarch algorithm to find intersection with scene
float SDFRenderer::raymarch(const glm::vec3& ro, const glm::vec3& rd) {
    float t = 0.0f; // Distance along ray
    for (int i = 0; i < 64; i++) {
        glm::vec3 p = ro + rd * t; // Current position
        float d = sdfScene(p); // Distance to scene
        if (d < 0.001f) return t; // Hit (close enough)
        t += d; // Step forward
        if (t > 20.0f) return -1.0f; // Too far, miss
    }
    return -1.0f; // Missed after max steps
}

// Helper function to determine which object is under the cursor
void SDFRenderer::updateObjectUnderCursor() {
    // Calculate ray direction based on mouse position and camera orientation
    float horizontalAngle = -(mouseX / static_cast<float>(width)) * 2.0f * 3.14159f;
    float verticalAngle = ((1.0f - mouseY / static_cast<float>(height)) - 0.5f) * 3.14159f * 0.5f;
    
    // Calculate the ray direction from camera
    glm::vec3 rayDir(
        sin(horizontalAngle) * cos(verticalAngle),
        sin(verticalAngle),
        cos(horizontalAngle) * cos(verticalAngle)
    );
    
    // Camera position (mapped from 4D to 3D)
    glm::vec3 rayOrigin = getmapcoord(glm::vec4(cameraX, cameraY, cameraZ, cameraW));
    
    // Perform raymarching to find intersection with scene
    float t = raymarch(rayOrigin, rayDir);
    
    // Reset object under cursor
    objectUnderCursor = -1;
    
    // If we hit something, determine which object it was
    if (t > 0.0f) {
        glm::vec3 hitPoint = rayOrigin + rayDir * t;
        objectUnderCursor = getHitObjectIndex(hitPoint);
    }
}

void SDFRenderer::setWindowSize(int w, int h) {
    width = w;
    height = h;
}

void SDFRenderer::setMouseButtonState(bool pressed) {
    mouseLeftPressed = pressed;
    
    if (pressed) {
        // When the mouse button is pressed, start dragging the object currently under the cursor
        if (objectUnderCursor >= 0 && objectUnderCursor < objectManager.getObjectCount()) {
            draggedObjectIndex = objectUnderCursor;
            
            // Store initial position and distance from camera
            draggedObjectInitialPos = objectManager.getObjectPosition(draggedObjectIndex);
            // Calculate distance using the 3D mapped positions
            glm::vec3 mappedObjectPos = getmapcoord(draggedObjectInitialPos);
            glm::vec3 mappedCameraPos = getmapcoord(glm::vec4(cameraX, cameraY, cameraZ, cameraW));
            draggedObjectDistance = glm::length(mappedObjectPos - mappedCameraPos);
        }
    } else {
        // When released, stop dragging but don't clear selection
        draggedObjectIndex = -1;
        currentDragX = currentDragY = 0.0f;
    }
}

void SDFRenderer::setShiftKeyState(bool pressed) {
    shiftKeyPressed = pressed;
}

void SDFRenderer::setMouseDragStart(float x, float y) {
    dragStartX = x;
    dragStartY = y;
    currentDragX = currentDragY = 0.0f;
}

void SDFRenderer::storeDragOffset() {
    // With the new implementation, we don't need to store drag offset separately
    // as the sphere's position is directly updated during dragging
    currentDragX = currentDragY = 0.0f;
}

void SDFRenderer::moveCamera(float dx, float dy, float dz) {
    // Calculate new position in 4D by adding the 3D movement vector to the mapped position
    glm::vec3 mappedPos = getmapcoord(glm::vec4(cameraX, cameraY, cameraZ, cameraW));
    glm::vec3 newMappedPos = mappedPos + glm::vec3(dx, dy, dz);
    
    // Convert back to 4D
    glm::vec4 newPos4D = getrealcoord(newMappedPos);
    
    // Update camera position
    cameraX = newPos4D.x;
    cameraY = newPos4D.y;
    cameraZ = newPos4D.z;
    cameraW = newPos4D.w;
    
    // Update global variables (which are still 3D)
    ::cameraX = cameraX;
    ::cameraY = cameraY;
    ::cameraZ = cameraZ;
}

void SDFRenderer::setCameraPosition(float x, float y, float z) {
    // Create a 4D position with w=7
    glm::vec4 newPos = getrealcoord(glm::vec3(x, y, z));
    
    cameraX = newPos.x;
    cameraY = newPos.y;
    cameraZ = newPos.z;
    cameraW = newPos.w;
    
    // Update global variables
    ::cameraX = cameraX;
    ::cameraY = cameraY;
    ::cameraZ = cameraZ;
}
