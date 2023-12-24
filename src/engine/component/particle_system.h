#ifndef PRT3_PARTICLE_SYSTEM_H
#define PRT3_PARTICLE_SYSTEM_H

#include "src/engine/rendering/renderer.h"
#include "src/engine/scene/node.h"
#include "src/util/serialization_util.h"
#include "src/util/uuid.h"

namespace prt3 {

class Scene;
template<typename T>
class ComponentStorage;

class EditorContext;

template<typename T>
void inner_show_component(EditorContext &, NodeID);

class ComponentManager;

class ParticleSystem {
public:
    struct Particle {
        glm::vec3 position;
        float t;
        glm::vec3 velocity;
        float lifetime;
        glm::vec4 color;
        float start_scale;
        float end_scale;
        float scale;
        float dampening;
        bool alive;
    };

    struct FloatSpread {
        float min;
        float max;

        inline float apply() const {
            return min + (max - min) * (random_float());
        }
    };

    union EmissionShape {
        constexpr EmissionShape() : point{} {}

        enum class Type : int32_t {
            point,
            circle_edge,
            sphere_surface,
            line
        };

        struct Point {} point;

        struct Circle {
            glm::vec3 direction;
            float radius;
        } circle;

        struct Sphere {
            float radius;
        } sphere;

        struct Line {
            glm::vec3 endpoint;
            glm::vec3 direction;
        } line;
    };

    struct Parameters {
        EmissionShape::Type shape_type;
        EmissionShape shape;

        FloatSpread start_scale;
        FloatSpread end_scale;

        glm::vec4 start_color;
        glm::vec4 end_color;

        float emission_rate;

        FloatSpread lifetime;
        FloatSpread velocity;

        FloatSpread dampening;

        float gravity;

        ResourceID texture_id = NO_RESOURCE;
        bool animated;
        uint32_t tex_div_w;
        uint32_t tex_div_h;

        float framerate;
        uint32_t n_frames;
        bool loop_animation;

        bool prewarm;
        bool active;

        uint32_t max_particles;
    };

    ParticleSystem(
        Scene & scene,
        NodeID node_id
    );

    ParticleSystem(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }

    ResourceID const & texture_id() const { return m_parameters.texture_id; }
    ResourceID & texture_id() { return m_parameters.texture_id; }

    inline uint32_t active_particles() const
    { return m_active_particles; }

    inline Parameters const & parameters() const {
        return m_parameters;
    }

    inline Parameters & parameters() {
        return m_parameters;
    }

    void set_parameters(Parameters const & parameters) {
        m_parameters = parameters;
    }

    static void collect_render_data(
        std::vector<ParticleSystem> const & components,
        ParticleData & data
    );

    void advance_simulation(Scene & scene, float duration, float delta_time);

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Particle System"; }
    static constexpr UUID uuid = 6487834433703112638ull;

private:
    NodeID m_node_id;

    std::vector<Particle> m_particles;

    Parameters m_parameters;

    float m_particle_timer;

    bool m_init = false;

    uint32_t m_active_particles = 0;

    void set_default_parameters();

    void remove(Scene & /*scene*/) {}

    void init(Scene & scene, float delta_time);

    void emit_particle(Scene & scene, Particle & particle);

    void update_system(Scene & scene, float delta_time);

    static void update(
        Scene & scene,
        float delta_time,
        std::vector<ParticleSystem> & components
    );

    friend class ComponentStorage<ParticleSystem>;
    friend class ComponentManager;
    friend void inner_show_component<ParticleSystem>(EditorContext &, NodeID);
};

} // namespace prt3

#endif // PRT3_PARTICLE_SYSTEM_H
