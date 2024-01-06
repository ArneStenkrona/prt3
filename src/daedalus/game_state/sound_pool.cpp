#include "sound_pool.h"

#include "src/engine/scene/scene.h"

using namespace dds;

SoundPool::SoundPool(prt3::Scene & scene) {
    prt3::AudioManager & man = scene.audio_manager();

    m_sound_id_to_audio_id[footstep_default] =
        man.load_audio("assets/audio/sfx/character/footstep_default.ogg");

    for (uint32_t i = 0; i < N_SOURCES; ++i) {
        m_sources[i] = man.create_sound_source();
        man.set_sound_source_reference_distance(m_sources[i], 1.0f);
    }

    for (uint32_t i = 0; i < N_SOURCES; ++i) {
        m_source_is_free[i] = true;
        m_free_list.insert(i);
        m_loaded_sounds[i] = no_sound;
    }
}

void SoundPool::update(prt3::Scene & scene) {
    prt3::AudioManager & man = scene.audio_manager();

    for (uint32_t i = 0; i < N_SOURCES; ++i) {
        bool playing = man.sound_source_playing(m_sources[i]);
        if (!playing && !m_source_is_free[i]) {
            m_source_is_free[i] = true;
            m_free_list.insert(i);
            m_cached_sounds[m_loaded_sounds[i]].insert(i);
        }
    }
}

bool SoundPool::play_sound(
    prt3::Scene & scene,
    SoundID sound_id,
    float pitch,
    float gain,
    float rolloff_factor,
    float max_distance,
    glm::vec3 pos
) {
    if (m_free_list.empty()) return false;

    prt3::AudioManager & man = scene.audio_manager();

    int32_t index;
    if (!m_cached_sounds[sound_id].empty()) {
        index = *m_cached_sounds[sound_id].begin();
        m_cached_sounds[sound_id].erase(index);
    } else {
        index = *m_free_list.begin();
        if (m_loaded_sounds[index] != no_sound) {
            m_cached_sounds[m_loaded_sounds[index]].erase(index);
        }
    }

    m_free_list.erase(index);

    m_source_is_free[index] = false;
    m_loaded_sounds[index] = sound_id;

    prt3::SoundSourceID source_id = m_sources[index];
    prt3::AudioID audio_id = get_audio_id(sound_id);

    man.set_sound_source_pitch(source_id, pitch);
    man.set_sound_source_gain(source_id, gain);
    man.set_sound_source_rolloff_factor(source_id, rolloff_factor);
    man.set_sound_source_max_distance(source_id, max_distance);

    man.play_sound_source(source_id, audio_id, false);
    man.set_sound_source_position(source_id, pos.x, pos.y, pos.z);

    return true;
}
