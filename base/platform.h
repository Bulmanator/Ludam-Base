#if !defined(BASE_PLATFORM_H_)
#define BASE_PLATFORM_H_

enum Platform_Init_Flags {
    PlatformInit_OpenWindow  = (1 << 0),
    PlatformInit_EnableAudio = (1 << 1),
};

#define PLATFORM_GET_MEMORY_ALLOCATOR(name) Memory_Allocator *name(void)
typedef PLATFORM_GET_MEMORY_ALLOCATOR(Platform_Get_Memory_Allocator);

struct Thread_Context {
    // @Todo: We can add more things here if need be
    //
    Memory_Arena scratch[4];
};

#define PLATFORM_GET_THREAD_CONTEXT(name) Thread_Context *name(void)
typedef PLATFORM_GET_THREAD_CONTEXT(Platform_Get_Thread_Context);

enum Platform_Path {
    PlatformPath_Executable = 0,
    PlatformPath_User,
    PlatformPath_Working
};

#define PLATFORM_GET_PATH(name) str8 name(Platform_Path path)
typedef PLATFORM_GET_PATH(Platform_Get_Path);

enum Path_Entry_Type {
    PathEntry_File = 0,
    PathEntry_Directory
};

struct Path_Entry {
    Path_Entry_Type type;

    str8 full_path;
    str8 basename;

    uptr size;
    u64  time;

    Path_Entry *next;
};

struct Path_List {
    u32 entry_count;

    Path_Entry *first;
    Path_Entry *last;
};

#define PLATFORM_LIST_PATH(name) Path_List name(Memory_Arena *arena, str8 path)
typedef PLATFORM_LIST_PATH(Platform_List_Path);

enum File_Access_Flags {
    FileAccess_Read  = (1 << 0),
    FileAccess_Write = (1 << 1)
};

struct File_Handle {
    b32 errors;
    void *platform;
};

#define PLATFORM_OPEN_FILE(name) File_Handle name(str8 path, u32 access_flags)
#define PLATFORM_CLOSE_FILE(name) void name(File_Handle *handle)

typedef PLATFORM_OPEN_FILE(Platform_Open_File);
typedef PLATFORM_CLOSE_FILE(Platform_Close_File);

#define PLATFORM_ACCESS_FILE(name) void name(File_Handle *handle, void *data, uptr offset, uptr size)
typedef PLATFORM_ACCESS_FILE(Platform_Access_File);

struct Platform_Context {
    Platform_Get_Memory_Allocator *GetMemoryAllocator;
    Platform_Get_Thread_Context   *GetThreadContext;

    Platform_Get_Path   *GetPath;

    Platform_List_Path  *ListPath;

    Platform_Open_File  *OpenFile;
    Platform_Close_File *CloseFile;

    Platform_Access_File *ReadFile;
    Platform_Access_File *WriteFile;
};

extern Platform_Context *Platform;

#endif  // BASE_PLATFORM_H_
