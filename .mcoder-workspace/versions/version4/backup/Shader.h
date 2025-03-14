
#pragma once
#include <GL/glew.h>
#include <string>

class Shader {
public:
    // Constructor reads and builds the shader
    Shader();
    
    // Use the shader program
    void use();
    
    // Compile and link shaders from source strings
    bool compile(const char* vertexSource, const char* fragmentSource);
    
    // Utility uniform functions
    void setFloat(const std::string &name, float value);
    void setVec2(const std::string &name, float x, float y);
    void setVec3(const std::string &name, float x, float y, float z);
    
    // Get the shader program ID
    GLuint getID() const { return ID; }
    
    // Destructor
    ~Shader();

private:
    // Program ID
    GLuint ID;
    
    // Utility function for checking shader compilation/linking errors
    void checkCompileErrors(GLuint shader, std::string type);
};
