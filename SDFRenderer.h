
#pragma once
#include <GL/glew.h>
#include "Shader.h"
#include "ObjectManager.h"

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
    
    // Input state tracking
    void setShiftKeyState(bool pressed);
    
private:
    // SDF helper functions that match the shader implementations
    float sdfSphere(const glm::vec3& p);
    float sdfCube(const glm::vec3& p);
    float sdfObject(const glm::vec3& p, int objIndex);
    float sdfScene(const glm::vec3& p);
    int getHitObjectIndex(const glm::vec3& p);
    float raymarch(const glm::vec3& ro, const glm::vec3& rd);
    
    // Helper function to determine which object is under the cursor
    void updateObjectUnderCursor();

    // OpenGL objects
    GLuint VAO, VBO, EBO;
    
    // Camera position in 4D space (x,y,z components stored separately for convenience)
    float cameraX, cameraY, cameraZ;
    float cameraW; // W-component of camera position
    
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
    
    // Object selection
    int objectUnderCursor; // Index of object under cursor (-1 if none)
    bool shiftKeyPressed; // Whether Shift key is currently pressed (now unused for multi-selection)
    
    // Object manager to handle objects in the scene
    ObjectManager objectManager;
    
    // Currently dragged object index (-1 if none)
    int draggedObjectIndex;
    
    // Store initial position of dragged object and its distance from camera
    glm::vec4 draggedObjectInitialPos;
    float draggedObjectDistance;
};
