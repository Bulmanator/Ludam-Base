function void Initialise(Audio_State *state, Memory_Arena *arena, v2 global_volume) {
    state->arena         = arena;
    state->global_volume = global_volume;
    state->free          = 0;

    state->playing.next  = &state->playing;
    state->playing.prev  = &state->playing;
}

function void MixPlayingSounds(Audio_State *state, Asset_Manager *assets, Audio_Buffer *buffer) {
    Scratch_Memory scratch = GetScratch();

    f32 *left  = AllocArray(scratch.arena, f32, buffer->sample_count);
    f32 *right = AllocArray(scratch.arena, f32, buffer->sample_count);

    Playing_Sound *sentinel = &state->playing;

    for (Playing_Sound *sound = sentinel->next; (sound != sentinel);) {
        Playing_Sound *next = sound->next;

        Amt_Sound *info = GetSoundInfo(assets, sound->handle);
        s16 *samples    = GetSoundData(assets, sound->handle);

        u32 sample_count = Min(buffer->sample_count, info->sample_count - sound->samples_played);

        // @Todo: This is offset by 2 * sound->samples_played because there are two channels and we leave
        // samples interleaved. We should probably de-interleave them when packing so we can just have two
        // separate channel buffers to read from
        //
        samples += (2 * sound->samples_played);
        for (u32 it = 0; it < sample_count; ++it) {
            left[it]  += state->global_volume.x * sound->volume.x * (*samples++);
            right[it] += state->global_volume.y * sound->volume.y * (*samples++);
        }

        sound->samples_played += sample_count;

        b32 is_looped   = (sound->flags & PlayingSound_Looped);
        b32 is_finished = (sound->samples_played == info->sample_count);
        if (is_finished) {
            if (is_looped) {
                sound->samples_played = 0;
            }
            else {
                StopSound(state, sound);
            }
        }

        sound = next;
    }

    s16 *out_samples = buffer->samples;
    for (u32 it = 0; it < buffer->sample_count; ++it) {
        *out_samples++ = cast(s16) (left[it]  + 0.5f);
        *out_samples++ = cast(s16) (right[it] + 0.5f);
    }
}

function Playing_Sound *PlaySound(Audio_State *state, Sound_Handle handle, u32 flags, v2 volume) {
    Playing_Sound *result = 0;
    if (state->free) {
        result = state->free;
        state->free = result->next;
    }
    else {
        result = AllocType(state->arena, Playing_Sound, Allocation_NoClear);
    }

    ZeroSize(result, sizeof(Playing_Sound));

    result->handle = handle;

    result->samples_played = 0;

    result->volume = volume;
    result->flags  = flags;

    result->free   = false;

    // Insert into dlist
    //
    Playing_Sound *sentinel = &state->playing;
    result->prev         = sentinel;
    result->next         = sentinel->next;
    sentinel->next->prev = result;
    sentinel->next       = result;

    return result;
}

function void StopSound(Audio_State *state, Playing_Sound *sound) {
    if (sound->free) { return; }

    // Remove from dlist
    //
    sound->next->prev = sound->prev;
    sound->prev->next = sound->next;

    sound->next = state->free;
    state->free = sound;

    sound->free = true;

    sound->prev = 0;
}

function void SetVolume(Audio_State *state, v2 volume) {
    state->global_volume.x = Clamp01(volume.x);
    state->global_volume.y = Clamp01(volume.y);
}
