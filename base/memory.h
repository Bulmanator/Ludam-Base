#if !defined(BASE_MEMORY_H_)
#define BASE_MEMORY_H_

#define Bytes(x)     ((x) <<  0)
#define Kilobytes(x) ((x) << 10)
#define Megabytes(x) ((x) << 20)
#define Gigabytes(x) ((x) << 30)
#define Terabytes(x) ((x) << 40)

#define MEMORY_ALLOCATOR_RESERVE(name) void *name(void *context, uptr size, u32 flags)
#define MEMORY_ALLOCATOR_MODIFY(name)  void  name(void *context, void *base, uptr size)

typedef MEMORY_ALLOCATOR_RESERVE(Memory_Allocator_Reserve);
typedef MEMORY_ALLOCATOR_MODIFY(Memory_Allocator_Modify);

struct Memory_Allocator {
    void *context; // global to the allocator, used by the platform. Can be null

    Memory_Allocator_Reserve *Reserve;
    Memory_Allocator_Modify  *Commit;
    Memory_Allocator_Modify  *Decommit;
    Memory_Allocator_Modify  *Release;
};

#if !defined(MEMORY_ARENA_COMMIT_ALIGN)
#    define MEMORY_ARENA_COMMIT_ALIGN Kilobytes(4)
#endif

struct Memory_Arena {
    Memory_Allocator *alloc;

    u8  *base;
    uptr size;
    uptr used;

    uptr committed;
};

enum Allocation_Flags {
    Allocation_NoClear = 0x1,
};

struct Scratch_Memory {
    Memory_Arena *arena;
    uptr used;

    ~Scratch_Memory();
};

// Initialises the arena for use and reserves the given size from the allocator
//
function void Initialise(Memory_Arena *arena, Memory_Allocator *alloc, uptr size);

// Resets the arena usage back to zero. If 'release' is supplied as true will also instruct the allocator to
// release the backing memory. If backing memory is released the arena will no longer be usable until
// initialising it again
//
function void Reset(Memory_Arena *arena, b32 release = false);

// Removes the given size from the last allocation that was made on the arena. This is not valid across multiple
// allocations because of alignment properties
//
function void RemoveSize(Memory_Arena *arena, uptr size);

// Allocation macros. These should be used to allocate from arenas.
//
// Parameters are as follows:
//     - arena = the memory arena to allocate from
//     - size / T / T count = The size / typename / typename and count to allocate
//     - flags = See allocation flags above. Defaults to 0.
//     - alignment = The byte boundary to align allocation to. Defaults to 4. Must be a power of 2.
//
#define AllocSize(arena, size, ...) __AllocSize(arena, size, ##__VA_ARGS__)
#define AllocType(arena, T, ...) (T *) __AllocSize(arena, sizeof(T), ##__VA_ARGS__)
#define AllocArray(arena, T, count, ...) (T *) __AllocSize(arena, (count) * sizeof(T), ##__VA_ARGS__)

// This will allocate a structure containing an arena within its own arena and return the pointer to that
// allocated structure to you. size is the max size of the arena
//
#define AllocInline(alloc, size, T, arena_name) (T *) __AllocInline(alloc, size, sizeof(T), OffsetTo(T, arena_name))

// Scratch memory
//
function Scratch_Memory CreateScratch(Memory_Arena *arena);

// These two pull from a pool of available scratch memory in thread local storage.
//
function Scratch_Memory GetScratch(Memory_Arena **conflicting_arenas, u32 count);
function Scratch_Memory GetScratch();

// Manually release scratch, this will happen automatically when the scope the scratch memory was created in
// exits
//
function void ReleaseScratch(Scratch_Memory *scratch);

// Utility functions
//
function void ZeroSize(void *base, uptr size);
function void CopySize(void *dst, void *src, uptr size);

#endif  // BASE_MEMORY_H_
