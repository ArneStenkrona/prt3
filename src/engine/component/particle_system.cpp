#include "particle_system.h"

#include "src/engine/component/component_utility.h"
#include "src/engine/scene/scene.h"
#include "src/util/random.h"

using namespace prt3;

ParticleSystem::ParticleSystem(Scene & /*scene*/, NodeID node_id)
 : m_node_id{node_id} {
    set_default_parameters();
 }

ParticleSystem::ParticleSystem(
    Scene & scene,
    NodeID node_id,
    std::istream & in
)
 : m_node_id{node_id} {
    read_stream(in, m_parameters.shape_type);
    read_stream(in, m_parameters.shape);
    read_stream(in, m_parameters.start_scale);
    read_stream(in, m_parameters.end_scale);
    read_stream(in, m_parameters.start_color);
    read_stream(in, m_parameters.end_color);
    read_stream(in, m_parameters.start_emissive);
    read_stream(in, m_parameters.end_emissive);
    read_stream(in, m_parameters.emission_rate);
    read_stream(in, m_parameters.lifetime);
    read_stream(in, m_parameters.velocity);
    read_stream(in, m_parameters.dampening);
    read_stream(in, m_parameters.gravity);
    m_parameters.texture_id = deserialize_texture(in, scene);
    read_stream(in, m_parameters.animated);
    read_stream(in, m_parameters.tex_div_w);
    read_stream(in, m_parameters.tex_div_h);
    read_stream(in, m_parameters.framerate);
    read_stream(in, m_parameters.n_frames);
    read_stream(in, m_parameters.loop_animation);
    read_stream(in, m_parameters.prewarm);
    read_stream(in, m_parameters.active);
    read_stream(in, m_parameters.max_particles);
}

void ParticleSystem::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    write_stream(out, m_parameters.shape_type);
    write_stream(out, m_parameters.shape);
    write_stream(out, m_parameters.start_scale);
    write_stream(out, m_parameters.end_scale);
    write_stream(out, m_parameters.start_color);
    write_stream(out, m_parameters.end_color);
    write_stream(out, m_parameters.start_emissive);
    write_stream(out, m_parameters.end_emissive);
    write_stream(out, m_parameters.emission_rate);
    write_stream(out, m_parameters.lifetime);
    write_stream(out, m_parameters.velocity);
    write_stream(out, m_parameters.dampening);
    write_stream(out, m_parameters.gravity);
    serialize_texture(out, scene, m_parameters.texture_id);
    write_stream(out, m_parameters.animated);
    write_stream(out, m_parameters.tex_div_w);
    write_stream(out, m_parameters.tex_div_h);
    write_stream(out, m_parameters.framerate);
    write_stream(out, m_parameters.n_frames);
    write_stream(out, m_parameters.loop_animation);
    write_stream(out, m_parameters.prewarm);
    write_stream(out, m_parameters.active);
    write_stream(out, m_parameters.max_particles);
}

void ParticleSystem::set_default_parameters() {
    Parameters & params = m_parameters;
    params.shape_type = ParticleSystem::EmissionShape::Type::circle_edge;
    params.shape.circle.direction = glm::vec3{0.0f, 1.0f, 0.0f};
    params.shape.circle.radius = 1.0f;

    params.start_scale.min = 1.0f;
    params.start_scale.max = 1.0f;
    params.end_scale.min = 0.0f;
    params.end_scale.max = 0.0f;
    params.start_color = glm::vec4{1.0f};
    params.end_color = glm::vec4{1.0f};
    params.emission_rate = 5.0f;
    params.lifetime.min = 5.0f;
    params.lifetime.max = 5.0f;
    params.velocity.min = 1.0f;
    params.velocity.max = 1.0f;
    params.dampening.min = 0.0f;
    params.dampening.max = 0.0f;
    params.gravity = -1.0f;
    params.texture_id = NO_RESOURCE;
    params.animated = false;
    params.tex_div_w = 1.0f;
    params.tex_div_h = 1.0f;
    params.framerate = 30.0f;
    params.n_frames = 0;
    params.loop_animation = true;
    params.prewarm = false;
    params.active = true;
    params.max_particles = 64;
}

void ParticleSystem::advance_simulation(
    Scene & scene,
    float duration,
    float delta_time
) {
    for (float t = 0.0f; t < duration; t += delta_time) {
        update_system(scene, delta_time);
    }
}


void ParticleSystem::init(Scene & scene, float delta_time) {
    m_particles.resize(m_parameters.max_particles);
    for (Particle & particle : m_particles) {
        particle.alive = false;
    }

    m_particle_timer = m_parameters.emission_rate == 0.0f ?
        0.0f : 1.0f / m_parameters.emission_rate;

    switch (m_parameters.shape_type) {
        case EmissionShape::Type::circle_edge: {
            EmissionShape::Circle & circle = m_parameters.shape.circle;
            if (circle.direction == glm::vec3{0.0f}) {
                circle.direction = glm::vec3{0.0f, 1.0f, 0.0f};
            } else {
                circle.direction = glm::normalize(circle.direction);
            }
            break;
        }
       default: {
            break;
        }
    }

    m_active_particles = 0;

    if (m_parameters.prewarm) {
        advance_simulation(scene, m_parameters.lifetime.max, delta_time);
    }

    m_init = true;
}

void ParticleSystem::emit_particle(Scene & scene, Particle & particle) {
    Parameters const & params = m_parameters;

    Transform tform = scene.get_node(node_id()).get_global_transform(scene);

    glm::vec3 pos;
    glm::vec3 dir;
    switch (params.shape_type) {
        case EmissionShape::Type::point: {
            pos = glm::vec3{0.0f};
            dir = random_direction();
            break;
        }
        case EmissionShape::Type::circle_edge: {
            EmissionShape::Circle const & circle = params.shape.circle;
            glm::vec3 v = glm::vec3{1.0f, 0.0f, 0.0f};
            if (glm::dot(v, circle.direction) > 0.9f) {
                v = glm::vec3{0.0f, 1.0f, 0.0f};
            }
            glm::vec3 a = glm::normalize(glm::cross(v, circle.direction));
            glm::vec3 b = glm::normalize(glm::cross(a, circle.direction));

            float theta = 2.0f * glm::pi<float>() * random_float();

            pos = circle.radius * (glm::cos(theta) * a + glm::sin(theta) * b);

            dir = glm::mat3{tform.to_matrix()} * circle.direction;
            break;
        }
        case EmissionShape::Type::sphere_surface: {
            EmissionShape::Sphere const & sphere = params.shape.sphere;
            dir = random_direction();
            pos = tform.scale * dir * sphere.radius;
            break;
        }
        case EmissionShape::Type::line: {
            EmissionShape::Line const & line = params.shape.line;
            Transform scale_rot = tform;
            scale_rot.position = glm::vec3{0.0f};
            glm::vec3 endpoint = glm::vec3{
                scale_rot.to_matrix() * glm::vec4{line.endpoint, 1.0f}
            };
            pos = glm::mix(glm::vec3{0.0f}, endpoint, random_float());
            break;
        }
    }

    particle.position = tform.position + pos;

    particle.t = 0.0f;
    particle.velocity = dir * params.velocity.apply();
    particle.lifetime = params.lifetime.apply();
    particle.color = params.start_color;
    particle.emissive = params.start_emissive;
    particle.start_scale = params.start_scale.apply();
    particle.end_scale = params.end_scale.apply();
    particle.scale = particle.start_scale;
    particle.dampening = params.dampening.apply();
    particle.alive = true;

    ++m_active_particles;
}

static inline uint32_t get_frame(
    float t,
    float framerate,
    uint32_t n_frames,
    bool loop
) {
    uint32_t frame = static_cast<uint32_t>(t * framerate);
    if (loop) {
        frame = frame % n_frames;
    } else {
        frame = frame > n_frames ? n_frames : frame;
    }

    return frame;
}

void ParticleSystem::update_system(Scene & scene, float delta_time) {
    Parameters const & params = m_parameters;
    glm::vec3 gf =
        params.gravity * glm::vec3{0.0f, -1.0f, 0.0f} * delta_time;

    unsigned emit_count = 0;

    if (params.max_particles != m_particles.size()) {
        m_particles.resize(params.max_particles);
    }

    if (params.emission_rate * m_particle_timer >= 1.0f) {
        /* emit particle */
        emit_count =
            static_cast<unsigned>(params.emission_rate * m_particle_timer);
        m_particle_timer -= 1.0f / params.emission_rate;
    }

    for (Particle & particle : m_particles) {
        if (particle.alive && particle.t >= particle.lifetime) {
            particle.alive = false;
            --m_active_particles;
        }

        if (!particle.alive && emit_count > 0) {
            --emit_count;
            emit_particle(scene, particle);
            continue;
        }

        particle.position += particle.velocity * delta_time;

        particle.velocity =
            particle.velocity / (1.0f + (particle.dampening * delta_time));
        particle.velocity += gf;

        float interp = particle.t / particle.lifetime;
        particle.scale =
            glm::mix(particle.start_scale, particle.end_scale, interp);
        particle.color =
            glm::mix(params.start_color, params.end_color, interp);
        particle.emissive =
            glm::mix(params.start_emissive, params.end_emissive, interp);

        particle.t += delta_time;
    }

    m_particle_timer += delta_time;
}

void ParticleSystem::update(
    Scene & scene,
    float delta_time,
    std::vector<ParticleSystem> & components
) {
    for (ParticleSystem & ps : components) {
        if (!ps.m_parameters.active) ps.m_init = false;
    }

    for (ParticleSystem & ps : components) {
        if (ps.m_parameters.active && !ps.m_init) {
            ps.init(scene, delta_time);
        }
    }

    for (ParticleSystem & ps : components) {
        if (!ps.m_parameters.active) continue;
        ps.update_system(scene, delta_time);
    }
}

void ParticleSystem::collect_render_data(
        std::vector<ParticleSystem> const & components,
        ParticleData & data
) {
    for (ParticleSystem const & ps : components) {
        if (!ps.m_parameters.active) continue;

        ParticleSystem::Parameters const & params = ps.m_parameters;

        uint32_t n_particles = 0;
        for (Particle const & particle : ps.m_particles) {
            if (particle.alive) ++n_particles;
        }

        if (n_particles == 0) continue;

        ParticleData::TextureRange range;
        range.start_index = data.attributes.size();
        range.count = n_particles;
        range.texture = params.texture_id;
        float inv_div_w = params.animated ? 1.0f / params.tex_div_w : 1.0f;
        float inv_div_h = params.animated ? 1.0f / params.tex_div_h : 1.0f;
        range.inv_div.x = inv_div_w;
        range.inv_div.y = inv_div_h;

        data.textures.push_back(range);
        uint32_t i = data.attributes.size();
        data.attributes.resize(i + n_particles);

        for (Particle const & particle : ps.m_particles) {
            if (!particle.alive) continue;
            ParticleAttributes & attr = data.attributes[i];
            attr.pos_size = glm::vec4{particle.position, particle.scale};
            attr.color[0] = static_cast<uint8_t>(255.0f * particle.color[0]);
            attr.color[1] = static_cast<uint8_t>(255.0f * particle.color[1]);
            attr.color[2] = static_cast<uint8_t>(255.0f * particle.color[2]);
            attr.color[3] = static_cast<uint8_t>(255.0f * particle.color[3]);
            attr.emissive = particle.emissive;

            if (ps.m_parameters.animated) {
                uint32_t frame = get_frame(
                    particle.t,
                    params.framerate,
                    params.n_frames,
                    ps.m_parameters.loop_animation
                );
                uint32_t ind_x = frame % ps.m_parameters.tex_div_w;
                uint32_t ind_y = frame * inv_div_h;
                float x = static_cast<float>(ind_x) + 0.5f;
                float y = static_cast<float>(ind_y) + 0.5f;

                attr.base_uv = glm::vec2{x * inv_div_w, y * inv_div_h};
            } else {
                attr.base_uv = glm::vec2{0.5f, 0.5f};
            }

            ++i;
        }
    }
}
