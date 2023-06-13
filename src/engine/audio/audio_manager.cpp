#include "audio_manager.h"

#include "src/util/log.h"

#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"

using namespace prt3;

// #define CHECK_AL_ERRORS() check_al_errors(__FILE__, __LINE__)
#define CHECK_AL_ERRORS()

bool check_al_errors(char const * filename, int line)
{
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        char const * msg;
        switch(error)
        {
        case AL_INVALID_NAME:
            msg = "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
            break;
        case AL_INVALID_ENUM:
            msg = "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
            break;
        case AL_INVALID_VALUE:
            msg = "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case AL_INVALID_OPERATION:
            msg ="AL_INVALID_OPERATION: the requested operation is not valid";
            break;
        case AL_OUT_OF_MEMORY:
            msg = "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
            break;
        default:
            msg = "UNKNOWN AL ERROR: ";
        }
        PRT3ERROR("***ERROR*** (%s:%d)\n    %s\n", filename, line, msg);
        return false;
    }
    return true;
}

AudioManager::AudioManager() {
#ifdef __EMSCRIPTEN__
    m_sample_rate = EM_ASM_INT({
        var audio_context = window.audio_context || window.webkitaudio_context;
        var ctx = new AudioContext();
        var sr = ctx.sampleRate;
        ctx.close();
        return sr;
    });

#else //__EMSCRIPTEN__
    m_sample_rate = 44100;
#endif //__EMSCRIPTEN__
}

AudioManager::~AudioManager() {
    if (!m_initialized) {
        return;
    }

    alDeleteSources(1, &m_track_source);
    CHECK_AL_ERRORS();
    alDeleteBuffers(n_track_buffers, &m_track_buffers[0]);
    CHECK_AL_ERRORS();

    ALCdevice * device = alcGetContextsDevice(m_al_context);
    alcDestroyContext(m_al_context);
    CHECK_AL_ERRORS();
    alcCloseDevice(device);
    CHECK_AL_ERRORS();

    for (tml_message * msg : m_midis) {
        if (msg != nullptr) {
            tml_free(msg);
        }
    }

    for (tsf * sf : m_sound_fonts) {
        if (sf != nullptr) {
            tsf_close(sf);
        }
    }
}

void AudioManager::update() {
    if (!m_initialized) {
        init();
    } else {
        update_current_track();
    }
}

void AudioManager::update_current_track() {
    if (m_current_track.messages.empty()) return;

    queue_midi_stream(m_track_source, m_current_track);
}

void AudioManager::fill_midi_buffer(
    MidiClip & clip
) {
    clip.data_size = 0;
    if (clip.messages.empty()) {
        return;
    }

    MidiClipState & state = clip.state;
    //Number of samples to process
    unsigned int sample_count =
        (clip.buffer_size / (2 * sizeof(float))); // 2 output channels

    char * stream = clip.buffer.data();

    tml_message * msg = &clip.messages[state.msg_index];

    static constexpr unsigned int block_size = 16;
    for (unsigned int sample_block = block_size;
         sample_count;
         sample_count -= sample_block,
            stream += (sample_block * (2 * sizeof(float)))) {
        if (sample_block > sample_count) sample_block = sample_count;

        double ms_per_block =
            1000.0 / static_cast<double>(m_sample_rate);

        state.midi_ms += sample_block * ms_per_block;
        while (state.msg_index < clip.messages.size() &&
               state.midi_ms >= msg->time) {
            switch (msg->type) {
                case TML_PROGRAM_CHANGE:
                    // channel program (preset) change
                    // (special handling for 10th MIDI channel with drums)
                    tsf_channel_set_presetnumber(
                        state.sound_font,
                        msg->channel,
                        msg->program,
                        (msg->channel == 9)
                    );
                    break;
                case TML_NOTE_ON:
                    // play a note
                    tsf_channel_note_on(
                        state.sound_font,
                        msg->channel,
                        msg->key,
                        msg->velocity / 127.0f
                    );
                    break;
                case TML_NOTE_OFF:
                    // stop a note
                    tsf_channel_note_off(
                        state.sound_font,
                        msg->channel,
                        msg->key
                    );
                    break;
                case TML_PITCH_BEND:
                    // pitch wheel modification
                    tsf_channel_set_pitchwheel(
                        state.sound_font,
                        msg->channel,
                        msg->pitch_bend
                    );
                    break;
                case TML_CONTROL_CHANGE:
                    // MIDI controller messages
                    tsf_channel_midi_control(
                        state.sound_font,
                        msg->channel,
                        msg->control,
                        msg->control_value
                    );
                    break;
            }
            state.msg_index = state.msg_index + 1;
            msg = &clip.messages[state.msg_index];
        }

        if (state.msg_index >= clip.messages.size() && clip.looping) {
            state.msg_index = 0;
            state.midi_ms = 0.0;
        }

        // Render the block of audio samples in float format
        tsf_render_float(state.sound_font, (float*)stream, sample_block, 0);
        clip.data_size += sample_block * (2 * sizeof(float));
    }
}

void AudioManager::queue_midi_stream(
    ALuint const source,
    MidiClip & clip
) {
    ALint n_processed = 0;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &n_processed);
    CHECK_AL_ERRORS();

    while (n_processed > 0) {
        --n_processed;

        fill_midi_buffer(clip);

        ALuint buffer;
        alSourceUnqueueBuffers(source, 1, &buffer);
        CHECK_AL_ERRORS();

        alBufferData(
            buffer,
            AL_FORMAT_STEREO_FLOAT32,
            clip.buffer.data(),
            clip.data_size,
            m_sample_rate
        );

        CHECK_AL_ERRORS();
        alSourceQueueBuffers(source, 1, &buffer);
        CHECK_AL_ERRORS();
    }

    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    CHECK_AL_ERRORS();

    if (state != AL_PLAYING) {
        alSourcePlay(source);
        CHECK_AL_ERRORS();
    }
}

MidiID AudioManager::load_midi(char const * path) {
    auto it = m_path_to_midi.find(std::string(path));
    if (it != m_path_to_midi.end()) {
        return it->second;
    }

    MidiID id;
    if (!m_free_midi_ids.empty()) {
        id = m_free_midi_ids.back();
        m_free_midi_ids.pop_back();
    } else {
        id = m_midis.size();
        m_midis.push_back({});
    }

    m_midis[id] = tml_load_filename(path);
    if (!m_midis[id])
    {
        PRT3ERROR("Could not load MIDI file: %s\n", path);
        id = NO_MIDI;
    }

    m_path_to_midi[path] = id;

    return id;
}

void AudioManager::free_midi(MidiID id) {
    tml_free(m_midis[id]);
    m_midis[id] = nullptr;
    m_free_midi_ids.push_back(id);
    m_path_to_midi.erase(m_midi_to_path.at(id));
    m_midi_to_path.erase(id);
}

SoundFontID AudioManager::load_sound_font(char const * path) {
    auto it = m_path_to_sound_font.find(std::string(path));
    if (it != m_path_to_sound_font.end()) {
        return it->second;
    }

    SoundFontID id;
    if (!m_free_sound_font_ids.empty()) {
        id = m_free_sound_font_ids.back();
        m_free_sound_font_ids.pop_back();
    } else {
        id = m_sound_fonts.size();
        m_sound_fonts.push_back({});
    }

    m_sound_fonts[id] = tsf_load_filename(path);
    if (!m_sound_fonts[id])
    {
        PRT3ERROR("Could not load sound font: %s\n", path);
        id = NO_SOUND_FONT;
    }

    tsf_channel_set_bank_preset(m_sound_fonts[id], 9, 128, 0);

    // Set the SoundFont rendering output mode
    tsf_set_output(
        m_sound_fonts[id],
        TSF_STEREO_INTERLEAVED,
        m_sample_rate,
        0.0f
    );

    m_path_to_sound_font[path] = id;

    return id;
}

void AudioManager::free_sound_font(SoundFontID id) {
    tsf_close(m_sound_fonts[id]);
    m_sound_fonts[id] = nullptr;
    m_free_sound_font_ids.push_back(id);
    m_path_to_sound_font.erase(m_sound_font_to_path.at(id));
    m_sound_font_to_path.erase(id);
}

void AudioManager::play_midi(MidiID midi_id, SoundFontID sound_font_id) {
    if (m_current_track.midi_id != NO_MIDI) {
        stop_midi();
    }

    tml_message *curr = m_midis[midi_id];

    while (curr) {
        m_current_track.messages.emplace_back(*curr);
        curr = curr->next;
    }

    m_current_track.midi_id = midi_id;
    m_current_track.state.sound_font = m_sound_fonts[sound_font_id];
    m_current_track.state.msg_index = 0;
    m_current_track.state.midi_ms = 0.0;

    m_current_track.buffer = {};
}

void AudioManager::stop_midi() {
    tsf_reset(m_current_track.state.sound_font);
    m_current_track = {};
}

void AudioManager::init() {
#ifdef __EMSCRIPTEN__
    int running = EM_ASM_INT({
        var audio_context = window.audio_context || window.webkitaudio_context;
        var ctx = new AudioContext();
        ctx.resume();
        return ctx.state === "suspended" ? 0 : 1;
    });

    if (running == 0) return;
#endif // __EMSCRIPTEN__
    CHECK_AL_ERRORS();

    ALCdevice * device = alcOpenDevice(NULL);
    CHECK_AL_ERRORS();

    if (!device)
    {
        PRT3ERROR("Failed to AL device.\n");
    }

    m_al_context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(m_al_context)) {
        PRT3ERROR("Failed to make AL context current.\n");
    }
    CHECK_AL_ERRORS();

    alGenBuffers(n_track_buffers, &m_track_buffers[0]);
    CHECK_AL_ERRORS();

    alGenSources(1, &m_track_source);
    CHECK_AL_ERRORS();
    alSourcef(m_track_source, AL_PITCH, 1);
    CHECK_AL_ERRORS();
    alSourcef(m_track_source, AL_GAIN, 1.0f);
    CHECK_AL_ERRORS();
    alSource3f(m_track_source, AL_POSITION, 0, 0, 0);
    CHECK_AL_ERRORS();
    alSource3f(m_track_source, AL_VELOCITY, 0, 0, 0);
    CHECK_AL_ERRORS();
    alSourcei(m_track_source, AL_LOOPING, AL_FALSE);
    CHECK_AL_ERRORS();

    for (size_t i = 0; i < n_track_buffers; ++i) {
        alBufferData(
            m_track_buffers[i],
            AL_FORMAT_STEREO_FLOAT32,
            m_current_track.buffer.data(),
            1,
            m_sample_rate
        );
    }

    alSourceQueueBuffers(m_track_source, n_track_buffers, &m_track_buffers[0]);

    m_initialized = true;
}
