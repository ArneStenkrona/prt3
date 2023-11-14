#include "audio_manager.h"

#include "src/util/log.h"

#include "src/engine/audio/audio_mem_util.h"

#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define O1_HEAP_INSTANCE_CAPACITY 671088640u // 10X normal amount
#else // __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define O1_HEAP_INSTANCE_CAPACITY 67108864u // 64 MiB
#endif // __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)

#define TSF_MALLOC prt3::o1hmalloc
#define TSF_REALLOC prt3::o1hrealloc
#define TSF_FREE prt3::o1hfree
#define TSF_IMPLEMENTATION
#include "tsf.h"

#define TML_MALLOC prt3::o1hmalloc
#define TML_REALLOC prt3::o1hrealloc
#define TML_FREE prt3::o1hfree
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

static AudioManager * s_am_ptr = nullptr;

AudioManager::AudioManager(BackendType backend_type) {
    m_is_dummy = backend_type == BackendType::dummy;
    if (m_is_dummy) return;
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

    if (s_am_ptr != nullptr) {
        PRT3ERROR("Expected only one instance of AudioManager.\n");
    }
    s_am_ptr = this;

    m_ov_callbacks.read_func = audio_read_func;
    m_ov_callbacks.seek_func = audio_seek_func;
    m_ov_callbacks.close_func = audio_close_func;
    m_ov_callbacks.tell_func = audio_tell_func;

    init_o1h_instance(O1_HEAP_INSTANCE_CAPACITY);
}

AudioManager::~AudioManager() {
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

    free_o1h_instance();

    if (!m_initialized) {
        return;
    }

    alDeleteSources(1, &m_track_source);
    CHECK_AL_ERRORS();
    alDeleteBuffers(n_track_buffers, &m_track_buffers[0]);
    CHECK_AL_ERRORS();

    for (SoundSource & source : m_sound_sources) {
        if (!source.active) continue;

        alDeleteSources(1, &source.source);
        CHECK_AL_ERRORS();
        alDeleteBuffers(SoundSource::n_buffers, &source.buffers[0]);
        CHECK_AL_ERRORS();
        if (source.audio_id != NO_AUDIO) {
            ov_clear(&source.file);
        }
    }

    ALCdevice * device = alcGetContextsDevice(m_al_context);
    alcDestroyContext(m_al_context);
    CHECK_AL_ERRORS();
    alcCloseDevice(device);
    CHECK_AL_ERRORS();
}

SoundSourceID AudioManager::create_sound_source(NodeID node_id) {
    MidiID id;
    if (!m_free_sound_source_ids.empty()) {
        id = m_free_sound_source_ids.back();
        m_free_sound_source_ids.pop_back();
    } else {
        id = m_sound_sources.size();
        m_sound_sources.push_back({});
    }

    SoundSource & source = m_sound_sources[id];
    source.active = true;
    source.section = 0;
    source.node_id = node_id;
    source.audio_id = NO_AUDIO;
    source.playing = false;
    source.looping = false;
    source.pitch = 1.0f;
    source.gain = 1.0f;

    if (m_initialized) {
        init_sound_source(id);
    }

    return id;
}

void AudioManager::free_sound_source(SoundSourceID id) {
    SoundSource & source = m_sound_sources[id];
    source.active = false;
    alDeleteSources(1, &source.source);
    CHECK_AL_ERRORS();
    alDeleteBuffers(SoundSource::n_buffers, &source.buffers[0]);
    CHECK_AL_ERRORS();
    if (source.audio_id != NO_AUDIO) {
        ov_clear(&source.file);
    }
}

void AudioManager::play_sound_source(
    SoundSourceID id,
    AudioID audio_id,
    bool looping
) {
    SoundSource & source = m_sound_sources[id];

    if (source.audio_id != NO_AUDIO) {
        ov_clear(&source.file);
    }

    source.audio_id = audio_id;
    source.playing = true;
    source.looping = looping;

    source.section = 0;
    source.pos = 0;

    void * void_id = reinterpret_cast<void*>(static_cast<intptr_t>(id+1));
    if(ov_open_callbacks(void_id, &source.file, NULL, 0, m_ov_callbacks) != 0) {
        PRT3ERROR("Invalid Ogg file '%s'.", m_audio_to_path.at(audio_id).c_str());
    }
}

void AudioManager::stop_sound_source(SoundSourceID id) {
    m_sound_sources[id].playing = false;
}

void AudioManager::set_sound_source_pitch(SoundSourceID id, float pitch) {
    m_sound_sources[id].pitch = pitch;
    if (m_initialized) {
        alSourcef(m_sound_sources[id].source, AL_PITCH, pitch);
    }
}

void AudioManager::set_sound_source_gain(SoundSourceID id, float gain) {
    m_sound_sources[id].gain = gain;
    if (m_initialized) {
        alSourcef(m_sound_sources[id].source, AL_GAIN, gain);
    }
}

size_t AudioManager::audio_read_func(
    void * ptr,
    size_t size,
    size_t nmemb,
    void * void_id
) {
    SoundSourceID id =
        static_cast<SoundSourceID>(reinterpret_cast<intptr_t>(void_id)) - 1;
    SoundSource & source = s_am_ptr->m_sound_sources[id];
    AudioClip & clip = s_am_ptr->m_audio_clips[source.audio_id];
    size_t n_bytes = std::min(clip.data.size() - source.pos, size * nmemb);
    memcpy(ptr, &clip.data[source.pos], n_bytes);
    source.pos += n_bytes;
    return n_bytes;
}

int AudioManager::audio_seek_func(void * void_id, int64_t offset, int whence) {
    SoundSourceID id =
        static_cast<SoundSourceID>(reinterpret_cast<intptr_t>(void_id)) - 1;
    SoundSource & source = s_am_ptr->m_sound_sources[id];
    AudioClip & clip = s_am_ptr->m_audio_clips[source.audio_id];
    int64_t new_pos;

    switch (whence) {
        case SEEK_CUR: {
            new_pos = source.pos + offset;
            break;
        }
        case SEEK_END: {
            new_pos = static_cast<int64_t>(clip.data.size()) + offset;
            break;
        }
        case SEEK_SET:
        default: {
            new_pos = offset;
        }
    }

    if (new_pos < 0 || new_pos > static_cast<int64_t>(clip.data.size())) {
        return -1;
    }

    source.pos = static_cast<size_t>(new_pos);
    return 0;
}

long AudioManager::audio_tell_func(void * void_id) {
    SoundSourceID id =
        static_cast<SoundSourceID>(reinterpret_cast<intptr_t>(void_id)) - 1;
    SoundSource & source = s_am_ptr->m_sound_sources[id];
    return static_cast<long>(source.pos);
}

AudioID AudioManager::load_audio(char const * path) {
    auto it = m_path_to_audio.find(std::string(path));
    if (it != m_path_to_audio.end()) {
        return it->second;
    }

    FILE * fp = fopen(path, "rb");
    if(!fp) {
        PRT3ERROR("Failed to open Ogg file '%s'.", path);
        return NO_AUDIO;
    }

    MidiID id;
    if (!m_free_audio_ids.empty()) {
        id = m_free_audio_ids.back();
        m_free_audio_ids.pop_back();
    } else {
        id = m_audio_clips.size();
        m_audio_clips.push_back({});
    }

    fseek(fp, 0l, SEEK_END);
    size_t n_bytes = ftell(fp);
    fseek(fp, 0l, SEEK_SET);

    AudioClip & clip = m_audio_clips[id];
    clip.data.resize(n_bytes);

    fread(clip.data.data(), 1, n_bytes, fp);

    fclose(fp);

    m_audio_to_path[id] = path;
    m_path_to_audio[path] = id;
    return id;
}

void AudioManager::free_audio(AudioID id) {
    m_audio_clips[id].data.clear();
    m_audio_clips[id].data.shrink_to_fit();
    m_free_audio_ids.push_back(id);
    m_path_to_audio.erase(m_audio_to_path.at(id));
    m_audio_to_path.erase(id);
}

void AudioManager::update(
    Transform const & camera_transform,
    Transform const * transforms
) {
    if (!m_initialized) {
        init();
        return;
    }

    glm::vec3 const & cam_pos = camera_transform.position;
    glm::vec3 const & cam_front = camera_transform.get_front();
    alListener3f(AL_POSITION, cam_pos.x, cam_pos.y, cam_pos.z);
    float orientation[6] =
        {cam_front.x, cam_front.y, cam_front.z, 0.0f, 1.0f, 0.0f};
    alListenerfv(AL_ORIENTATION, orientation);

    update_sound_sources(transforms);
    update_current_track();
}

void AudioManager::update_sound_sources(Transform const * transforms) {
    for (SoundSource & source : m_sound_sources) {
        if (!source.active) continue;
        if (!source.playing) continue;
        if (source.audio_id == NO_AUDIO) continue;

        glm::vec3 const & pos = transforms[source.node_id].position;
        alSource3f(source.source, AL_POSITION, pos.x, pos.y, pos.z);

        OggVorbis_File & audio = source.file;

        ALint n_processed = 0;
        alGetSourcei(source.source, AL_BUFFERS_PROCESSED, &n_processed);
        CHECK_AL_ERRORS();

        static constexpr int buf_length = 4096;
        float buf[buf_length * 2]; // * 2 due to possibly stereo

        vorbis_info* info = ov_info(&audio, -1);

        while (n_processed > 0) {
            --n_processed;

            size_t read_bytes = 0;
            size_t read_samples = 0;
            bool reading = true;
            while (reading) {
                float ** buf_ptr = nullptr;

                int64_t total = ov_pcm_total(&audio, -1);
                int64_t pos = ov_pcm_tell(&audio);

                if (pos >= total) {
                    if (source.looping) {
                        source.section = 0;
                        ov_time_seek(&audio, 0.0f);
                    } else {
                        source.playing = false;
                    }
                }

                pos = ov_pcm_tell(&audio);
                int len = std::min(512, static_cast<int>(total - pos));

                if (len <= 0) {
                    break;
                }

                long n_samples =
                    ov_read_float(
                        &audio,
                        &buf_ptr,
                        len,
                        &source.section
                    );

                if (n_samples < 0) {
                    break;
                }

                auto n_bytes = n_samples * 4 * info->channels;
                if (info->channels == 1) {
                    memcpy(&buf[0] + read_samples, &buf_ptr[0][0], n_bytes);
                } else {
                    unsigned n_samples_ui = static_cast<unsigned>(n_samples);
                    for (unsigned i = 0; i < n_samples_ui; ++i) {
                        buf[2*(i + read_samples)] = buf_ptr[0][i];
                        buf[2*(i + read_samples)+1] = buf_ptr[1][i];
                    }
                }
                read_bytes += n_bytes;
                read_samples += n_samples;
                reading = read_samples < buf_length && source.playing;
            }

            if (read_samples <= 0) {
                PRT3ERROR("Failed to read ogg audio.\n");
                break;
            }

            ALuint buffer;
            alSourceUnqueueBuffers(source.source, 1, &buffer);
            CHECK_AL_ERRORS();

            ALint format = info->channels == 1 ?
                AL_FORMAT_MONO_FLOAT32 :
                AL_FORMAT_STEREO_FLOAT32;

            alBufferData(
                buffer,
                format,
                &buf[0],
                read_bytes,
                info->rate
            );
            CHECK_AL_ERRORS();

            alSourceQueueBuffers(source.source, 1, &buffer);
            CHECK_AL_ERRORS();
        }

        ALint state;
        alGetSourcei(source.source, AL_SOURCE_STATE, &state);
        CHECK_AL_ERRORS();

        if (state != AL_PLAYING) {
            alSourcePlay(source.source);
            CHECK_AL_ERRORS();
        }
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

    static constexpr unsigned int block_size = 64;
    for (unsigned int sample_block = block_size;
         sample_count;
         sample_count -= sample_block,
            stream += (sample_block * (2 * sizeof(float)))) {
        if (sample_block > sample_count) sample_block = sample_count;

        double ms_per_block =
            1000.0 / static_cast<double>(m_sample_rate);

        tml_message * msg = &clip.messages[state.msg_index];

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

    m_midi_to_path[id] = path;
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

    m_sound_font_to_path[id] = path;
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
    if (m_current_track.state.sound_font) {
        tsf_reset(m_current_track.state.sound_font);
    }

    m_current_track = {};
}

void AudioManager::init() {
    if (m_is_dummy) return;
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
    alSourcef(m_track_source, AL_PITCH, 1.0f);
    CHECK_AL_ERRORS();
    alSourcef(m_track_source, AL_GAIN, 1.0f);
    CHECK_AL_ERRORS();
    alSource3f(m_track_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    CHECK_AL_ERRORS();
    alSource3f(m_track_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    CHECK_AL_ERRORS();
    alSourcei(m_track_source, AL_LOOPING, AL_FALSE);
    CHECK_AL_ERRORS();
    alSourcei(m_track_source, AL_ROLLOFF_FACTOR, 0.0f);
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

    for (SoundSourceID id = 0;
         id < static_cast<SoundSourceID>(m_sound_sources.size());
         ++id
    ) {
        if (m_sound_sources[id].active) {
            init_sound_source(id);
        }
    }

    m_initialized = true;
}

void AudioManager::init_sound_source(SoundSourceID id) {
    SoundSource & source = m_sound_sources[id];

    alGenSources(1, &source.source);
    alSourcef(source.source, AL_PITCH, source.pitch);
    alSourcef(source.source, AL_GAIN, source.gain);
    alSource3f(source.source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source.source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source.source, AL_LOOPING, AL_FALSE);

    alGenBuffers(SoundSource::n_buffers, &source.buffers[0]);

    uint32_t zero = 0x0;
    for (size_t i = 0; i < SoundSource::n_buffers; ++i) {
        alBufferData(
            source.buffers[i],
            AL_FORMAT_STEREO_FLOAT32,
            &zero,
            1,
            m_sample_rate
        );
    }

    alSourceQueueBuffers(
        source.source,
        SoundSource::n_buffers,
        &source.buffers[0]
    );
}
