#ifndef PRT3_SHADER_H
#define PRT3_SHADER_H

// Courtesy of https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h

#include "src/backend/opengl/gl_utility.h"

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
        GL_CHECK(glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value));
    }

    inline void set_uint(GLuint id, const std::string &name, uint32_t value) {
        GL_CHECK(glUniform1ui(glGetUniformLocation(id, name.c_str()), value));
    }

    inline void set_int(GLuint id, const std::string &name, int32_t value) {
        GL_CHECK(glUniform1i(glGetUniformLocation(id, name.c_str()), value));
    }

    inline void set_float(GLuint id, const std::string &name, float value) {
        GL_CHECK(glUniform1f(glGetUniformLocation(id, name.c_str()), value));
    }

    inline void set_vec2(GLuint id, const std::string &name, const glm::vec2 &value) {
        GL_CHECK(glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]));
    }
    inline void set_vec2(GLuint id, const std::string &name, float x, float y) {
        GL_CHECK(glUniform2f(glGetUniformLocation(id, name.c_str()), x, y));
    }

    inline void set_vec3(GLuint id, const std::string &name, const glm::vec3 &value) {
        GL_CHECK(glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]));
    }
    inline void set_vec3(GLuint id, const std::string &name, float x, float y, float z) {
        GL_CHECK(glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z));
    }

    inline void set_vec4(GLuint id, const std::string &name, const glm::vec4 &value) {
        GL_CHECK(glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]));
    }
    inline void set_vec4(GLuint id, const std::string &name, float x, float y, float z, float w) {
        GL_CHECK(glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w));
    }

    inline void set_mat2(GLuint id, const std::string &name, const glm::mat2 &mat) {
        GL_CHECK(glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]));
    }

    inline void set_mat3(GLuint id, const std::string &name, const glm::mat3 &mat) {
        GL_CHECK(glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]));
    }

    inline void set_mat4(GLuint id, const std::string &name, const glm::mat4 &mat) {
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]));
    }

    void check_compile_errors(
        GLuint shader,
        std::string const & type,
        std::string const & path
    );
} // namespace glshaderutility

} // namespace prt3

#endif
