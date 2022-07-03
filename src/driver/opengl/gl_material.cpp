#include "gl_material.h"

using namespace prt3;

GLMaterial::GLMaterial(std::string const & vertex_shader_path,
                       std::string const & fragment_shader_path)
 : m_shader{vertex_shader_path.c_str(), fragment_shader_path.c_str()} {

}
