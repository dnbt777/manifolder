
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <random>

// Handles the storage and management of SDF objects using struct of arrays pattern
class ObjectManager {
public:
    // Constructor
    ObjectManager();
    
    // Add a new object with specified type and position
    int addObject(int type, const glm::vec4& position);
    
    // Add a randomly positioned object of the given type
    int addRandomObject(int type);
    
    // Generate random objects (count of each type)
    void generateRandomObjects(int sphereCount, int cubeCount);
    
    // Get number of objects
    int getObjectCount() const;
    
    // Get object type at index
    int getObjectType(int index) const;
    
    // Get object position at index
    glm::vec4 getObjectPosition(int index) const;
    
    // Get object 3D position at index (after mapping from 4D)
    glm::vec3 getObject3DPosition(int index) const;
    
    // Get types array pointer for shader uniform
    const int* getTypesArray() const;
    
    // Get positions array pointer for shader uniform (3D mapped positions)
    const float* getPositionsArray() const;
    
    // Select an object by index
    void selectObject(int index);
    
    // Deselect an object by index
    void deselectObject(int index);
    
    // Clear all selections
    void clearSelections();
    
    // Check if an object is selected
    bool isObjectSelected(int index) const;
    
    // Get the list of selected object indices
    const std::vector<int>& getSelectedObjects() const;
    
    // Get the number of selected objects
    int getSelectedCount() const;
    
    // Set position of an object (4D)
    void setObjectPosition(int index, const glm::vec4& position);
    
    // Set position of an object using 3D position (will be unmapped to 4D)
    void setObject3DPosition(int index, const glm::vec3& position);
    
private:
    // Struct of Arrays pattern for object data
    std::vector<int> m_objectTypes;     // 0 = sphere, 1 = cube
    std::vector<glm::vec4> m_positions; // Object positions (4D)
    std::vector<int> m_selectedObjects; // List of indices of selected objects
    
    // Random number generator
    std::mt19937 m_rng;
    std::uniform_real_distribution<float> m_dist;
};
