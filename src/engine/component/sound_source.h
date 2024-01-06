#ifndef PRT3_SOUND_SOURCE_H
#define PRT3_SOUND_SOURCE_H

#include "src/engine/audio/audio_manager.h"
#include "src/engine/scene/node.h"
#include "src/util/uuid.h"

namespace prt3 {

class Scene;
template<typename T>
class ComponentStorage;

class EditorContext;

template<typename T>
void inner_show_component(EditorContext &, NodeID);
class SoundSourceComponent {
public:
    SoundSourceComponent(Scene & scene, NodeID node_id);
    SoundSourceComponent(Scene & scene, NodeID node_id, std::istream & in);

    float pitch() const { return m_pitch; }
    float gain() const { return m_gain; }

    float rolloff_factor() const { return m_rolloff_factor; }
    float reference_distance() const { return m_reference_distance; }
    float max_distance() const { return m_max_distance; }

    void set_pitch(Scene & scene, float pitch);
    void set_gain(Scene & scene, float gain);
    void set_rolloff_factor();
    void set_reference_distance();
    void set_max_distance();
    void set_max_gain();
    void set_min_gain();

    NodeID node_id() const { return m_node_id; }
    SoundSourceID sound_source_id() const { return m_sound_source_id; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Sound Source"; }
    static constexpr UUID uuid = 3011660303716332922ull;

private:
    NodeID m_node_id;
    SoundSourceID m_sound_source_id;

    float m_pitch = 1.0f;
    float m_gain = 1.0f;

    float m_rolloff_factor = 1.0f;
    float m_reference_distance = 1.0f;
    float m_max_distance = 100.0f;

    void remove(Scene & scene);

    static void update(
        Scene & scene,
        float delta_time,
        std::vector<SoundSourceComponent> & components
    );

    friend class ComponentStorage<SoundSourceComponent>;
    friend class ComponentManager;
    friend void inner_show_component<SoundSourceComponent>(EditorContext &, NodeID);
};

} // namespace prt3

#endif // PRT3_SOUND_SOURCE_H
