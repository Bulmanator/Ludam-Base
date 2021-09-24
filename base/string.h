#if !defined(BASE_STRING_H_)
#define BASE_STRING_H_

#define WrapConst(str) WrapCount((u8 *) str, sizeof(str) - 1)

function str8 WrapCount(u8 *data, uptr count);
function str8 WrapRange(u8 *start, u8 *end);

function str8 WrapZ(u8 *z);
function str8 WrapZ(const char *z);

function str8 Advance(str8 base, uptr count);
function str8 Remove(str8 base, uptr count);

function str8 Suffix(str8 base, uptr count);
function str8 Prefix(str8 base, uptr count);

function str8 Substring(str8 base, uptr start, uptr end);

enum String_Compare_Flags {
    StringCompare_NoCase = (1 << 0),
};

function b32 StringsEqual(str8 a, str8 b, u32 flags = 0);

function u8 ToUppercase(u8 c);
function u8 ToLowercase(u8 c);

#endif  // BASE_STRING_H_
