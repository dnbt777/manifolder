
#include "ShaderSources.h"

// Camera movement variables
float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = 2.0f;
float cameraSpeed = 0.1f;

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0); // the screen quad 
}
)";

// Fragment Shader: Renders merged sphere and cube with lighting
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec2 u_resolution; 
uniform float u_time;      
uniform vec2 u_mouse;      
uniform vec3 u_cameraPos;  
uniform float u_isDragging;

// Define maximum number of objects
#define MAX_OBJECTS 50

// Object data arrays
uniform int u_objectCount;
uniform int u_objectTypes[MAX_OBJECTS];        // 0 = sphere, 1 = cube
uniform vec3 u_objectPositions[MAX_OBJECTS];   // Object positions
uniform int u_objectSelected[MAX_OBJECTS];     // 1 if selected, 0 if not

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

// Calculate the blend weight for each object based on proximity
vec2 smoothMinWeight(float a, float b, float k) {
    float h = max(k - abs(a - b), 0.0) / k;
    float m = h * h * 0.5; // Blend factor
    
    // Calculate individual weights
    float weight_a = (a < b) ? (1.0 - m) : m;
    float weight_b = (a < b) ? m : (1.0 - m);
    
    return vec2(weight_a, weight_b);
}

// Object-specific SDFs with world position
float sdfObject(vec3 p, int objIndex) {
    // Get object data
    int type = u_objectTypes[objIndex];
    vec3 position = u_objectPositions[objIndex];
    
    // Calculate distance based on object type
    if (type == 0) {
        // Sphere
        return sdfSphere(p - position);
    } else if (type == 1) {
        // Cube
        return sdfCube(p - position);
    }
    
    return 1000.0; // Default large distance for unknown types
}

// Structure to hold both distance and color information
struct SDFResult {
    float distance;
    vec3 color;
};

// Get color for object based on type and selection state
vec3 getObjectColor(int objIndex) {
    int objType = u_objectTypes[objIndex];
    bool isSelected = u_objectSelected[objIndex] == 1;
    
    if (isSelected) {
        return vec3(0.2, 0.4, 0.9); // Selected objects are blue
    } else if (objType == 0) {
        return vec3(0.8, 0.2, 0.2); // Sphere - red
    } else if (objType == 1) {
        return vec3(0.8, 0.4, 0.0); // Cube - orange
    }
    return vec3(1.0); // Default white
}

// Combined SDF: finds minimum distance to any object in the scene and calculates blended color
SDFResult sdfScene(vec3 p) {
    float minDist = 1000.0;
    vec3 blendedColor = vec3(0.0);
    float totalWeight = 0.0;
    
    // First pass: calculate distances for each object
    float objectDists[MAX_OBJECTS];
    for (int i = 0; i < u_objectCount && i < MAX_OBJECTS; i++) {
        objectDists[i] = sdfObject(p, i);
    }
    
    // Second pass: blend distances and calculate color weights
    minDist = 1000.0;
    for (int i = 0; i < u_objectCount && i < MAX_OBJECTS; i++) {
        float dist = objectDists[i];
        
        // For each object pair, calculate smooth blending
        for (int j = 0; j < i; j++) {
            float smoothed = smoothMin(dist, objectDists[j], 0.3);
            
            // If this blend creates a new minimum, update distances
            if (smoothed < minDist) {
                // Calculate blend weights
                vec2 weights = smoothMinWeight(dist, objectDists[j], 0.3);
                
                // Get colors for both objects including selection state
                vec3 color_i = getObjectColor(i);
                vec3 color_j = getObjectColor(j);
                
                // Blend colors based on weights
                blendedColor = color_i * weights.x + color_j * weights.y;
                minDist = smoothed;
            }
        }
        
        // Also check individual object distances
        if (dist < minDist) {
            minDist = dist;
            
            // Set color based on object (including selection state)
            blendedColor = getObjectColor(i);
        }
    }
    
    // Return both distance and blended color
    return SDFResult(minDist, blendedColor);
}

// Find the closest object hit (returns index, or -1 if none)
int getHitObjectIndex(vec3 p) {
    float minDist = 1000.0;
    int closestIndex = -1;
    
    for (int i = 0; i < u_objectCount && i < MAX_OBJECTS; i++) {
        float dist = sdfObject(p, i);
        if (dist < minDist && dist < 0.01) {
            minDist = dist;
            closestIndex = i;
        }
    }
    
    return closestIndex;
}

// Raymarching: traces a ray to find the scene
float raymarch(vec3 ro, vec3 rd) {
    float t = 0.0; // Distance along ray
    for (int i = 0; i < 64; i++) {
        vec3 p = ro + rd * t; // Current position
        float d = sdfScene(p).distance; // Distance to scene
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
        sdfScene(p + vec3(eps, 0.0, 0.0)).distance - sdfScene(p - vec3(eps, 0.0, 0.0)).distance,
        sdfScene(p + vec3(0.0, eps, 0.0)).distance - sdfScene(p - vec3(0.0, eps, 0.0)).distance,
        sdfScene(p + vec3(0.0, 0.0, eps)).distance - sdfScene(p - vec3(0.0, 0.0, eps)).distance
    );
    return normalize(n); // Unit vector
}

void main() {
    // Convert pixel coords to [-1, 1], adjust for aspect ratio
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    // Mouse-controlled camera rotation with natural (non-inverted) controls
    float horizontalAngle = -(u_mouse.x / u_resolution.x) * 2.0 * 3.14159; // Map mouse X to full rotation (negative for natural control)
    float verticalAngle = ((1.0 - u_mouse.y / u_resolution.y) - 0.5) * 3.14159 * 0.5; // Map mouse Y to limited tilt (inverted for natural control)
    
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
    
    // Check if the center ray (cursor) is pointing at an object
    bool centerRay = abs(uv.x) < 0.01 && abs(uv.y) < 0.01;

    // Raymarch the scene
    float t = raymarch(ro, rd);
    if (t > 0.0) { // Hit something
        vec3 p = ro + rd * t; // Hit point
        vec3 normal = getNormal(p); // Surface normal
        
        // Get the blended color from our scene evaluation
        SDFResult sceneResult = sdfScene(p);
        vec3 baseColor = sceneResult.color;
        
        // Find which object was hit (for cursor hover highlighting)
        int hitObjectIndex = getHitObjectIndex(p);
        
        // Only override with blue if it's the center ray (cursor hovering) but not already selected
        if (hitObjectIndex >= 0 && hitObjectIndex < MAX_OBJECTS) {
            bool isSelected = u_objectSelected[hitObjectIndex] == 1;
            if (centerRay && !isSelected) {
                // Object under cursor (hovered) is highlighted in blue
                baseColor = vec3(0.2, 0.4, 0.9);
            }
        }
        
        // Lighting: fixed light at (2, 2, 2)
        vec3 lightPos = vec3(2.0, 2.0, 2.0);
        vec3 lightDir = normalize(lightPos - p);
        float diffuse = max(dot(normal, lightDir), 0.0); // Diffuse lighting
        vec3 ambient = vec3(0.1); // Ambient light
        
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
