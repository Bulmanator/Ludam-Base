#if !defined(BASE_AMT_H_)
#define BASE_AMT_H_

#define AMT_CODE(a, b, c, d) (((u32) (d)) << 24 | ((u32) (c)) << 16 | ((u32) (b)) << 8 | ((u32) (a)))

#define AMTF_SIGNATURE AMT_CODE('A', 'M', 'T', 'F')
#define AMTF_VERSION (2)

// These are the definitions for our custom file type to store assets. The type ends with .amt
//
// Its layout is as follows
//
// [Amt Header    | 1  * 128 bytes]
// [n * Amt_Asset | n  * 128 bytes]
// [Asset data    | file dependent]
// [User data     | file dependent]
//
// 'n' is the number of assets in the file. Defined by 'asset_count' in the header
// User data is defined by the packing system and can be anything desired. Its offset and size are defined by
// 'user_data_offset' and 'user_data_size' in the header
//

enum Amt_Type {
    AmtType_Unknown,

    AmtType_Image,
    AmtType_Sound
};

#pragma pack(push, 1)

struct Amt_Header {
    u32 signature;
    u32 version;

    u32 asset_count;

    u32 pad32[5];

    u64 user_data_offset;
    u64 user_data_size;

    u64 pad64[2];
};

struct Amt_Image {
    u32 width;
    u32 height;
};

struct Amt_Sound {
    u32 channel_count;
    u32 sample_count;
};

struct Amt_Asset {
    u32 type;
    union {
        Amt_Image image;
        Amt_Sound sound;

        u8 pad[44];
    };

    u64 data_offset;
    u64 data_size;
};

#pragma pack(pop)

#endif  // BASE_AMT_H_
