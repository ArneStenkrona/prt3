#ifndef PRT3_SHADER_H
#define PRT3_SHADER_H

// Courtesy of https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace prt3 {

namespace glshaderutility {
    GLuint create_shader(const char* vertexPath, const char* fragmentPath);

    inline void set_bool(GLuint id, const std::string &name, bool value) {
        glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
    }

    inline void set_uint(GLuint id, const std::string &name, uint32_t value) {
        glUniform1ui(glGetUniformLocation(id, name.c_str()), value);
    }

    inline void set_int(GLuint id, const std::string &name, int32_t value) {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }

    inline void set_float(GLuint id, const std::string &name, float value) {
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }

    inline void set_vec2(GLuint id, const std::string &name, const glm::vec2 &value) {
        glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }
    inline void set_vec2(GLuint id, const std::string &name, float x, float y) {
        glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
    }

    inline void set_vec3(GLuint id, const std::string &name, const glm::vec3 &value) {
        glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }
    inline void set_vec3(GLuint id, const std::string &name, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
    }

    inline void set_vec4(GLuint id, const std::string &name, const glm::vec4 &value) {
        glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }
    inline void set_vec4(GLuint id, const std::string &name, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
    }

    inline void set_mat2(GLuint id, const std::string &name, const glm::mat2 &mat) {
        glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    inline void set_mat3(GLuint id, const std::string &name, const glm::mat3 &mat) {
        glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    inline void set_mat4(GLuint id, const std::string &name, const glm::mat4 &mat) {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void check_compile_errors(
        GLuint shader,
        std::string const & type,
        std::string const & path
    );
} // namespace glshaderutility

} // namespace prt3

#endif
