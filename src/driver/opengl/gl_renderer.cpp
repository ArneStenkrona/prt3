#include "gl_renderer.h"

#include "src/driver/opengl/gl_texture.h"

using namespace prt3;

GLRenderer::GLRenderer(SDL_Window * window)
 : m_window{window} {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
}

void GLRenderer::upload_model(ModelManager::ModelHandle model_handle,
                              Model const & model,
                              ModelResource & resource) {
    // upload model buffers
    GLuint vao;
    GLuint vbo;
    glGenVertexArraysOES(1, &vao);
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    std::vector<Model::Vertex> const & vertices = model.vertex_buffer();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Model::Vertex), &vertices[0], GL_STATIC_DRAW);

    GLMaterial & default_material = m_materials[RenderBackend::DEFAULT_MATERIAL_ID];
    GLuint shader_program = default_material.shader().ID;

    GLint pos_attr = glGetAttribLocation(shader_program, "a_Position");
    glEnableVertexAttribArray(pos_attr);
    glVertexAttribPointer(pos_attr, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Model::Vertex),
                          reinterpret_cast<void*>(offsetof(Model::Vertex, position)));

    GLint normal_attr = glGetAttribLocation(shader_program, "a_Normal");
    glEnableVertexAttribArray(normal_attr);
    glVertexAttribPointer(normal_attr, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Model::Vertex),
                          reinterpret_cast<void*>(offsetof(Model::Vertex, normal)));

    GLint texcoord_attr = glGetAttribLocation(shader_program, "a_TexCoordinate");
    glEnableVertexAttribArray(texcoord_attr);
    glVertexAttribPointer(texcoord_attr, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Model::Vertex),
                          reinterpret_cast<void*>(offsetof(Model::Vertex, texture_coordinate)));

    GLint tan_attr = glGetAttribLocation(shader_program, "a_Tangent");
    glEnableVertexAttribArray(tan_attr);
    glVertexAttribPointer(tan_attr, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Model::Vertex),
                          reinterpret_cast<void*>(offsetof(Model::Vertex, tangent)));

    GLint bitan_attr = glGetAttribLocation(shader_program, "a_Bitangent");
    glEnableVertexAttribArray(bitan_attr);
    glVertexAttribPointer(bitan_attr, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Model::Vertex),
                          reinterpret_cast<void*>(offsetof(Model::Vertex, bitangent)));

    glBindVertexArrayOES(0);

    m_buffer_handles[model_handle] = {vao, vbo};

    // upload meshes
    resource.mesh_resource_ids.resize(model.meshes().size());
    resource.material_resource_ids.resize(model.meshes().size());
    size_t mesh_index = 0;
    for (Model::Mesh const & mesh : model.meshes()) {

        ResourceID id = m_meshes.size();

        std::vector<uint32_t> index_buffer;
        index_buffer.resize(mesh.num_indices);
        for (uint32_t i = mesh.start_index; i < mesh.num_indices; ++i)
        {
            index_buffer[i] = model.index_buffer()[mesh.start_index + i];
        }
        m_meshes.push_back({});
        GLMesh & gl_mesh = m_meshes.back();
        gl_mesh.init(vao, vbo, index_buffer, {});

        resource.mesh_resource_ids[mesh_index] = id;
        resource.material_resource_ids[mesh_index] = RenderBackend::DEFAULT_MATERIAL_ID;
        ++id;
    }
}