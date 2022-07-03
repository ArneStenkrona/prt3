#include "gl_renderer.h"

#include "src/driver/opengl/gl_texture.h"

#include "src/driver/opengl/gl_utility.h"

using namespace prt3;

GLRenderer::GLRenderer(SDL_Window * window)
 : m_window{window} {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glCheckError();
    glDisable(GL_CULL_FACE);
    glCheckError();

    /* create default material */
    m_materials.emplace_back("assets/shaders/opengl/standard.vs",
                             "assets/shaders/opengl/standard.fs");
}

GLRenderer::~GLRenderer() {
    // TODO: implement
}

void GLRenderer::render(RenderData const & render_data) {
    glClear(GL_COLOR_BUFFER_BIT);
    for (MeshRenderData const & mesh_data : render_data.mesh_data) {
        m_meshes[mesh_data.mesh_id].draw(
            m_materials[mesh_data.material_id],
            render_data.scene_data,
            mesh_data
        );
    }
    SDL_GL_SwapWindow(m_window);
    glCheckError();
}

void GLRenderer::upload_model(ModelManager::ModelHandle model_handle,
                              Model const & model,
                              ModelResource & resource) {
    // upload model buffers
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    glGenVertexArraysOES(1, &vao);
    glCheckError();
    glBindVertexArrayOES(vao);
    glCheckError();

    glGenBuffers(1, &vbo);
    glCheckError();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glCheckError();

    std::vector<Model::Vertex> const & vertices = model.vertex_buffer();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
    glCheckError();

    GLMaterial & default_material = m_materials[RenderBackend::DEFAULT_MATERIAL_ID];
    GLuint shader_program = default_material.shader().ID;

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

    // Create gl meshes
    resource.mesh_resource_ids.resize(model.meshes().size());
    resource.material_resource_ids.resize(model.meshes().size());
    size_t mesh_index = 0;
    for (Model::Mesh const & mesh : model.meshes()) {
        ResourceID id = m_meshes.size();

        m_meshes.push_back({});
        GLMesh & gl_mesh = m_meshes.back();
        gl_mesh.init(vao, mesh.start_index, mesh.num_indices, {});

        resource.mesh_resource_ids[mesh_index] = id;
        resource.material_resource_ids[mesh_index] = RenderBackend::DEFAULT_MATERIAL_ID;
    }

    glBindVertexArrayOES(0);
    glCheckError();
}