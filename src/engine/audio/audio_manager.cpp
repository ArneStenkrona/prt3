#include "audio_manager.h"

#include "src/util/log.h"

#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif //__EMSCRIPTEN__

using namespace prt3;

AudioManager::AudioManager() {}

AudioManager::~AudioManager() {
    if (!m_initialized) {
        return;
    }

    alDeleteSources(1, &m_track_source);
    alDeleteBuffers(n_track_buffers, &m_track_buffers[0]);

    ALCdevice * device = alcGetContextsDevice(m_al_context);
    alcDestroyContext(m_al_context);
    alcCloseDevice(device);

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
    if (m_current_track.origin == nullptr) return;

    fill_midi_buffer(m_current_track);
    queue_midi_stream(m_track_source, m_current_track);
}

void AudioManager::fill_midi_buffer(
    MidiClip & clip
) {
    if (!clip.buffer_consumed) {
        return;
    }

    clip.buffer_consumed = false;

    MidiClipState & state = clip.state;
	//Number of samples to process
	unsigned int sample_block, sample_count =
        (clip.buffer_size / (2 * sizeof(float))); //2 output channels

    char * stream = clip.buffer.data();

    static constexpr unsigned int block_size = 64;
    for (sample_block = block_size;
         sample_count;
         sample_count -= sample_block,
            stream += (sample_block * (2 * sizeof(float))))
	{
		if (sample_block > sample_count) sample_block = sample_count;

        double ms_per_block =
            1000.0 / static_cast<double>(state.sample_frequency);
		for (state.midi_ms += sample_block * ms_per_block;
             state.midi_msg && state.midi_ms >= state.midi_msg->time;
             state.midi_msg = state.midi_msg->next)
		{
			switch (state.midi_msg->type)
			{
				case TML_PROGRAM_CHANGE:
                    // channel program (preset) change
                    // (special handling for 10th MIDI channel with drums)
					tsf_channel_set_presetnumber(
                        state.sound_font,
                        state.midi_msg->channel,
                        state.midi_msg->program,
                        (state.midi_msg->channel == 9)
                    );
					break;
				case TML_NOTE_ON:
                    // play a note
					tsf_channel_note_on(
                        state.sound_font,
                        state.midi_msg->channel,
                        state.midi_msg->key,
                        state.midi_msg->velocity / 127.0f
                    );
					break;
				case TML_NOTE_OFF:
                    // stop a note
					tsf_channel_note_off(
                        state.sound_font,
                        state.midi_msg->channel,
                        state.midi_msg->key
                    );
					break;
				case TML_PITCH_BEND:
                    // pitch wheel modification
					tsf_channel_set_pitchwheel(
                        state.sound_font,
                        state.midi_msg->channel,
                        state.midi_msg->pitch_bend
                    );
					break;
				case TML_CONTROL_CHANGE:
                    // MIDI controller messages
					tsf_channel_midi_control(
                        state.sound_font,
                        state.midi_msg->channel,
                        state.midi_msg->control,
                        state.midi_msg->control_value
                    );
					break;
			}
		}

		// Render the block of audio samples in float format
		tsf_render_float(state.sound_font, (float*)stream, sample_block, 0);
	}
}

void AudioManager::queue_midi_stream(
    ALuint const source,
    MidiClip & clip
) {
    ALint n_processed = 0;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &n_processed);

    clip.n_queued -= n_processed;

    if (clip.n_queued < 2)
    {
        ALuint buffer;
        alSourceUnqueueBuffers(source, 1, &buffer);

        alBufferData(
            buffer,
            AL_FORMAT_STEREO_FLOAT32,
            clip.buffer.data(),
            MidiClip::buffer_size,
            m_sample_rate
        );
        alSourceQueueBuffers(source, 1, &buffer);

        clip.buffer_consumed = true;

        ++clip.n_queued;
    }
}

MidiID AudioManager::load_midi(char const * path) {
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

    return id;
}

void AudioManager::free_midi(MidiID id) {
    if (m_midis[id] == m_current_track.origin) {
        stop_midi();
    }

    tml_free(m_midis[id]);
    m_midis[id] = nullptr;
    m_free_midi_ids.push_back(id);
}

SoundFontID AudioManager::load_sound_font(char const * path) {
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

    return id;
}

void AudioManager::free_sound_font(SoundFontID id) {
    tsf_close(m_sound_fonts[id]);
    m_sound_fonts[id] = nullptr;
    m_free_sound_font_ids.push_back(id);
}

void AudioManager::play_midi(MidiID midi_id, SoundFontID sound_font_id) {
    m_current_track.origin = m_midis[midi_id];
    m_current_track.n_queued = 0;

    m_current_track.state.sound_font = m_sound_fonts[sound_font_id];
    m_current_track.state.midi_msg = m_midis[midi_id];
    m_current_track.state.midi_ms = 0.0;

    alSourceQueueBuffers(m_track_source, n_track_buffers, &m_track_buffers[0]);
    alSourcePlay(m_track_source);
}

void AudioManager::stop_midi() {
    m_current_track = {};
}

void AudioManager::init() {
#ifdef __EMSCRIPTEN__
    m_sample_rate = EM_ASM_INT({
        var AudioContext = window.AudioContext || window.webkitAudioContext;
        var ctx = new AudioContext();
        var sr = ctx.sampleRate;
        ctx.close();
    });
    if (m_sample_rate == 0) {
        PRT3ERROR("Failed to create audio context.\n");
        return;
    }

#else //__EMSCRIPTEN__
    m_sample_rate = 44100;
#endif //__EMSCRIPTEN__

    ALCdevice * device = alcOpenDevice(NULL);
    if (!device)
    {
        PRT3ERROR("Failed to AL device.\n");
    }

    m_al_context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(m_al_context)) {
        PRT3ERROR("Failed to make AL context current.\n");
    }

    alGenBuffers(n_track_buffers, &m_track_buffers[0]);

    alGenSources(1, &m_track_source);
    alSourcef(m_track_source, AL_PITCH, 1);
    alSourcef(m_track_source, AL_GAIN, 1.0f);
    alSource3f(m_track_source, AL_POSITION, 0, 0, 0);
    alSource3f(m_track_source, AL_VELOCITY, 0, 0, 0);
    alSourcei(m_track_source, AL_LOOPING, AL_FALSE);

    // TODO: remove >>>
    MidiID midi = load_midi("assets/audio/tracks/maze.mid");
    SoundFontID sf = load_sound_font("assets/audio/soundfonts/CT2MGM.sf2");

    play_midi(midi, sf);
    // TODO: <<< remove

    m_initialized = true;
}
