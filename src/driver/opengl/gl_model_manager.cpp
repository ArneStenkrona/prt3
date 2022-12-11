#include "gl_model_manager.h"

#include "src/driver/opengl/gl_utility.h"

#include <unordered_set>

using namespace prt3;

GLModelManager::GLModelManager(GLMaterialManager & material_manager)
 : m_material_manager{material_manager} {

}

void GLModelManager::upload_model(ModelHandle handle,
                                  Model const & model,
                                  ModelResource & resource) {
    // upload model buffers
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    glGenVertexArrays(1, &vao);
    glCheckError();
    glBindVertexArray(vao);
    glCheckError();

    glGenBuffers(1, &vbo);
    glCheckError();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glCheckError();

    if (!model.is_animated()) {
        bind_vertex_buffer(model);
    } else {
        bind_vertex_buffer(model);
    }

    glGenBuffers(1, &ebo);
    glCheckError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glCheckError();
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        model.index_buffer().size() * sizeof(uint32_t),
        model.index_buffer().data(),
        GL_STATIC_DRAW
    );
    glCheckError();

    m_model_buffer_handles[handle] = {vao, vbo, ebo};

    // Create gl materials
    resource.material_resource_ids.resize(model.meshes().size());
    std::vector<ResourceID> material_ids;
    material_ids.resize(model.materials().size());
    size_t material_index = 0;
    for (Material const & material : model.materials()) {
        material_ids[material_index] =
            m_material_manager.upload_material(material);
        ++material_index;
    }
    // Create gl meshes
    resource.mesh_resource_ids.resize(model.meshes().size());
    size_t mesh_index = 0;
    for (Model::Mesh const & mesh : model.meshes()) {
        ResourceID id = m_next_mesh_id;
        ++m_next_mesh_id;

        GLMesh & gl_mesh = m_meshes[id];
        gl_mesh.init(vao, mesh.start_index, mesh.num_indices);

        resource.mesh_resource_ids[mesh_index] = id;
        resource.material_resource_ids[mesh_index] = material_ids[mesh.material_index];
        ++mesh_index;
    }

    glBindVertexArray(0);
    glCheckError();
}

void GLModelManager::free_model(
    ModelHandle handle,
    ModelResource const & resource
) {
    glFinish();
    glFlush();

    ModelBufferHandles buffers = m_model_buffer_handles[handle];

    glDeleteBuffers(1, &buffers.vbo);
    glCheckError();
    glDeleteBuffers(1, &buffers.ebo);
    glCheckError();
    glDeleteBuffers(1, &buffers.vao);
    glCheckError();

    m_model_buffer_handles.erase(handle);

    for (auto const & id : resource.mesh_resource_ids) {
        m_meshes.erase(id);
    }

    static std::unordered_set<ResourceID> free_set;
    free_set.clear();

    for (auto const & id : resource.material_resource_ids) {
        if (free_set.find(id) != free_set.end()) {
            continue;
        }

        m_material_manager.free_material(id);
        free_set.insert(id);
    }
}

ResourceID GLModelManager::upload_line_mesh(
    std::vector<glm::vec3> const & vertices
) {
    // upload model buffers
    GLuint vao;
    GLuint vbo;

    glGenVertexArrays(1, &vao);
    glCheckError();
    glBindVertexArray(vao);
    glCheckError();

    glGenBuffers(1, &vbo);
    glCheckError();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glCheckError();

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(vertices[0]),
        vertices.data(),
        GL_STATIC_DRAW
    );
    glCheckError();

    GLuint shader_program = m_material_manager.wireframe_shader().shader();

    GLint pos_attr = glGetAttribLocation(shader_program, "a_Position");
    glCheckError();
    glEnableVertexAttribArray(pos_attr);
    glCheckError();
    glVertexAttribPointer(
        pos_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        reinterpret_cast<void*>(0)
    );
    glCheckError();


    // Create gl meshes
    ResourceID id = m_next_mesh_id;
    ++m_next_mesh_id;

    m_line_mesh_buffer_handles[id] = {vao, vbo};

    GLMesh & gl_mesh = m_meshes[id];
    gl_mesh.init(vao, 0, static_cast<uint32_t>(vertices.size()));

    glBindVertexArray(0);
    glCheckError();

    return id;
}

void GLModelManager::update_line_mesh(
    ResourceID id,
    std::vector<glm::vec3> const & vertices
) {
    LineMeshBufferHandles buffers = m_line_mesh_buffer_handles[id];

    glBindBuffer(GL_ARRAY_BUFFER, buffers.vbo);
    glCheckError();

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(vertices[0]),
        vertices.data(),
        GL_STATIC_DRAW
    );
    glCheckError();
}

void GLModelManager::free_line_mesh(ResourceID id) {
    LineMeshBufferHandles buffers = m_line_mesh_buffer_handles[id];

    glDeleteBuffers(1, &buffers.vbo);
    glCheckError();
    glDeleteBuffers(1, &buffers.vao);
    glCheckError();

    m_line_mesh_buffer_handles.erase(id);
    m_meshes.erase(id);
}

void GLModelManager::bind_vertex_buffer(Model const & model) {
    std::vector<Model::Vertex> const & vertices = model.vertex_buffer();
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(vertices[0]),
        vertices.data(),
        GL_STATIC_DRAW
    );
    glCheckError();

    GLuint shader_program = m_material_manager.standard_shader().shader();

    GLint pos_attr = glGetAttribLocation(shader_program, "a_Position");
    glCheckError();
    glEnableVertexAttribArray(pos_attr);
    glCheckError();
    glVertexAttribPointer(
        pos_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::Vertex),
        reinterpret_cast<void*>(offsetof(Model::Vertex, position))
    );
    glCheckError();

    GLint normal_attr = glGetAttribLocation(shader_program, "a_Normal");
    glCheckError();
    glEnableVertexAttribArray(normal_attr);
    glCheckError();
    glVertexAttribPointer(
        normal_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::Vertex),
        reinterpret_cast<void*>(offsetof(Model::Vertex, normal))
    );
    glCheckError();

    GLint texcoord_attr = glGetAttribLocation(shader_program, "a_TexCoordinate");
    glCheckError();
    glEnableVertexAttribArray(texcoord_attr);
    glCheckError();
    glVertexAttribPointer(
        texcoord_attr,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::Vertex),
        reinterpret_cast<void*>(offsetof(Model::Vertex, texture_coordinate))
    );
    glCheckError();

    GLint tan_attr = glGetAttribLocation(shader_program, "a_Tangent");
    glCheckError();
    if (tan_attr != -1) {
        glEnableVertexAttribArray(tan_attr);
        glCheckError();
        glVertexAttribPointer(
            tan_attr,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Model::Vertex),
            reinterpret_cast<void*>(offsetof(Model::Vertex, tangent))
        );
        glCheckError();
    }

    GLint bitan_attr = glGetAttribLocation(shader_program, "a_Bitangent");
    glCheckError();
    if (bitan_attr != -1) {
        glEnableVertexAttribArray(bitan_attr);
        glCheckError();
        glVertexAttribPointer(
            bitan_attr, 3, GL_FLOAT, GL_FALSE,
            sizeof(Model::Vertex),
            reinterpret_cast<void*>(offsetof(Model::Vertex, bitangent))
        );
        glCheckError();
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

    glBufferData(
        GL_ARRAY_BUFFER,
        boned_vertices.size() * sizeof(boned_vertices[0]),
        boned_vertices.data(),
        GL_STATIC_DRAW
    );
    glCheckError();

    GLuint shader_program =
        m_material_manager.standard_animated_shader().shader();

    GLint pos_attr = glGetAttribLocation(shader_program, "a_Position");
    glCheckError();
    glEnableVertexAttribArray(pos_attr);
    glCheckError();
    glVertexAttribPointer(
        pos_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::BonedVertex),
        reinterpret_cast<void*>(0)
    );
    glCheckError();

    GLint normal_attr = glGetAttribLocation(shader_program, "a_Normal");
    glCheckError();
    glEnableVertexAttribArray(normal_attr);
    glCheckError();
    glVertexAttribPointer(
        normal_attr,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::BonedVertex),
        reinterpret_cast<void*>(12)
    );
    glCheckError();

    GLint texcoord_attr = glGetAttribLocation(shader_program, "a_TexCoordinate");
    glCheckError();
    glEnableVertexAttribArray(texcoord_attr);
    glCheckError();
    glVertexAttribPointer(
        texcoord_attr,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Model::BonedVertex),
        reinterpret_cast<void*>(24)
    );
    glCheckError();

    GLint tan_attr = glGetAttribLocation(shader_program, "a_Tangent");
    glCheckError();
    if (tan_attr != -1) {
        glEnableVertexAttribArray(tan_attr);
        glCheckError();
        glVertexAttribPointer(
            tan_attr,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Model::BonedVertex),
            reinterpret_cast<void*>(32)
        );
        glCheckError();
    }

    GLint bitan_attr = glGetAttribLocation(shader_program, "a_Bitangent");
    glCheckError();
    if (bitan_attr != -1) {
        glEnableVertexAttribArray(bitan_attr);
        glCheckError();
        glVertexAttribPointer(
            bitan_attr, 3, GL_FLOAT, GL_FALSE,
            sizeof(Model::BonedVertex),
            reinterpret_cast<void*>(44)
        );
        glCheckError();
    }

    GLint bone_id_attr = glGetAttribLocation(shader_program, "a_BoneIDs");
    glCheckError();
    if (bone_id_attr != -1) {
        glEnableVertexAttribArray(bone_id_attr);
        glCheckError();
        glVertexAttribPointer(
            bone_id_attr, 4, GL_UNSIGNED_INT, GL_FALSE,
            sizeof(Model::BonedVertex),
            reinterpret_cast<void*>(56)
        );
        glCheckError();
    }

    GLint bone_weights_attr = glGetAttribLocation(shader_program, "a_BoneWeights");
    glCheckError();
    if (bone_weights_attr != -1) {
        glEnableVertexAttribArray(bone_weights_attr);
        glCheckError();
        glVertexAttribPointer(
            bone_weights_attr, 4, GL_FLOAT, GL_FALSE,
            sizeof(Model::BonedVertex),
            reinterpret_cast<void*>(72)
        );
        glCheckError();
    }
}
