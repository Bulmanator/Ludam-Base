#if !defined(BASE_ASSETS_H_)
#define BASE_ASSETS_H_

struct Image_Handle {
    u32 value;
};

struct Sound_Handle {
    u32 value;
};

struct Asset_Hash {
    u64 hash_value;
    u32 asset_index;

    Asset_Hash *next;
};

struct Asset_File {
    Amt_Header header;
    u32 base_asset_index;

    File_Handle handle;
};

struct Asset {
    b32 loaded;

    u32 type;
    u32 file_index;

    union {
        Texture_Handle texture;
        u64 sample_buffer_offset;
    };

    Amt_Asset amt;
};

struct Asset_Manager {
    Memory_Arena *arena;

    u32 asset_count;
    Asset *assets;

    u32 file_count;
    Asset_File *files;

    u8  *sample_buffer;
    uptr sample_buffer_used;
    uptr sample_buffer_size;

    Texture_Transfer_Queue *texture_queue;
    u32 next_texture_index;
    u32 default_texture_flags;

    Asset_Hash *hash_slots[256];
};

// Validity tests for asset handles
//
function b32 IsValid(Image_Handle handle);
function b32 IsValid(Sound_Handle handle);

// Initialise the asset manager
//
function void Initialise(Asset_Manager *assets, Memory_Arena *arena, Texture_Transfer_Queue *texture_queue, u32 default_texture_flags = 0);

function void SetDefaultTextureFlags(Asset_Manager *assets, u32 flags);

// Get a handle to an image asset using its human readable name
//
function Image_Handle GetImageByName(Asset_Manager *assets, str8 name);
function Image_Handle GetImageByName(Asset_Manager *assets, const char *name);

// Get a handle to a sound asset using its human readable name
//
function Sound_Handle GetSoundByName(Asset_Manager *assets, str8 name);
function Sound_Handle GetSoundByName(Asset_Manager *assets, const char *name);

// Get information about a given asset from its handle
//
function Amt_Image *GetImageInfo(Asset_Manager *assets, Image_Handle handle);
function Amt_Sound *GetSoundInfo(Asset_Manager *assets, Sound_Handle handle);

// Get backing data attached to an asset. These calls will implicitly load the asset if it wasn't already
// loaded
//
function Texture_Handle GetImageData(Asset_Manager *assets, Image_Handle handle);
function s16 *GetSoundData(Asset_Manager *assets, Sound_Handle handle);

// Manually load an asset using its given handle
//
function void LoadImage(Asset_Manager *assets, Image_Handle handle);
function void LoadSound(Asset_Manager *assets, Sound_Handle handle);

#endif  // BASE_ASSETS_H_
