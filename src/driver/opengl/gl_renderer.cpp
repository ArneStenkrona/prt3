#include "gl_renderer.h"

#include "src/driver/opengl/gl_utility.h"

#include <vector>
#include <unordered_map>
#include <cassert>

using namespace prt3;

GLRenderer::GLRenderer(SDL_Window * window)
 : m_window{window},
   m_material_manager{m_texture_manager},
   m_model_manager{m_material_manager}
  {
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

        std::vector<GLMaterial> const & materials = m_material_manager.materials();
    for (auto const & pair : m_material_queues) {
        GLMaterial material = materials[pair.first];
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

        std::vector<GLMesh> const & meshes = m_model_manager.meshes();
        for (MeshRenderData const & mesh_data : pair.second) {
            meshes[mesh_data.mesh_id].draw(
                materials[mesh_data.material_id],
                render_data.scene_data,
                mesh_data
            );
        }
    }

    SDL_GL_SwapWindow(m_window);
    glCheckError();
}
