
#pragma once

// Vertex Shader: Passes 2D positions to fragment shader
extern const char* vertexShaderSource;

// Fragment Shader: Renders merged sphere and cube with lighting
extern const char* fragmentShaderSource;

// Variables for camera movement
extern float cameraX;
extern float cameraY;
extern float cameraZ;
extern float cameraSpeed;
