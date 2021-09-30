#if !defined(BASE_AUDIO_H_)
#define BASE_AUDIO_H_

struct Audio_Buffer {
    s16 *samples;
    u32  sample_count;
};

enum Playing_Sound_Flags {
    PlayingSound_Looped = (1 << 0),
};

struct Playing_Sound {
    Sound_Handle handle;

    v2 volume;
    u32 samples_played;
    u32 flags;

    b32 free;
    Playing_Sound *next;
    Playing_Sound *prev;
};

struct Audio_State {
    Memory_Arena *arena;

    v2 global_volume;

    Playing_Sound playing;
    Playing_Sound *free;
};

// Initialise the audio state with default parameters
//
function void Initialise(Audio_State *state, Memory_Arena *arena, v2 global_volume = V2(0.5f, 0.5f));

// Mixes all currently playing sound into the audio buffer supplied. Mainly for the platform layer to call
// so it can actually output sounds
//
function void MixPlayingSounds(Audio_State *state, Asset_Manager *assets, Audio_Buffer *buffer);

// Play the sound attached to the given handle with the supplied parameters. The volume value does not override
// the global volume value, they are instead multiplied together
//
function Playing_Sound *PlaySound(Audio_State *state, Sound_Handle handle, u32 flags = 0, v2 volume = V2(1.0f, 1.0f));

// Stop a specific sound from playing
//
function void StopSound(Audio_State *state, Playing_Sound *sound);

// Set the global volume value for all playing sounds
//
function void SetVolume(Audio_State *state, v2  volume = V2(0.5f, 0.5f));

#endif  // BASE_AUDIO_H_
