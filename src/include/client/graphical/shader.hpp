#pragma once

#if !(defined(__gl_h_) || defined(__GL_H__) || defined(_GL_H) || defined(__X_GL_H))
#include <GL/glew.h>
#endif
#include <glm/glm.hpp>

enum ShaderType { Vertex, Fragment, Geometry, Compute };

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace familyline::graphics
{
/**
 * Represents a shader file
 *
 * We unite those shader files in a shader program.
 * @see ShaderProgram
 */
class Shader
{
private:
    const char* _file;
    ShaderType _type;
    int _handle;

    std::string readAndProcessFile(const char* file);    
    std::string readFile(const char* file);

public:
    Shader(const char* file, ShaderType type);
    void compile();

    int getHandle() const { return this->_handle; }
    ShaderType getType() const { return this->_type; }
};

class ShaderProgram
{
private:
    std::string_view _name;
    std::vector<std::pair<ShaderType, Shader>> _files;
    int _handle;
    std::unordered_map<std::string, GLint> _uniform_cache;

    GLint getUniformLocation(std::string_view name);

public:
    ShaderProgram(std::string_view name, std::initializer_list<Shader> shaders);
    void link();

    int getHandle() const { return this->_handle; }
    std::string_view getName() const { return _name; }

    void setUniform(std::string_view name, glm::vec3 val);
    void setUniform(std::string_view name, glm::vec4 val);
    void setUniform(std::string_view name, glm::mat4 val);
    void setUniform(std::string_view name, int val);
    void setUniform(std::string_view name, float val);

    // TODO: Move this to a shader manager?
    // void use();
};
}  // namespace familyline::graphics
