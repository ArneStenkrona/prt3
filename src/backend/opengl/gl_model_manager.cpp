#include "gl_model_manager.h"

#include "src/backend/opengl/gl_utility.h"

#include <unordered_set>

using namespace prt3;

GLModelManager::GLModelManager(GLMaterialManager & material_manager)
 : m_material_manager{material_manager} {

}

void GLModelManager::upload_model(
    ModelHandle handle,
    Model const & model,
    std::vector<ResourceID> & mesh_resource_ids
) {
    // upload model buffers
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    GL_CHECK(glGenVertexArrays(1, &vao));
    GL_CHECK(glBindVertexArray(vao));

    GL_CHECK(glGenBuffers(1, &vbo));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));

    if (!model.is_animated()) {
        bind_vertex_buffer(model);
    } else {
        bind_boned_vertex_buffer(model);
    }

    GL_CHECK(glGenBuffers(1, &ebo));

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
    GL_CHECK(glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        model.index_buffer().size() * sizeof(uint32_t),
        model.index_buffer().data(),
        GL_STATIC_DRAW
    ));

    m_model_buffer_handles[handle] = {vao, vbo, ebo};

    // Create gl meshes
    mesh_resource_ids.resize(model.meshes().size());
    size_t mesh_index = 0;
    for (Model::Mesh const & mesh : model.meshes()) {
        ResourceID id = m_next_mesh_id;
        ++m_next_mesh_id;

        GLMesh & gl_mesh = m_meshes[id];
        gl_mesh.init(vao, mesh.start_index, mesh.num_indices);

        mesh_resource_ids[mesh_index] = id;
        ++mesh_index;
    }

    GL_CHECK(glBindVertexArray(0));
}

void GLModelManager::free_model(
    ModelHandle handle,
    std::vector<ResourceID> const & mesh_resource_ids
) {
    glFinish();
    glFlush();

    ModelBufferHandles buffers = m_model_buffer_handles[handle];

    GL_CHECK(glDeleteBuffers(1, &buffers.vbo));
    GL_CHECK(glDeleteBuffers(1, &buffers.ebo));
    GL_CHECK(glDeleteBuffers(1, &buffers.vao));

    m_model_buffer_handles.erase(handle);

    for (auto const & id : mesh_resource_ids) {
        m_meshes.erase(id);
    }
}

ResourceID GLModelManager::upload_pos_mesh(
    glm::vec3 const * vertices,
    size_t n
) {
    // upload model buffers
    GLuint vao;
    GLuint vbo;

    GL_CHECK(glGenVertexArrays(1, &vao));
    GL_CHECK(glBindVertexArray(vao));

    GL_CHECK(glGenBuffers(1, &vbo));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));

    GL_CHECK(glBufferData(
        GL_ARRAY_BUFFER,
        n * sizeof(vertices[0]),
        vertices,
        GL_STATIC_DRAW
    ));

    GLShader const & shader = m_material_manager.wireframe_shader();

    static const GLVarString pos_str = "a_Position";
    GLint pos_attr;
    GL_CHECK(pos_attr = shader.get_attrib_loc(pos_str));
    GL_CHECK(glEnableVertexAttribArray(pos_attr));
    GL_CHECK(glVertexAttribPointer(
        pos_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        reinterpret_cast<void*>(0)
    ));

    // Create gl meshes
    ResourceID id = m_next_mesh_id;
    ++m_next_mesh_id;

    m_pos_mesh_buffer_handles[id] = {vao, vbo};

    GLMesh & gl_mesh = m_meshes[id];
    gl_mesh.init(vao, 0, static_cast<uint32_t>(n));

    GL_CHECK(glBindVertexArray(0));

    return id;
}

void GLModelManager::update_pos_mesh(
    ResourceID id,
    glm::vec3 const * vertices,
    size_t n
) {
    PosMeshBufferHandles buffers = m_pos_mesh_buffer_handles[id];

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffers.vbo));

    GL_CHECK(glBufferData(
        GL_ARRAY_BUFFER,
        n * sizeof(vertices[0]),
        vertices,
        GL_STATIC_DRAW
    ));

    GLMesh & gl_mesh = m_meshes[id];
    gl_mesh.init(buffers.vao, 0, static_cast<uint32_t>(n));
}

void GLModelManager::free_pos_mesh(ResourceID id) {
    PosMeshBufferHandles buffers = m_pos_mesh_buffer_handles[id];

    GL_CHECK(glDeleteBuffers(1, &buffers.vbo));
    GL_CHECK(glDeleteBuffers(1, &buffers.vao));

    m_pos_mesh_buffer_handles.erase(id);
    m_meshes.erase(id);
}

void GLModelManager::bind_vertex_buffer(Model const & model) {
    std::vector<Model::Vertex> const & vertices = model.vertex_buffer();
    GL_CHECK(glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(vertices[0]),
        vertices.data(),
        GL_STATIC_DRAW
    ));

    GLShader const & shader = m_material_manager.standard_shader();

    static const GLVarString pos_str = "a_Position";
    GLint pos_attr;
    GL_CHECK(pos_attr = shader.get_attrib_loc(pos_str));
    GL_CHECK(glEnableVertexAttribArray(pos_attr));
    GL_CHECK(glVertexAttribPointer(
        pos_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::Vertex),
        reinterpret_cast<void*>(offsetof(Model::Vertex, position))
    ));

    static const GLVarString normal_str = "a_Normal";
    GLint normal_attr;
    GL_CHECK(normal_attr = shader.get_attrib_loc(normal_str));
    GL_CHECK(glEnableVertexAttribArray(normal_attr));
    GL_CHECK(glVertexAttribPointer(
        normal_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::Vertex),
        reinterpret_cast<void*>(offsetof(Model::Vertex, normal))
    ));

    static const GLVarString tex_coord_str = "a_TexCoordinate";
    GLint texcoord_attr;
    GL_CHECK(texcoord_attr = shader.get_attrib_loc(tex_coord_str));
    GL_CHECK(glEnableVertexAttribArray(texcoord_attr));
    GL_CHECK(glVertexAttribPointer(
        texcoord_attr,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::Vertex),
        reinterpret_cast<void*>(offsetof(Model::Vertex, texture_coordinate))
    ));

    static const GLVarString tan_str = "a_Tangent";
    GLint tan_attr;
    GL_CHECK(tan_attr = shader.get_attrib_loc(tan_str));
    if (tan_attr != -1) {
        GL_CHECK(glEnableVertexAttribArray(tan_attr));
        GL_CHECK(glVertexAttribPointer(
            tan_attr,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Model::Vertex),
            reinterpret_cast<void*>(offsetof(Model::Vertex, tangent))
        ));
    }

    static const GLVarString bitan_str = "a_Bitangent";
    GLint bitan_attr;
    GL_CHECK(bitan_attr = shader.get_attrib_loc(bitan_str));
    if (bitan_attr != -1) {
        GL_CHECK(glEnableVertexAttribArray(bitan_attr));
        GL_CHECK(glVertexAttribPointer(
            bitan_attr, 3, GL_FLOAT, GL_FALSE,
            sizeof(Model::Vertex),
            reinterpret_cast<void*>(offsetof(Model::Vertex, bitangent))
        ));
    }
}

void GLModelManager::bind_boned_vertex_buffer(Model const & model) {
    thread_local std::vector<Model::BonedVertex> boned_vertices;
    boned_vertices.resize(model.vertex_buffer().size());

    std::vector<Model::Vertex> const & vertices = model.vertex_buffer();
    std::vector<Model::BoneData> const & bone_data = model.vertex_bone_buffer();

    for (size_t i = 0; i < vertices.size(); ++i) {
        boned_vertices[i].vertex_data = vertices[i];
        boned_vertices[i].bone_data = bone_data[i];
    }

    GL_CHECK(glBufferData(
        GL_ARRAY_BUFFER,
        boned_vertices.size() * sizeof(boned_vertices[0]),
        boned_vertices.data(),
        GL_STATIC_DRAW
    ));

    GLuint shader_program =
        m_material_manager.standard_animated_shader().shader();

    GLint pos_attr;
    GL_CHECK(pos_attr = glGetAttribLocation(shader_program, "a_Position"));
    GL_CHECK(glEnableVertexAttribArray(pos_attr));
    GL_CHECK(glVertexAttribPointer(
        pos_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::BonedVertex),
        reinterpret_cast<void*>(0)
    ));

    GLint normal_attr;
    GL_CHECK(normal_attr = glGetAttribLocation(shader_program, "a_Normal"));
    GL_CHECK(glEnableVertexAttribArray(normal_attr));
    GL_CHECK(glVertexAttribPointer(
        normal_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::BonedVertex),
        reinterpret_cast<void*>(12)
    ));

    GLint texcoord_attr;
    GL_CHECK(texcoord_attr = glGetAttribLocation(shader_program, "a_TexCoordinate"));
    GL_CHECK(glEnableVertexAttribArray(texcoord_attr));
    GL_CHECK(glVertexAttribPointer(
        texcoord_attr,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::BonedVertex),
        reinterpret_cast<void*>(24)
    ));

    GLint tan_attr;
    GL_CHECK(tan_attr = glGetAttribLocation(shader_program, "a_Tangent"));
    if (tan_attr != -1) {
        GL_CHECK(glEnableVertexAttribArray(tan_attr));
        GL_CHECK(glVertexAttribPointer(
            tan_attr,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Model::BonedVertex),
            reinterpret_cast<void*>(32)
        ));
    }

    GLint bitan_attr;
    GL_CHECK(bitan_attr = glGetAttribLocation(shader_program, "a_Bitangent"));
    if (bitan_attr != -1) {
        GL_CHECK(glEnableVertexAttribArray(bitan_attr));
        GL_CHECK(glVertexAttribPointer(
            bitan_attr, 3, GL_FLOAT, GL_FALSE,
            sizeof(Model::BonedVertex),
            reinterpret_cast<void*>(44)
        ));
    }

    GLint bone_id_attr;
    GL_CHECK(bone_id_attr = glGetAttribLocation(shader_program, "a_BoneIDs"));
    if (bone_id_attr != -1) {
        GL_CHECK(glEnableVertexAttribArray(bone_id_attr));
        GL_CHECK(glVertexAttribIPointer(
            bone_id_attr, 4, GL_UNSIGNED_INT,
            sizeof(Model::BonedVertex),
            reinterpret_cast<void*>(56)
        ));
    }

    GLint bone_weights_attr;
    GL_CHECK(bone_weights_attr = glGetAttribLocation(shader_program, "a_BoneWeights"));
    if (bone_weights_attr != -1) {
        GL_CHECK(glEnableVertexAttribArray(bone_weights_attr));
        GL_CHECK(glVertexAttribPointer(
            bone_weights_attr, 4, GL_FLOAT, GL_FALSE,
            sizeof(Model::BonedVertex),
            reinterpret_cast<void*>(72)
        ));
    }
}
