function Path_List GetAssetFileList(Memory_Arena *arena, Path_List *dir_list) {
    Path_List result = {};

    str8 valid_extensions[] = {
        WrapConst(".png"),
        WrapConst(".wav")
    };

    for (Path_Entry *entry = dir_list->first; entry != 0; entry = entry->next) {
        str8 ext = Suffix(entry->basename, 4);

        b32 valid = false;
        for (u32 it = 0; it < ArraySize(valid_extensions); ++it) {
            if (StringsEqual(ext, valid_extensions[it])) {
                valid = true;
                break;
            }
        }

        if (!valid) { continue; }

        Path_Entry *copy = AllocCopy(arena, Path_Entry, entry);
        copy->next = 0;

        if (!result.first) {
            result.first = copy;
            result.last  = copy;
        }
        else {
            result.last->next = copy;
            result.last = copy;
        }

        result.entry_count += 1;
    }

    return result;
}

function b32 LoadSoundAsset(Packer_Context *packer, Packed_Asset *asset, Path_Entry *entry) {
    b32 result = false;

    Scratch_Memory scratch = GetScratch();

    File_Handle handle = Platform->OpenFile(entry->full_path, FileAccess_Read);
    if (handle.errors) { return result; }

    u8 *file_data = AllocArray(scratch.arena, u8, entry->size, Allocation_NoClear);
    Platform->ReadFile(&handle, file_data, 0, entry->size);
    if (handle.errors) { return result; }

    Wav_Header *header = cast(Wav_Header *) file_data;
    if (header->riff_code != RiffCode_RIFF) {
        return result;
    }

    if (header->wave_code != RiffCode_WAVE) {
        return result;
    }

    u32 size = header->size;
    u32 size_used = sizeof(Wav_Header);

    u32 channel_count = 0;

    Wav_Chunk *chunk = cast(Wav_Chunk *) (header + 1);
    while (size_used < size) {
        switch (chunk->code) {
            case RiffCode_fmt: {
                Fmt_Chunk *fmt = cast(Fmt_Chunk *) (chunk + 1);

                if (fmt->format      != PCM_FORMAT) { return result; }
                if (fmt->channels    != 2) { return result; }
                if (fmt->sample_rate != 48000) { return result; }
                if (fmt->bit_depth   != 16) { return result; }

                channel_count = fmt->channels;
            }
            break;
            case RiffCode_data: {
                if (channel_count == 0) { return result; }

                u32 sample_count = chunk->size / (channel_count * sizeof(s16));
                asset->amt->data_size = chunk->size;

                asset->amt->sound.channel_count = channel_count;
                asset->amt->sound.sample_count  = sample_count;

                asset->data = AllocArray(&packer->arena, u8, chunk->size);
                CopySize(asset->data, cast(void *) (chunk + 1), chunk->size);
            }
            break;
            default: {
                // Ignore chunks we don't care about
                //
            }
            break;
        }

        size_used += (chunk->size + sizeof(Wav_Chunk));
        chunk = cast(Wav_Chunk *) (cast(u8 *) (chunk + 1) + chunk->size);
    }

    Platform->CloseFile(&handle);

    asset->amt->type = AmtType_Sound;

    result = true;
    return result;
}

function b32 LoadImageAsset(Packer_Context *packer, Packed_Asset *asset, Path_Entry *entry) {
    b32 result = false;

    (void) packer;

    Scratch_Memory scratch = GetScratch();

    const char *zpath = CopyZ(scratch.arena, entry->full_path);

    s32 width, height, c;
    u8 *data = stbi_load(zpath, &width, &height, &c, 0);
    if (!data) {
        return result;
    }

    if (c != 4) {
        return result;
    }

    asset->amt->type = AmtType_Image;

    asset->amt->image.width  = width;
    asset->amt->image.height = height;

    asset->amt->data_size = 4 * width * height;

    asset->data = data;

    result = true;
    return result;
}

function void PackAssetsToAmt(Packer_Context *packer, str8 directory) {
    Scratch_Memory scratch = GetScratch();

    Path_List dir_list = Platform->ListPath(scratch.arena, directory);
    Path_List asset_list = GetAssetFileList(scratch.arena, &dir_list);

    if (!asset_list.entry_count) {
        return; // No assets
    }

    packer->asset_count = asset_list.entry_count;

    str8 working = Platform->GetPath(PlatformPath_Executable);
    str8 file = FormatStr(scratch.arena, "%.*s/data/data.amt", str8_unpack(working));

    File_Handle current_handle = Platform->OpenFile(file, FileAccess_Read);
    if (!current_handle.errors) {
        Amt_Header header = {};
        Platform->ReadFile(&current_handle, &header, 0, sizeof(Amt_Header));

        if (header.asset_count == packer->asset_count) {
            // Already packed, nothing to do
            //
            Platform->CloseFile(&current_handle);
            return;
        }
    }

    Initialise(&packer->arena, Platform->GetMemoryAllocator(), Gigabytes(1));

    packer->amt_assets = AllocArray(&packer->arena, Amt_Asset, packer->asset_count);

    packer->assets = AllocArray(&packer->arena, Packed_Asset, packer->asset_count);

    u64 user_data_size = 0;

    u32 asset_index = 0;
    for (Path_Entry *entry = asset_list.first; entry != 0; entry = entry->next) {
        Packed_Asset *asset = &packer->assets[asset_index];

        str8 ext  = Suffix(entry->basename, 4);
        str8 name = Remove(entry->basename, 4);

        asset->name = name;
        asset->amt  = &packer->amt_assets[asset_index];
        asset->data = 0;

        if (StringsEqual(ext, WrapConst(".wav"))) {
            if (!LoadSoundAsset(packer, asset, entry)) { continue; }
        }
        else if (StringsEqual(ext, WrapConst(".png"))) {
            if (!LoadImageAsset(packer, asset, entry)) { continue; }
        }

        user_data_size += (name.count + 1);

        asset_index += 1;
    }

    u8 *user_data = AllocArray(scratch.arena, u8, user_data_size);
    u8 *user_ptr  = user_data;

    u64 current_data_offset = sizeof(Amt_Header) + (asset_index * sizeof(Amt_Asset));
    for (u32 it = 0; it < asset_index; ++it) {
        Packed_Asset *asset = &packer->assets[it];

        CopySize(user_ptr, asset->name.data, asset->name.count);
        user_ptr[asset->name.count] = 0;

        user_ptr += (asset->name.count + 1);

        asset->amt->data_offset = current_data_offset;
        current_data_offset += asset->amt->data_size;
    }

    Amt_Header header = {};
    header.signature   = AMTF_SIGNATURE;
    header.version     = AMTF_VERSION;

    header.asset_count = asset_index;

    header.user_data_offset = current_data_offset;
    header.user_data_size   = user_data_size;

    File_Handle handle = Platform->OpenFile(file, FileAccess_Write);
    if (handle.errors) { return; }

    // Write header
    //
    Platform->WriteFile(&handle, &header, 0, sizeof(Amt_Header));

    // Write asset information
    //
    Platform->WriteFile(&handle, packer->amt_assets, sizeof(Amt_Header), asset_index * sizeof(Amt_Asset));

    // Write asset data
    //
    for (u32 it = 0; it < asset_index; ++it) {
        Packed_Asset *asset = &packer->assets[it];

        Platform->WriteFile(&handle, asset->data, asset->amt->data_offset, asset->amt->data_size);
    }

    // Write user data
    //
    Platform->WriteFile(&handle, user_data, header.user_data_offset, header.user_data_size);

    Platform->CloseFile(&handle);

    Reset(&packer->arena, true);
}
