#include "gl_model_manager.h"

#include "src/driver/opengl/gl_utility.h"

using namespace prt3;

GLModelManager::GLModelManager(GLMaterialManager & material_manager)
 : m_material_manager{material_manager} {

}

void GLModelManager::upload_model(ModelManager::ModelHandle model_handle,
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

    std::vector<Model::Vertex> const & vertices = model.vertex_buffer();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
    glCheckError();

    GLuint shader_program = m_material_manager.standard_shader();

    GLint pos_attr = glGetAttribLocation(shader_program, "a_Position");
    glCheckError();
    glEnableVertexAttribArray(pos_attr);
    glCheckError();
    glVertexAttribPointer(pos_attr, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Model::Vertex),
                          reinterpret_cast<void*>(offsetof(Model::Vertex, position)));
    glCheckError();

    GLint normal_attr = glGetAttribLocation(shader_program, "a_Normal");
    glCheckError();
    glEnableVertexAttribArray(normal_attr);
    glCheckError();
    glVertexAttribPointer(normal_attr, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Model::Vertex),
                          reinterpret_cast<void*>(offsetof(Model::Vertex, normal)));
    glCheckError();

    GLint texcoord_attr = glGetAttribLocation(shader_program, "a_TexCoordinate");
    glCheckError();
    glEnableVertexAttribArray(texcoord_attr);
    glCheckError();
    glVertexAttribPointer(texcoord_attr, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Model::Vertex),
                          reinterpret_cast<void*>(offsetof(Model::Vertex, texture_coordinate)));
    glCheckError();

    GLint tan_attr = glGetAttribLocation(shader_program, "a_Tangent");
    glCheckError();
    if (tan_attr != -1) {
        glEnableVertexAttribArray(tan_attr);
        glCheckError();
        glVertexAttribPointer(tan_attr, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Model::Vertex),
                              reinterpret_cast<void*>(offsetof(Model::Vertex, tangent)));
        glCheckError();
    }

    GLint bitan_attr = glGetAttribLocation(shader_program, "a_Bitangent");
    glCheckError();
    if (bitan_attr != -1) {
        glEnableVertexAttribArray(bitan_attr);
        glCheckError();
        glVertexAttribPointer(bitan_attr, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Model::Vertex),
                              reinterpret_cast<void*>(offsetof(Model::Vertex, bitangent)));
        glCheckError();
    }

    glGenBuffers(1, &ebo);
    glCheckError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glCheckError();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.index_buffer().size() * sizeof(uint32_t),
                 model.index_buffer().data(), GL_STATIC_DRAW);
    glCheckError();

    m_buffer_handles[model_handle] = {vao, vbo, ebo};

    // Create gl materials
    resource.material_resource_ids.resize(model.meshes().size());
    std::vector<ResourceID> material_ids;
    material_ids.resize(model.materials().size());
    size_t material_index = 0;
    for (Model::Material const & material : model.materials()) {
        material_ids[material_index] = m_material_manager.upload_material(material);
        ++material_index;
    }
    // Create gl meshes
    resource.mesh_resource_ids.resize(model.meshes().size());
    size_t mesh_index = 0;
    for (Model::Mesh const & mesh : model.meshes()) {
        ResourceID id = m_meshes.size();

        m_meshes.push_back({});
        GLMesh & gl_mesh = m_meshes.back();
        gl_mesh.init(vao, mesh.start_index, mesh.num_indices);

        resource.mesh_resource_ids[mesh_index] = id;
        resource.material_resource_ids[mesh_index] = material_ids[mesh.material_index];
        ++mesh_index;
    }

    glBindVertexArray(0);
    glCheckError();
}
