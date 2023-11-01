#include "gl_shader_utility.h"

#include "src/backend/opengl/gl_utility.h"
#include "src/util/log.h"

using namespace prt3;

GLuint glshaderutility::create_shader(const char* vertexPath,
                                      const char* fragmentPath) {
    GLuint id;
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    // std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    // ensure ifstream objects can throw exceptions:
    // vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    // fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    // gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    // try
    // {
    // open files
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    std::stringstream vShaderStream, fShaderStream;
    // read file's buffer contents into streams
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    // close file handlers
    vShaderFile.close();
    fShaderFile.close();
    // convert stream into string
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

    // TODO: error handling
    // } catch (std::ifstream::failure& e) {
    //     std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    // }
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    GL_CHECK(vertex = glCreateShader(GL_VERTEX_SHADER));
    GL_CHECK(glShaderSource(vertex, 1, &vShaderCode, NULL));
    GL_CHECK(glCompileShader(vertex));
    check_compile_errors(vertex, "VERTEX", vertexPath);
    // fragment Shader
    GL_CHECK(fragment = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CHECK(glShaderSource(fragment, 1, &fShaderCode, NULL));
    GL_CHECK(glCompileShader(fragment));
    check_compile_errors(fragment, "FRAGMENT", fragmentPath);
    // shader Program
    GL_CHECK(id = glCreateProgram());
    GL_CHECK(glAttachShader(id, vertex));
    GL_CHECK(glAttachShader(id, fragment));
    GL_CHECK(glLinkProgram(id));
    check_compile_errors(id, "PROGRAM", (std::string(vertexPath) + ", ") + fragmentPath);
    // delete the shaders as they're linked into our program now and no longer necessery
    GL_CHECK(glDeleteShader(vertex));
    GL_CHECK(glDeleteShader(fragment));
    return id;
}

void glshaderutility::check_compile_errors(
    GLuint shader,
    std::string const & type,
    std::string const & path
) {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM")
    {
        GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
        if(!success)
        {
            GL_CHECK(glGetShaderInfoLog(shader, 1024, NULL, infoLog));
            PRT3ERROR("%s:\nERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- ", path.c_str(), type.c_str(), infoLog);
        }
    }
    else
    {
        GL_CHECK(glGetProgramiv(shader, GL_LINK_STATUS, &success));
        if(!success)
        {
            GL_CHECK(glGetProgramInfoLog(shader, 1024, NULL, infoLog));
            PRT3ERROR("%s:\nERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- ", path.c_str(), type.c_str(), infoLog);
        }
    }
}
