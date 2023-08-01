#ifndef PRT3_AUDIO_MANAGER_H
#define PRT3_AUDIO_MANAGER_H

#include "src/engine/component/transform.h"
#include "src/engine/scene/node.h"

#include "minivorbis.h"

#include <AL/al.h>
#ifdef __EMSCRIPTEN__
// workaround: emscripten implements this ext but does not define the macros
#ifndef AL_EXT_float32
#define AL_EXT_float32 1
#define AL_FORMAT_MONO_FLOAT32 0x10010
#define AL_FORMAT_STEREO_FLOAT32 0x10011
#endif // AL_EXT_float32
#else
#include "AL/alext.h"
#endif // __EMSCRIPTEN__
#include <AL/alc.h>

#include <vector>
#include <array>
#include <string>

struct OggVorbis_File;
struct tml_message;
struct tsf;

namespace prt3 {

class Scene;

typedef int32_t SoundSourceID;
constexpr SoundSourceID NO_SOUND_SOURCE = -1;
typedef int32_t AudioID;
constexpr AudioID NO_AUDIO = -1;
typedef int32_t MidiID;
constexpr MidiID NO_MIDI = -1;
typedef int32_t SoundFontID;
constexpr MidiID NO_SOUND_FONT = -1;

struct MidiClipState {
    double midi_ms = 0.0;
    unsigned int msg_index = 0;
    tsf * sound_font;
};

struct MidiClip {
    MidiID midi_id = NO_MIDI;
    SoundFontID sound_font_id = NO_SOUND_FONT;

    std::vector<tml_message> messages;

    MidiClipState state;

    static constexpr size_t buffer_size = 6400; // needs to last a frame
    std::array<char, buffer_size> buffer;
    size_t data_size = 0;

    bool looping = true;
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    SoundSourceID create_sound_source(NodeID node_id);
    void free_sound_source(SoundSourceID id);
    void play_sound_source(SoundSourceID id, AudioID audio_id, bool looping);
    void stop_sound_source(SoundSourceID id);
    void set_sound_source_pitch(SoundSourceID id, float pitch);
    void set_sound_source_gain(SoundSourceID id, float gain);

    /**
     * Only supports loading ogg vorbis
     */
    AudioID load_audio(char const * path);
    void free_audio(AudioID id);

    MidiID load_midi(char const * path);
    void free_midi(MidiID id);

    SoundFontID load_sound_font(char const * path);
    void free_sound_font(SoundFontID id);

    void play_midi(MidiID midi_id, SoundFontID sound_font_id);
    void stop_midi();

    MidiID get_playing_midi() const { return m_current_track.midi_id; }
    SoundFontID get_playing_sound_font() const
    { return m_current_track.sound_font_id; }

private:
    struct SoundSource {
        bool active; // allocated or freed?

        static constexpr size_t n_buffers = 2;
        ALuint buffers[n_buffers];
        ALuint source;

        NodeID node_id;
        AudioID audio_id;

        int section;

        bool playing;
        bool looping;

        float pitch;
        float gain;

        OggVorbis_File file;
        size_t pos;
    };

    struct AudioClip {
        std::vector<char> data;
    };

    std::vector<SoundSource> m_sound_sources;
    std::vector<SoundSourceID> m_free_sound_source_ids;

    std::vector<AudioClip> m_audio_clips;
    std::vector<AudioID> m_free_audio_ids;
    std::unordered_map<std::string, AudioID> m_path_to_audio;
    std::unordered_map<AudioID, std::string> m_audio_to_path;

    ov_callbacks m_ov_callbacks;

    std::vector<tml_message*> m_midis;
    std::vector<MidiID> m_free_midi_ids;
    std::unordered_map<std::string, MidiID> m_path_to_midi;
    std::unordered_map<MidiID, std::string> m_midi_to_path;

    MidiClip m_current_track;
    static constexpr size_t n_track_buffers = 4;
    ALuint m_track_buffers[n_track_buffers];
    ALuint m_track_source;

    std::vector<tsf*> m_sound_fonts;
    std::vector<SoundFontID> m_free_sound_font_ids;
    std::unordered_map<std::string, SoundFontID> m_path_to_sound_font;
    std::unordered_map<SoundFontID, std::string> m_sound_font_to_path;

    ALCcontext * m_al_context;

    unsigned int m_sample_rate;

    bool m_initialized = false;

    void update(
        Transform const & camera_transform,
        Transform const * transforms
    );

    void update_sound_sources(
        Transform const * transforms
    );

    void update_current_track();

    void fill_midi_buffer(MidiClip & clip);

    void queue_midi_stream(ALuint const source, MidiClip & clip);

    void init();

    void init_sound_source(SoundSourceID id);

    static size_t audio_read_func(
        void * ptr,
        size_t size,
        size_t nmemb,
        void * void_id
    );

    static int audio_seek_func(void * void_id, int64_t offset, int whence);
    static int audio_close_func(void * /*void_id*/) { return 0; /* ignored */ }
    static long audio_tell_func(void * void_id);

    friend class Engine;
    friend class Scene;
};

} // namespace prt3

#endif // PRT3_AUDIO_MANAGER_H
