#include "sound_source.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"

using namespace prt3;

SoundSourceComponent::SoundSourceComponent(
    Scene & scene,
    NodeID node_id
) : m_node_id{node_id}  {
    m_sound_source_id = scene.audio_manager().create_sound_source();
}

SoundSourceComponent::SoundSourceComponent(
    Scene & scene,
    NodeID node_id,
    std::istream & in
) : m_node_id{node_id} {
    AudioManager & man = scene.audio_manager();

    m_sound_source_id = man.create_sound_source();

    read_stream(in, m_pitch);
    read_stream(in, m_gain);
    read_stream(in, m_rolloff_factor);
    read_stream(in, m_reference_distance);
    read_stream(in, m_max_distance);

    SoundSourceID id = m_sound_source_id;
    man.set_sound_source_pitch(id, m_pitch);
    man.set_sound_source_gain(id, m_gain);
    man.set_sound_source_rolloff_factor(id, m_rolloff_factor);
    man.set_sound_source_reference_distance(id, m_reference_distance);
    man.set_sound_source_max_distance(id, m_max_distance);
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
    write_stream(out, m_rolloff_factor);
    write_stream(out, m_reference_distance);
    write_stream(out, m_max_distance);
}

void SoundSourceComponent::remove(Scene & scene) {
    scene.audio_manager().free_sound_source(m_sound_source_id);
}

void SoundSourceComponent::update(
    Scene & scene,
    float /*delta_time*/,
    std::vector<SoundSourceComponent> & components
) {
    AudioManager & man = scene.audio_manager();
    for (SoundSourceComponent & comp : components) {
        glm::vec3 pos = scene.get_cached_transform(comp.node_id()).position;
        SoundSourceID id = comp.sound_source_id();
        man.set_sound_source_position(id, pos.x, pos.y, pos.z);
    }
}

