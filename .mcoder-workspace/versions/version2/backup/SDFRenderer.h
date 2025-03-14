
#pragma once
#include <GL/glew.h>
#include "Shader.h"

class SDFRenderer {
public:
    // Constructor
    SDFRenderer();
    
    // Initialize the renderer
    bool initialize();
    
    // Render the scene with current time
    void render(float time);
    
    // Clean up resources
    void cleanup();
    
    // Destructor
    ~SDFRenderer();
    
    // Set mouse position (for interactive effects)
    void setMousePosition(float x, float y);
    
    // Update window size (for proper aspect ratio)
    void setWindowSize(int w, int h);

    // Set mouse button state (for tracking drag operations)
    void setMouseButtonState(bool pressed);
    
    // Set drag start position
    void setMouseDragStart(float x, float y);
    
    // Store current drag offset as permanent offset
    void storeDragOffset();
    
    // Camera movement methods
    void moveCamera(float dx, float dy, float dz);
    void setCameraPosition(float x, float y, float z);
    
private:
    // OpenGL objects
    GLuint VAO, VBO, EBO;
    
    // Camera position in 3D space
    float cameraX, cameraY, cameraZ;
    
    // Shader program
    Shader shader;
    
    // Window dimensions
    int width, height;
    
    // Mouse position
    float mouseX, mouseY;
    
    // Mouse drag handling
    bool mouseLeftPressed;
    float dragStartX, dragStartY;
    float currentDragX, currentDragY;
    float savedDragX, savedDragY;
    
    // Shape-specific dragging
    bool draggingShape;
    int selectedShape; // 0=none, 1=sphere, 2=cube
};
