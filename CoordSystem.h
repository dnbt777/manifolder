
#pragma once
#include <glm/glm.hpp>

// Transforms a 4D point to a 3D point by dropping the w component
inline glm::vec3 getmapcoord(const glm::vec4& point) {
    return glm::vec3(point.x, point.y, point.z);
}

// Transforms a 3D point to a 4D point by adding w=7
inline glm::vec4 getrealcoord(const glm::vec3& point) {
    return glm::vec4(point.x, point.y, point.z, 7.0f);
}
