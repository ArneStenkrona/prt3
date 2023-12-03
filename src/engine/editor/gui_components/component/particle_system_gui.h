#ifndef PRT3_PARTICLE_SYSTEM_GUI_H
#define PRT3_PARTICLE_SYSTEM_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/component_gui_utility.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/particle_system.h"

#include "imgui.h"

#include <string>

namespace prt3 {

bool edit_float_spread(
    EditorContext & context,
    NodeID id,
    char const * label,
    ParticleSystem::FloatSpread & fs
) {
    ImGui::Text("%s", label);
    ImGui::PushID(label);

    ParticleSystem::FloatSpread new_fs = fs;
    if (display_value<float>("min", new_fs.min)) {
        new_fs.min = glm::max(new_fs.min, 0.0f);
        new_fs.max = glm::max(new_fs.min, new_fs.max);
    }

    if (display_value<float>("max", new_fs.max)) {
        new_fs.max = glm::max(new_fs.max, 0.0f);
        new_fs.min = glm::min(new_fs.min, new_fs.max);
    }

    ImGui::PopID();

    if (new_fs.min != fs.min || new_fs.max != fs.max) {
        Scene & scene = context.scene();
        ParticleSystem & comp = scene.get_component<ParticleSystem>(id);
        size_t offset = reinterpret_cast<char *>(&fs) -
                        reinterpret_cast<char *>(&comp);

        context.editor().perform_action<ActionSetField<
            ParticleSystem, ParticleSystem::FloatSpread>
        >(
            id,
            offset,
            new_fs
        );

        return true;
    }

    return false;
}

template<>
void inner_show_component<ParticleSystem>(
    EditorContext & context,
    NodeID id
) {
    ImGui::PushItemWidth(125);

    Scene & scene = context.scene();
    ParticleSystem & comp = scene.get_component<ParticleSystem>(id);

    ParticleSystem::Parameters & params = comp.m_parameters;

    /* shape */
    static char const * shapes[] = {
        "point",
        "circle edge",
        "sphere surface",
        "line segment"
    };
    static int32_t shapes_enum[] = {
        static_cast<int32_t>(ParticleSystem::EmissionShape::Type::point),
        static_cast<int32_t>(ParticleSystem::EmissionShape::Type::circle_edge),
        static_cast<int32_t>(ParticleSystem::EmissionShape::Type::sphere_surface),
        static_cast<int32_t>(ParticleSystem::EmissionShape::Type::line),
    };

    static int enum_to_index[] = {
         0, // mesh
         1, // circle_edge
         2, // sphere_surface
         3  // line
    };

    int32_t shape_type = static_cast<int32_t>(params.shape_type);

    static int current_shape = 0;
    current_shape = enum_to_index[shape_type];

    ImGui::Combo("shape", &current_shape, shapes, IM_ARRAYSIZE(shapes));
    bool shape_changed = shapes_enum[current_shape] != shape_type;
    if (shape_changed) {
        context.editor().perform_action<ActionSetField<
            ParticleSystem, int32_t>
        >(
            id,
            offsetof(ParticleSystem, m_parameters.shape_type),
            shapes_enum[current_shape]
        );
    }

    switch (params.shape_type) {
        case ParticleSystem::EmissionShape::Type::point: {
            break;
        }
        case ParticleSystem::EmissionShape::Type::circle_edge: {
            edit_field<ParticleSystem, glm::vec3>(
                context,
                id,
                "direction",
                offsetof(ParticleSystem, m_parameters.shape.circle.direction)
            );

            edit_field<ParticleSystem, float>(
                context,
                id,
                "radius",
                offsetof(ParticleSystem, m_parameters.shape.circle.radius)
            );
            break;
        }
        case ParticleSystem::EmissionShape::Type::sphere_surface: {
            edit_field<ParticleSystem, float>(
                context,
                id,
                "radius",
                offsetof(ParticleSystem, m_parameters.shape.sphere.radius)
            );
            break;
        }
        case ParticleSystem::EmissionShape::Type::line: {
            edit_field<ParticleSystem, glm::vec3>(
                context,
                id,
                "endpoint",
                offsetof(ParticleSystem, m_parameters.shape.line.endpoint)
            );

            edit_field<ParticleSystem, glm::vec3>(
                context,
                id,
                "direction",
                offsetof(ParticleSystem, m_parameters.shape.line.direction)
            );
            break;
        }
    }

    edit_float_spread(context, id, "start scale", params.start_scale);
    edit_float_spread(context, id, "end scale", params.end_scale);

    edit_field<ParticleSystem, glm::vec4>(
        context,
        id,
        "start color",
        offsetof(ParticleSystem, m_parameters.start_color)
    );

    edit_field<ParticleSystem, glm::vec4>(
        context,
        id,
        "end color",
        offsetof(ParticleSystem, m_parameters.end_color)
    );

    edit_field<ParticleSystem, float>(
        context,
        id,
        "emission rate",
        offsetof(ParticleSystem, m_parameters.emission_rate)
    );

    edit_float_spread(context, id, "lifetime", params.lifetime);
    edit_float_spread(context, id, "velocity", params.velocity);
    edit_float_spread(context, id, "dampening", params.dampening);

    edit_field<ParticleSystem, float>(
        context,
        id,
        "gravity",
        offsetof(ParticleSystem, m_parameters.gravity)
    );

    set_texture<ParticleSystem>(
        context,
        id,
        offsetof(ParticleSystem, m_parameters.texture_id)
    );

    if (params.texture_id != NO_RESOURCE) {
        edit_field<ParticleSystem, float>(
            context,
            id,
            "framerate",
            offsetof(ParticleSystem, m_parameters.framerate)
        );

        edit_field<ParticleSystem, uint32_t>(
            context,
            id,
            "number of frames",
            offsetof(ParticleSystem, m_parameters.n_frames)
        );

        edit_field<ParticleSystem, bool>(
            context,
            id,
            "loop animation",
            offsetof(ParticleSystem, m_parameters.loop_animation)
        );
    }

    edit_field<ParticleSystem, bool>(
        context,
        id,
        "prewarm",
        offsetof(ParticleSystem, m_parameters.prewarm)
    );

    edit_field<ParticleSystem, bool>(
        context,
        id,
        "active",
        offsetof(ParticleSystem, m_parameters.active)
    );

    edit_field<ParticleSystem, uint32_t>(
        context,
        id,
        "max particles",
        offsetof(ParticleSystem, m_parameters.max_particles)
    );

    ImGui::PopItemWidth();
}

} // namespace prt3

#endif // PRT3_PARTICLE_SYSTEM_GUI_H
