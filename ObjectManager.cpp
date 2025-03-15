
#include "ObjectManager.h"
#include "CoordSystem.h"
#include <algorithm>

ObjectManager::ObjectManager() : m_rng(std::random_device{}()) {
    // Initialize random distribution for [-5, 5] range
    m_dist = std::uniform_real_distribution<float>(-5.0f, 5.0f);
}

int ObjectManager::addObject(int type, const glm::vec4& position) {
    m_objectTypes.push_back(type);
    m_positions.push_back(position);
    return static_cast<int>(m_objectTypes.size() - 1);
}

int ObjectManager::addRandomObject(int type) {
    // Generate random 3D position and use getrealcoord to convert to 4D
    glm::vec3 randomPos3D(m_dist(m_rng), m_dist(m_rng), m_dist(m_rng));
    return addObject(type, getrealcoord(randomPos3D));
}

void ObjectManager::generateRandomObjects(int sphereCount, int cubeCount) {
    // Generate spheres
    for (int i = 0; i < sphereCount; i++) {
        addRandomObject(0); // 0 = sphere
    }
    
    // Generate cubes
    for (int i = 0; i < cubeCount; i++) {
        addRandomObject(1); // 1 = cube
    }
}

int ObjectManager::getObjectCount() const {
    return static_cast<int>(m_objectTypes.size());
}

int ObjectManager::getObjectType(int index) const {
    if (index >= 0 && index < m_objectTypes.size()) {
        return m_objectTypes[index];
    }
    return -1; // Invalid index
}

glm::vec4 ObjectManager::getObjectPosition(int index) const {
    if (index >= 0 && index < m_positions.size()) {
        return m_positions[index];
    }
    return glm::vec4(0.0f, 0.0f, 0.0f, 7.0f); // Invalid index, use default w=7
}

glm::vec3 ObjectManager::getObject3DPosition(int index) const {
    if (index >= 0 && index < m_positions.size()) {
        // Map 4D position to 3D
        return getmapcoord(m_positions[index]);
    }
    return glm::vec3(0.0f); // Invalid index
}

const int* ObjectManager::getTypesArray() const {
    return m_objectTypes.data();
}

// This function requires special handling since we store 4D positions but need 3D for shaders
const float* ObjectManager::getPositionsArray() const {
    static std::vector<glm::vec3> mappedPositions;
    mappedPositions.clear();
    mappedPositions.reserve(m_positions.size());
    
    // Map all 4D positions to 3D for the shader
    for (const auto& pos : m_positions) {
        mappedPositions.push_back(getmapcoord(pos));
    }
    
    return reinterpret_cast<const float*>(mappedPositions.data());
}

void ObjectManager::selectObject(int index) {
    if (index >= 0 && index < m_objectTypes.size()) {
        // Only add if not already selected
        if (!isObjectSelected(index)) {
            m_selectedObjects.push_back(index);
        }
    }
}

void ObjectManager::deselectObject(int index) {
    auto it = std::find(m_selectedObjects.begin(), m_selectedObjects.end(), index);
    if (it != m_selectedObjects.end()) {
        m_selectedObjects.erase(it);
    }
}

void ObjectManager::clearSelections() {
    m_selectedObjects.clear();
}

bool ObjectManager::isObjectSelected(int index) const {
    return std::find(m_selectedObjects.begin(), m_selectedObjects.end(), index) != m_selectedObjects.end();
}

const std::vector<int>& ObjectManager::getSelectedObjects() const {
    return m_selectedObjects;
}

int ObjectManager::getSelectedCount() const {
    return static_cast<int>(m_selectedObjects.size());
}

void ObjectManager::setObjectPosition(int index, const glm::vec4& position) {
    if (index >= 0 && index < m_positions.size()) {
        m_positions[index] = position;
    }
}

void ObjectManager::setObject3DPosition(int index, const glm::vec3& position) {
    if (index >= 0 && index < m_positions.size()) {
        // Convert the 3D position to 4D using getrealcoord
        m_positions[index] = getrealcoord(position);
    }
}
