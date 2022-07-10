#include "gl_renderer.h"

#include "src/driver/opengl/gl_texture.h"
#include "src/driver/opengl/gl_utility.h"

#include <SDL_image.h>

using namespace prt3;

GLRenderer::GLRenderer(SDL_Window * window)
 : m_window{window},
   m_standard_shader{"assets/shaders/opengl/standard.vs",
                     "assets/shaders/opengl/standard.fs"} {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    glEnable(GL_DEPTH_TEST);
    glCheckError();
    glDepthFunc(GL_LESS);
    glCheckError();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glCheckError();
    glEnable(GL_CULL_FACE);
    glCheckError();
    glCullFace(GL_BACK);
    glCheckError();

    load_default_texture();
}

GLRenderer::~GLRenderer() {
    // TODO: implement
}

void GLRenderer::render(RenderData const & render_data) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();
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

    GLuint shader_program = m_standard_shader.ID;

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
        material_ids[material_index] = upload_material(material);
        ++material_index;
    }
    // Create gl meshes
    resource.mesh_resource_ids.resize(model.meshes().size());
    size_t mesh_index = 0;
    for (Model::Mesh const & mesh : model.meshes()) {
        ResourceID id = m_meshes.size();

        m_meshes.push_back({});
        GLMesh & gl_mesh = m_meshes.back();
        gl_mesh.init(vao, mesh.start_index, mesh.num_indices, {});

        resource.mesh_resource_ids[mesh_index] = id;
        resource.material_resource_ids[mesh_index] = material_ids[mesh.material_index];
        ++mesh_index;
    }

    glBindVertexArrayOES(0);
    glCheckError();
}

ResourceID GLRenderer::upload_material(Model::Material const & material) {
    ResourceID id = m_materials.size();
    m_materials.emplace_back(m_standard_shader,
                             retrieve_texture(material.albedo_map.c_str()),
                             retrieve_texture(material.normal_map.c_str()),
                             retrieve_texture(material.roughness_map.c_str()));
    return id;
}

GLuint GLRenderer::retrieve_texture(char const * path) {
    std::string path_str{path};
    if (m_textures.find(path_str) == m_textures.end()) {
        load_texture(path);
    }
    if (m_textures.find(path_str) != m_textures.end()) {
        return m_textures[path_str];
    }
    return m_default_texture;
}

void GLRenderer::load_texture(char const * path) {
    if (path[0] != '\0') {
        SDL_Surface * image = IMG_Load(path);

        // TODO: proper format detection
        GLenum format = GL_RGB;
        if(image->format->BytesPerPixel == 4) {
            format = GL_RGBA;
        }

        GLuint texture_handle;
        if (image) {
            glGenTextures(1, &texture_handle);
            glBindTexture(GL_TEXTURE_2D, texture_handle);
            glTexImage2D(GL_TEXTURE_2D, 0, format, image->w, image->h, 0,
                         format, GL_UNSIGNED_BYTE, image->pixels);
            glGenerateMipmap(GL_TEXTURE_2D);

            SDL_FreeSurface(image);

            m_textures[std::string(path)] = texture_handle;
        } else {
            // TODO: proper error handling
            std::cout << "failed to load texture at path \"" << path << "\"." << std::endl;
            std::cout << "IMG_Load: " << IMG_GetError() << std::endl;
        }
    } else {
        // No texture path
    }
}

void GLRenderer::load_default_texture() {
    // Default texture is a 1x1 white image
    unsigned char data[4] = { 255, 255, 255, 255 };

    glGenTextures(1, &m_default_texture);
    glBindTexture(GL_TEXTURE_2D, m_default_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, &data);
    // glGenerateMipmap(GL_TEXTURE_2D); Probably not needed for 1x1 image?
}
