#if !defined(AMTPACK_H_)
#define AMTPACK_H_

#if !defined(BASE_ASSET_PATH)
#    define BASE_ASSET_PATH "../assets"
#endif

struct Packed_Asset {
    str8 name;

    Amt_Asset *amt;
    u8 *data;
};

struct Packer_Context {
    Memory_Arena arena;

    u32 asset_count;
    Amt_Asset *amt_assets;

    Packed_Asset *assets;

    uptr user_data_offset;
    uptr user_data_size;
    u8 *user_data;
};

// Wav loading
//
#pragma pack(push, 1)
struct Wav_Header {
    u32 riff_code;
    u32 size;
    u32 wave_code;
};

#define RIFF_CODE(a, b, c, d) (((u32) (d)) << 24 | ((u32) (c)) << 16 | ((u32) (b)) << 8 | ((u32) (a)) << 0)

#define PCM_FORMAT (0x0001)

enum Riff_Code {
    RiffCode_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
    RiffCode_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
    RiffCode_fmt  = RIFF_CODE('f', 'm', 't', ' '),
    RiffCode_data = RIFF_CODE('d', 'a', 't', 'a')
};

struct Wav_Chunk {
    u32 code;
    u32 size;
};

struct Fmt_Chunk {
    u16 format;
    u16 channels;
    u32 sample_rate;
    u32 data_rate;
    u16 block_align;
    u16 bit_depth;
    u16 ext_size;
    u16 valid_bits;
    u32 channel_mask;
    u8  sub_format[16];
};

#pragma pack(pop)

#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"

function void PackAssetsToAmt(Packer_Context *packer, str8 directory);

#endif // AMTPACK_H_
