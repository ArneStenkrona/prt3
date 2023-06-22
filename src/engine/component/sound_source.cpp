#include "sound_source.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"

using namespace prt3;

SoundSourceComponent::SoundSourceComponent(
    Scene & scene,
    NodeID node_id
) : m_node_id{node_id}  {
    m_sound_source_id = scene.audio_manager().create_sound_source(m_node_id);
}

SoundSourceComponent::SoundSourceComponent(
    Scene & scene,
    NodeID node_id,
    std::istream & in
) : m_node_id{node_id} {
    AudioManager & man = scene.audio_manager();

    m_sound_source_id = man.create_sound_source(m_node_id);

    read_stream(in, m_pitch);
    read_stream(in, m_gain);

    scene.audio_manager().set_sound_source_pitch(m_sound_source_id, m_pitch);
    scene.audio_manager().set_sound_source_gain(m_sound_source_id, m_gain);
}

void SoundSourceComponent::set_pitch(Scene & scene, float pitch) {
    m_pitch = pitch;
    scene.audio_manager().set_sound_source_pitch(m_sound_source_id, m_pitch);
}

void SoundSourceComponent::set_gain(Scene & scene, float gain) {
    m_gain = gain;
    scene.audio_manager().set_sound_source_gain(m_sound_source_id, m_gain);
}

void SoundSourceComponent::serialize(
    std::ostream & out,
    Scene const & /*scene*/
) const {
    write_stream(out, m_pitch);
    write_stream(out, m_gain);
}

void SoundSourceComponent::remove(Scene & scene) {
    scene.audio_manager().free_sound_source(m_sound_source_id);
}
