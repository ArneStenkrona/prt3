#ifndef PRT3_SOUND_SOURCE_H
#define PRT3_SOUND_SOURCE_H

#include "src/engine/audio/audio_manager.h"
#include "src/engine/scene/node.h"
#include "src/util/uuid.h"

namespace prt3 {

class Scene;
template<typename T>
class ComponentStorage;

class SoundSourceComponent {
public:
    SoundSourceComponent(Scene & scene, NodeID node_id);
    SoundSourceComponent(Scene & scene, NodeID node_id, std::istream & in);

    float pitch() const { return m_pitch; }
    float gain() const { return m_gain; }

    void set_pitch(Scene & scene, float pitch);
    void set_gain(Scene & scene, float gain);

    NodeID node_id() const { return m_node_id; }
    SoundSourceID audio_source_id() const { return m_sound_source_id; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Sound Source"; }
    static constexpr UUID uuid = 3011660303716332922ull;

private:
    NodeID m_node_id;
    SoundSourceID m_sound_source_id;

    float m_pitch;
    float m_gain;

    void remove(Scene & scene);

    friend class ComponentStorage<SoundSourceComponent>;
};

} // namespace prt3

#endif // PRT3_SOUND_SOURCE_H
