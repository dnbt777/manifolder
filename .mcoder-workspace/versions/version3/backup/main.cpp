#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "SDFRenderer.h"

// Global renderer pointer for callbacks
SDFRenderer* g_renderer = nullptr;

// Key state tracking (for continuous movement)
struct {
    bool forward = false;   // W
    bool backward = false;  // S
    bool left = false;      // A
    bool right = false;     // D
    bool up = false;        // Space
    bool down = false;      // Shift
} keyState;

// Mouse callback function
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (g_renderer) {
        g_renderer->setMousePosition(static_cast<float>(xpos), static_cast<float>(ypos));
    }
}

// Keyboard callback function for WASD and Space/Shift movement
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!g_renderer) return;

    // Update key state based on press/release
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
        bool isPressed = (action == GLFW_PRESS);
        
        // Set the appropriate key state flag
        switch (key) {
            case GLFW_KEY_W:
                keyState.forward = isPressed;
                break;
            case GLFW_KEY_S:
                keyState.backward = isPressed;
                break;
            case GLFW_KEY_A:
                keyState.left = isPressed;
                break;
            case GLFW_KEY_D:
                keyState.right = isPressed;
                break;
            case GLFW_KEY_SPACE:
                keyState.up = isPressed;
                break;
            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_RIGHT_SHIFT:
                keyState.down = isPressed;
                break;
        }
    }
}

// Mouse button callback function
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (g_renderer) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                g_renderer->setMouseButtonState(true);
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                g_renderer->setMouseDragStart(static_cast<float>(xpos), static_cast<float>(ypos));
            } else if (action == GLFW_RELEASE) {
                g_renderer->setMouseButtonState(false);
                g_renderer->storeDragOffset();
            }
        }
    }
}

// Window resize callback function
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (g_renderer) {
        g_renderer->setWindowSize(width, height);
    }
}

int main() {
    // --- Initialize GLFW ---
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version to 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Simple SDF Renderer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Set background color
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    
    // Initialize the SDF renderer
    SDFRenderer renderer;
    if (!renderer.initialize()) {
        std::cerr << "Failed to initialize SDF renderer" << std::endl;
        return -1;
    }
    
    // Store renderer pointer for callbacks and set initial window size and mouse position
    g_renderer = &renderer;
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    renderer.setWindowSize(window_width, window_height);
    renderer.setMousePosition(window_width / 2.0f, window_height / 2.0f);
    
    // Set up callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    
    // Lock cursor to window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // --- Main loop ---
    float time = 0.0f;  // Keep time static or use it for other effects if needed
    
    // Variables for time-based movement
    double lastFrameTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        double currentFrameTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;
        
        // Process input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        
        // Handle continuous movement
        if (keyState.forward || keyState.backward || keyState.left || 
            keyState.right || keyState.up || keyState.down) {
            
            // Get mouse position to determine view direction
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            
            // Calculate horizontal angle - use negative for consistent control with shader
            float horizontalAngle = -(float)(xpos / width) * 2.0f * 3.14159f;
            
            // Camera speed scaled by delta time for consistent movement
            float cameraSpeed = 2.0f * deltaTime;
            
            // Forward/backward movement along view direction
            if (keyState.forward) {
                g_renderer->moveCamera(
                    sin(horizontalAngle) * cameraSpeed,
                    0.0f,
                    cos(horizontalAngle) * cameraSpeed
                );
            }
            if (keyState.backward) {
                g_renderer->moveCamera(
                    -sin(horizontalAngle) * cameraSpeed,
                    0.0f,
                    -cos(horizontalAngle) * cameraSpeed
                );
            }
            
            // Strafe left/right (perpendicular to view direction)
            if (keyState.left) {
                g_renderer->moveCamera(
                    -cos(horizontalAngle) * cameraSpeed,
                    0.0f,
                    sin(horizontalAngle) * cameraSpeed
                );
            }
            if (keyState.right) {
                g_renderer->moveCamera(
                    cos(horizontalAngle) * cameraSpeed,
                    0.0f,
                    -sin(horizontalAngle) * cameraSpeed
                );
            }
            
            // Up/down movement
            if (keyState.up) {
                g_renderer->moveCamera(0.0f, cameraSpeed, 0.0f);
            }
            if (keyState.down) {
                g_renderer->moveCamera(0.0f, -cameraSpeed, 0.0f);
            }
        }
            
        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render the SDF scene (time is now static)
        renderer.render(time);
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    renderer.cleanup();
    glfwTerminate();
    
    return 0;
}
