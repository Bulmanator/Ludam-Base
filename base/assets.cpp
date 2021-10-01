function Texture_Handle CreateTextureHandle(u32 index, u32 width, u32 height) {
    Texture_Handle result;
    result.index  = index;
    result.width  = cast(u16) width;
    result.height = cast(u16) height;

    return result;
}

function u8 *GetTextureTransferMemory(Texture_Transfer_Queue *texture_queue, Texture_Handle handle, u32 flags) {
    u8 *result = 0;
    if (texture_queue->transfer_count < ArraySize(texture_queue->transfer_info)) {
        uptr data_size = (4 * handle.width * handle.height);
        if ((texture_queue->transfer_used + data_size) > texture_queue->transfer_size) { return result; }

        result = texture_queue->transfer_base + texture_queue->transfer_used;

        Texture_Transfer_Info *info = &texture_queue->transfer_info[texture_queue->transfer_count];
        texture_queue->transfer_count += 1;

        info->handle = handle;
        info->flags  = flags;
        info->data   = result;

        texture_queue->transfer_used += data_size;
    }

    return result;
}

function u64 Hash64(str8 str) {
    u64 result = 5381;
    for (uptr it = 0; it < str.count; ++it) {
        result = ((result << 5) + result) + str.data[it];
    }

    return result;
}

function Path_List GetAmtFileList(Path_List *list, Memory_Arena *arena) {
    Path_List result = {};

    for (Path_Entry *entry = list->first; entry; entry = entry->next) {
        if (entry->type != PathEntry_File) { continue; }

        str8 ext = Suffix(entry->basename, 4);
        if (!StringsEqual(ext, WrapConst(".amt"))) { continue; }

        result.entry_count += 1;

        Path_Entry *copy = AllocCopy(arena, Path_Entry, entry);

        copy->next = 0;

        if (!result.first) {
            result.first = copy;
            result.last  = copy;
        }
        else {
            result.last->next = copy;
            result.last       = copy;
        }
    }

    return result;
}

function void AddHashLookupForAsset(Asset_Manager *assets, u32 index, str8 name) {
    u64 hash = Hash64(name);
    u32 hash_index = hash % ArraySize(assets->hash_slots);

    Asset_Hash *asset_hash = AllocType(assets->arena, Asset_Hash);

    asset_hash->hash_value  = hash;
    asset_hash->asset_index = index;

    asset_hash->next = assets->hash_slots[hash_index];
    assets->hash_slots[hash_index] = asset_hash;
}

function b32 IsValid(Image_Handle handle) {
    b32 result = (handle.value != 0);
    return result;
}

function b32 IsValid(Sound_Handle handle) {
    b32 result = (handle.value != 0);
    return result;
}

function void Initialise(Asset_Manager *assets, Memory_Arena *arena, Texture_Transfer_Queue *texture_queue, u32 default_texture_flags) {
    Scratch_Memory scratch = GetScratch();
    assets->arena = arena;

    assets->texture_queue = texture_queue;
    assets->default_texture_flags = default_texture_flags;

    str8 exe_path  = Platform->GetPath(PlatformPath_Executable);
    str8 data_path = FormatStr(scratch.arena, "%.*s/%s", str8_unpack(exe_path), "data");

    assets->asset_count = 1;
    assets->file_count  = 0;

    Path_List list = Platform->ListPath(scratch.arena, data_path);
    Path_List amt_list = GetAmtFileList(&list, scratch.arena);

    assets->file_count = 0;
    assets->files      = AllocArray(assets->arena, Asset_File, amt_list.entry_count);
    for (Path_Entry *entry = amt_list.first; entry; entry = entry->next) {
        Asset_File *file = &assets->files[assets->file_count];

        file->handle = Platform->OpenFile(entry->full_path, FileAccess_Read);
        if (file->handle.errors) { continue; }

        Amt_Header *header = &file->header;
        Platform->ReadFile(&file->handle, header, 0, sizeof(Amt_Header));

        if (header->signature != AMTF_SIGNATURE) {
            Platform->CloseFile(&file->handle);
            continue;
        }

        if (header->version != AMTF_VERSION) {
            Platform->CloseFile(&file->handle);
            continue;
        }

        file->base_asset_index  = assets->asset_count;
        assets->asset_count    += file->header.asset_count;

        assets->file_count += 1;
    }

    u32 asset_index = 1;
    u32 file_index  = 0;

    assets->assets = AllocArray(assets->arena, Asset, assets->asset_count);

    for (Path_Entry *entry = amt_list.first; entry; entry = entry->next) {
        Asset_File *file = &assets->files[file_index];

        // Load the amt asset information from the file
        //
        Amt_Asset *amt_assets = AllocArray(scratch.arena, Amt_Asset, file->header.asset_count);
        Platform->ReadFile(&file->handle, amt_assets, sizeof(Amt_Header), file->header.asset_count * sizeof(Amt_Header));

        // Load the user data from the file
        //
        u64 user_data_offset = file->header.user_data_offset;
        u64 user_data_size   = file->header.user_data_size;
        u8 *user_data        = AllocArray(scratch.arena, u8, user_data_size);

        Platform->ReadFile(&file->handle, user_data, user_data_offset, user_data_size);

        for (u32 it = 0; it < file->header.asset_count; ++it) {
            Asset *asset = &assets->assets[asset_index];

            asset->loaded      = false;
            asset->type        = amt_assets[it].type;

            asset->file_index  = file_index;

            asset->amt         = amt_assets[it];

            str8 name = WrapZ(user_data);
            AddHashLookupForAsset(assets, asset_index, name);

            user_data += (name.count + 1);

            asset_index += 1;
        }

        file_index += 1;
    }

    assets->sample_buffer_size = Megabytes(256);
    assets->sample_buffer_used = 0;
    assets->sample_buffer      = AllocArray(assets->arena, u8, assets->sample_buffer_size);

    Texture_Handle handle = CreateTextureHandle(0, 1, 1);
    u8 *white_texture = GetTextureTransferMemory(assets->texture_queue, handle, 0);

    white_texture[0] = 0xFF;
    white_texture[1] = 0xFF;
    white_texture[2] = 0xFF;
    white_texture[3] = 0xFF;

    assets->next_texture_index = 1;
}

function void SetDefaultTextureFlags(Asset_Manager *assets, u32 flags) {
    assets->default_texture_flags = flags;
}

function u32 GetAssetIndexByName(Asset_Manager *assets, str8 name) {
    u32 result = 0;

    u64 hash_value = Hash64(name);
    u32 hash_index = hash_value % ArraySize(assets->hash_slots);

    for (Asset_Hash *hash = assets->hash_slots[hash_index]; hash; hash = hash->next) {
        if (hash->hash_value == hash_value) {
            result = hash->asset_index;
            break;
        }
    }

    return result;
}

function Image_Handle GetImageByName(Asset_Manager *assets, str8 name) {
    Image_Handle result;
    result.value = GetAssetIndexByName(assets, name);

    return result;
}

function Image_Handle GetImageByName(Asset_Manager *assets, const char *name) {
    Image_Handle result = GetImageByName(assets, WrapZ(name));
    return result;
}

function Sound_Handle GetSoundByName(Asset_Manager *assets, str8 name) {
    Sound_Handle result;
    result.value = GetAssetIndexByName(assets, name);

    return result;
}

function Sound_Handle GetSoundByName(Asset_Manager *assets, const char *name) {
    Sound_Handle result = GetSoundByName(assets, WrapZ(name));
    return result;
}

function Asset *GetAssetByIndex(Asset_Manager *assets, u32 index) {
    Asset *result = 0;
    if (index < assets->asset_count) {
        result = &assets->assets[index];
    }

    return result;
}

function Amt_Image *GetImageInfo(Asset_Manager *assets, Image_Handle handle) {
    Amt_Image *result = 0;

    Asset *asset = GetAssetByIndex(assets, handle.value);
    if (asset && (asset->type == AmtType_Image)) {
        result = &asset->amt.image;
    }

    return result;
}

function Amt_Sound *GetSoundInfo(Asset_Manager *assets, Sound_Handle handle) {
    Amt_Sound *result = 0;

    Asset *asset = GetAssetByIndex(assets, handle.value);
    if (asset && (asset->type == AmtType_Sound)) {
        result = &asset->amt.sound;
    }

    return result;
}

function Texture_Handle GetImageData(Asset_Manager *assets, Image_Handle handle) {
    Texture_Handle result = {};

    Asset *asset = GetAssetByIndex(assets, handle.value);
    if (asset && (asset->type == AmtType_Image)) {
        if (!asset->loaded) { LoadImage(assets, handle); }

        result = asset->texture;
    }

    return result;
}

function s16 *GetSoundData(Asset_Manager *assets, Sound_Handle handle) {
    s16 *result = 0;

    Asset *asset = GetAssetByIndex(assets, handle.value);
    if (asset && (asset->type == AmtType_Sound)) {
        if (!asset->loaded) { LoadSound(assets, handle); }

        result = cast(s16 *) (assets->sample_buffer + asset->sample_buffer_offset);
    }

    return result;
}

function void LoadImage(Asset_Manager *assets, Image_Handle handle) {
    Asset *asset = GetAssetByIndex(assets, handle.value);
    if (asset && (asset->type == AmtType_Image)) {
        u64 offset = asset->amt.data_offset;
        u64 size   = asset->amt.data_size;

        u32 width  = asset->amt.image.width;
        u32 height = asset->amt.image.height;

        Texture_Handle texture_handle = CreateTextureHandle(assets->next_texture_index, width, height);
        u8 *transfer_data = GetTextureTransferMemory(assets->texture_queue, texture_handle, assets->default_texture_flags);

        Asset_File *file = &assets->files[asset->file_index];
        Platform->ReadFile(&file->handle, transfer_data, offset, size);

        // @Todo: Make sure this is less than max_texture_handles
        //
        assets->next_texture_index += 1;

        asset->texture = texture_handle;
        asset->loaded = true;
    }
}

function void LoadSound(Asset_Manager *assets, Sound_Handle handle) {
    Asset *asset = GetAssetByIndex(assets, handle.value);
    if (asset && !asset->loaded) {
        if (asset->type != AmtType_Sound) { return; }

        u64 offset = asset->amt.data_offset;
        u64 size   = asset->amt.data_size;

        // Make sure we have enough space
        //
        if ((assets->sample_buffer_used + size) > assets->sample_buffer_size) { return; }

        u8 *data = assets->sample_buffer + assets->sample_buffer_used;

        Asset_File *file = &assets->files[asset->file_index];
        Platform->ReadFile(&file->handle, data, offset, size);

        asset->sample_buffer_offset = assets->sample_buffer_used;

        assets->sample_buffer_used += size;

        asset->loaded = true;
    }
}

