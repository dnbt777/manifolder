
#include "ShaderSources.h"

// Camera movement variables
float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = 2.0f;
float cameraSpeed = 0.1f;

// Vertex Shader: Passes 2D positions to fragment shader
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0); // Simple 2D quad
}
)";

// Fragment Shader: Renders merged sphere and cube with lighting
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec2 u_resolution; // Window size (e.g., 800x600)
uniform float u_time;      // Time for camera rotation
uniform vec2 u_mouse;      // Mouse position in screen coordinates
uniform vec3 u_cameraPos;  // Camera position in 3D space

// SDF for a sphere: distance to a sphere of radius 0.5
float sdfSphere(vec3 p) {
    return length(p) - 0.5;
}

// SDF for a cube: distance to a cube with side length 1.0
float sdfCube(vec3 p) {
    vec3 d = abs(p) - vec3(0.5); // Half-size of cube is 0.5
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

// Smooth minimum: blends two distances smoothly
float smoothMin(float a, float b, float k) {
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}

uniform vec2 u_dragOffset;  // Mouse drag offset
uniform float u_isDragging; // Whether mouse is being dragged

// Combined SDF: merges sphere and cube
float sdfScene(vec3 p) {
    // Move sphere based on drag offset (scale factor adjusts sensitivity)
    vec3 sphereOffset = vec3(u_dragOffset.x / 200.0, -u_dragOffset.y / 200.0, 0.0);
    float sphere = sdfSphere(p - sphereOffset);
    float cube = sdfCube(p);
    return smoothMin(sphere, cube, 0.3); // k=0.3 controls smoothness
}

// Raymarching: traces a ray to find the scene
float raymarch(vec3 ro, vec3 rd) {
    float t = 0.0; // Distance along ray
    for (int i = 0; i < 64; i++) {
        vec3 p = ro + rd * t; // Current position
        float d = sdfScene(p); // Distance to scene
        if (d < 0.001) return t; // Hit (close enough)
        t += d; // Step forward
        if (t > 20.0) return -1.0; // Too far, miss
    }
    return -1.0; // Missed after max steps
}

// Normal: calculates surface direction for lighting
vec3 getNormal(vec3 p) {
    float eps = 0.001; // Small offset
    vec3 n = vec3(
        sdfScene(p + vec3(eps, 0.0, 0.0)) - sdfScene(p - vec3(eps, 0.0, 0.0)),
        sdfScene(p + vec3(0.0, eps, 0.0)) - sdfScene(p - vec3(0.0, eps, 0.0)),
        sdfScene(p + vec3(0.0, 0.0, eps)) - sdfScene(p - vec3(0.0, 0.0, eps))
    );
    return normalize(n); // Unit vector
}

void main() {
    // Convert pixel coords to [-1, 1], adjust for aspect ratio
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    // Mouse-controlled camera rotation (flip the horizontal direction to fix inversion)
    float horizontalAngle = -(u_mouse.x / u_resolution.x) * 2.0 * 3.14159; // Map mouse X to full rotation (negative for correct direction)
    float verticalAngle = (u_mouse.y / u_resolution.y) * 3.14159 * 0.5;   // Map mouse Y to limited tilt
    
    // Use camera position from uniform
    vec3 ro = u_cameraPos;
    
    // Calculate view direction based on mouse rotation
    vec3 lookDir = normalize(vec3(
        sin(horizontalAngle) * cos(verticalAngle),
        sin(verticalAngle),
        cos(horizontalAngle) * cos(verticalAngle)
    ));
    // Calculate camera basis vectors
    vec3 forward = lookDir;
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = normalize(cross(right, forward));
    
    // Ray direction with perspective
    vec3 rd = normalize(forward + uv.x * right + uv.y * up);
    
    // Check if the center ray hits an object (for cursor interaction)
    bool centerRay = abs(uv.x) < 0.01 && abs(uv.y) < 0.01;

    // Raymarch the scene
    float t = raymarch(ro, rd);
    if (t > 0.0) { // Hit the merged shape
        vec3 p = ro + rd * t; // Hit point
        vec3 normal = getNormal(p); // Surface normal

        // Lighting: fixed light at (2, 2, 2)
        vec3 lightPos = vec3(2.0, 2.0, 2.0);
        vec3 lightDir = normalize(lightPos - p);
        float diffuse = max(dot(normal, lightDir), 0.0); // Diffuse lighting
        vec3 ambient = vec3(0.1); // Ambient light
        // Change color to green if center ray is hitting the object
        vec3 baseColor = centerRay ? vec3(0.2, 0.8, 0.2) : vec3(0.8, 0.2, 0.2);
        vec3 color = baseColor * diffuse + ambient;

        FragColor = vec4(color, 1.0); // Output color
    } else {
                FragColor = vec4(0.0, 0.0, 0.2, 1.0); // Dark blue background
        
        // Draw crosshair if no object was hit
        if (length(uv) < 0.02 && (abs(uv.x) < 0.005 || abs(uv.y) < 0.005)) {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White crosshair
        }
    }
}
)";
