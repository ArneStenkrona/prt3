#include "gl_renderer.h"

#include "src/driver/opengl/gl_texture.h"
#include "src/driver/opengl/gl_utility.h"

#include <SDL_image.h>

#include <cassert>
#include <iostream>

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

    unsigned char data_0xffffffff[4] = { 0xff, 0xff, 0xff, 0xff };
    m_texture_1x1_0xffffffff = load_texture(data_0xffffffff, 1, 1, GL_RGBA,
                                            false);
    unsigned char data_0x000000ff[3] = { 0x00, 0x00, 0xff };
    m_texture_1x1_0x0000ff = load_texture(data_0x000000ff, 1, 1, GL_RGB,
                                          false);
    unsigned char data_0x80[3] = { 0x80 };
    m_texture_1x1_0x80 = load_texture(data_0x80, 1, 1, GL_LUMINANCE,
                                      false);
}

GLRenderer::~GLRenderer() {
    // TODO: implement
}

void GLRenderer::render(RenderData const & render_data) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();
    std::unordered_map<ResourceID, std::vector<MeshRenderData>> m_material_queues;
    // Render meshes
    for (MeshRenderData const & mesh_data : render_data.mesh_data) {
        m_material_queues[mesh_data.material_id].push_back(mesh_data);
    }

    for (auto const & pair : m_material_queues) {
        GLMaterial material = m_materials[pair.first];
        material.shader().use();
        glCheckError();
        material.shader().setVec3("u_ViewPosition", render_data.scene_data.view_position);
        glCheckError();
        // Light data
        LightRenderData const & light_data = render_data.light_data;
        material.shader().setInt("u_NumberOfPointLights", static_cast<int>(light_data.number_of_point_lights));

        material.shader().setVec3("u_PointLights[0].position", light_data.point_lights[0].position);
        material.shader().setVec3("u_PointLights[0].color", light_data.point_lights[0].light.color);
        material.shader().setFloat("u_PointLights[0].a", light_data.point_lights[0].light.quadratic_term);
        material.shader().setFloat("u_PointLights[0].b", light_data.point_lights[0].light.linear_term);
        material.shader().setFloat("u_PointLights[0].c", light_data.point_lights[0].light.constant_term);

        material.shader().setVec3("u_PointLights[1].position", light_data.point_lights[1].position);
        material.shader().setVec3("u_PointLights[1].color", light_data.point_lights[1].light.color);
        material.shader().setFloat("u_PointLights[1].a", light_data.point_lights[1].light.quadratic_term);
        material.shader().setFloat("u_PointLights[1].b", light_data.point_lights[1].light.linear_term);
        material.shader().setFloat("u_PointLights[1].c", light_data.point_lights[1].light.constant_term);

        material.shader().setVec3("u_PointLights[2].position", light_data.point_lights[2].position);
        material.shader().setVec3("u_PointLights[2].color", light_data.point_lights[2].light.color);
        material.shader().setFloat("u_PointLights[2].a", light_data.point_lights[2].light.quadratic_term);
        material.shader().setFloat("u_PointLights[2].b", light_data.point_lights[2].light.linear_term);
        material.shader().setFloat("u_PointLights[2].c", light_data.point_lights[2].light.constant_term);

        material.shader().setVec3("u_PointLights[3].position", light_data.point_lights[3].position);
        material.shader().setVec3("u_PointLights[3].color", light_data.point_lights[3].light.color);
        material.shader().setFloat("u_PointLights[3].a", light_data.point_lights[3].light.quadratic_term);
        material.shader().setFloat("u_PointLights[3].b", light_data.point_lights[3].light.linear_term);
        material.shader().setFloat("u_PointLights[3].c", light_data.point_lights[3].light.constant_term);
        glCheckError();

        for (MeshRenderData const & mesh_data : pair.second) {
            m_meshes[mesh_data.mesh_id].draw(
                m_materials[mesh_data.material_id],
                render_data.scene_data,
                mesh_data
            );
        }
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
                             retrieve_texture(material.albedo_map.c_str(),
                                m_texture_1x1_0xffffffff),
                             retrieve_texture(material.normal_map.c_str(),
                                m_texture_1x1_0x0000ff),
                             retrieve_texture(material.roughness_map.c_str(),
                                m_texture_1x1_0x80),
                            retrieve_texture(material.metallic_map.c_str(),
                                m_texture_1x1_0x80),
                             material.albedo,
                             material.metallic,
                             material.roughness);

    return id;
}

GLuint GLRenderer::retrieve_texture(char const * path, GLuint default_texture) {
    std::string path_str{path};
    if (m_textures.find(path_str) == m_textures.end()) {
        load_texture(path);
    }
    if (m_textures.find(path_str) != m_textures.end()) {
        return m_textures[path_str];
    }
    return default_texture;
}

void GLRenderer::load_texture(char const * path) {
    if (path[0] != '\0') {
        SDL_Surface * image = IMG_Load(path);


        GLuint texture_handle;
        if (image) {
            // TODO: proper format detection
            GLenum format = 0;
            switch (image->format->BytesPerPixel) {
                case 1: {
                    format = GL_LUMINANCE;
                    break;
                }
                case 2: {
                    format = GL_LUMINANCE_ALPHA;
                    break;
                }
                case 3: {
                    format = GL_RGB;
                    break;
                }
                case 4: {
                    format = GL_RGBA;
                    break;
                }
                default: {
                    assert(false && "Invalid number of channels");
                }
            }

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

GLuint GLRenderer::load_texture(unsigned char * data,
                                int w, int h,
                                GLenum format,
                                bool mipmap) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
                    format, GL_UNSIGNED_BYTE, data);
    if (mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    return texture;
}
