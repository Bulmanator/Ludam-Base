#if !defined(BASE_STRING_H_)
#define BASE_STRING_H_

#if defined(function)
#    undef function
#endif

// @Temp: This is here until we implement our own format string routine
//
#include <stdio.h>

#define function static

#define WrapConst(str) WrapCount((u8 *) str, sizeof(str) - 1)

function str8 WrapCount(u8 *data, uptr count);
function str8 WrapRange(u8 *start, u8 *end);

function str8 WrapZ(u8 *z);
function str8 WrapZ(const char *z);

function b32 IsValid(str8 str);

function str8 Advance(str8 base, uptr count);
function str8 Remove(str8 base, uptr count);

function str8 Suffix(str8 base, uptr count);
function str8 Prefix(str8 base, uptr count);

function str8 Substring(str8 base, uptr start, uptr end);

// Formatting strings
//
#define str8_unpack(str) (u32) (str).count, (str).data

function str8 FormatStrArgs(Memory_Arena *arena, const char *format, va_list args);
function str8 FormatStrArgs(str8 buffer, const char *format, va_list args);

function str8 FormatStr(Memory_Arena *arena, const char *format, ...);
function str8 FormatStr(str8 buffer, const char *format, ...);

// Make a copy of the string that is null-terminated
//
function const char *CopyZ(Memory_Arena *arena, str8 str);

enum String_Compare_Flags {
    StringCompare_NoCase = (1 << 0),
};

function b32 StringsEqual(str8 a, str8 b, u32 flags = 0);

function u8 ToUppercase(u8 c);
function u8 ToLowercase(u8 c);

#endif  // BASE_STRING_H_
