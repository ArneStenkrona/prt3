#ifndef DDS_SOUND_POOL_H
#define DDS_SOUND_POOL_H

#include "src/daedalus/game_state/id.h"

#include "src/engine/audio/audio_manager.h"

#include <array>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace prt3 {
    class Scene;
} // namespace prt3

namespace dds {

class SoundPool {
public:
    enum SoundID : DDSID {
        footstep_default,
        no_sound,
        n_sound_ids = no_sound
    };

    SoundPool(prt3::Scene & scene);

    prt3::AudioID get_audio_id(SoundID id) const
    { return m_sound_id_to_audio_id[id]; }

    void update(prt3::Scene & scene);

    bool play_sound(
        prt3::Scene & scene,
        SoundID sound_id,
        float pitch,
        float gain,
        float rolloff_factor,
        float max_distance,
        glm::vec3 position
    );

private:
    static constexpr uint32_t N_SOURCES = 256;
    std::array<prt3::SoundSourceID, N_SOURCES> m_sources;
    std::array<bool, N_SOURCES> m_source_is_free;
    std::array<SoundID, N_SOURCES> m_loaded_sounds;
    std::unordered_set<uint32_t> m_free_list;

    std::array<std::unordered_set<uint32_t>, n_sound_ids> m_cached_sounds;

    std::array<prt3::AudioID, N_SOURCES> m_sound_id_to_audio_id;
};

} // namespace dds

#endif // DDS_SOUND_POOL_H
