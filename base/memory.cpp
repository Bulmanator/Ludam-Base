function void Initialise(Memory_Arena *arena, Memory_Allocator *alloc, uptr size) {
    arena->alloc     = alloc;

    arena->base      = cast(u8 *) alloc->Reserve(alloc->context, size, 0);
    arena->size      = size;
    arena->used      = 0;
    arena->committed = 0;
}

function void Reset(Memory_Arena *arena, b32 release) {
    arena->used = 0;

    if (release) {
        Memory_Allocator *alloc = arena->alloc;

        u8  *base = arena->base;
        uptr size = arena->size;

        arena->alloc     = 0;

        arena->base      = 0;
        arena->size      = 0;
        arena->committed = 0;

        alloc->Release(alloc->context, base, size);
    }
}

function void RemoveSize(Memory_Arena *arena, uptr size) {
    if (arena->used >= size) {
        arena->used -= size;
    }
}

function uptr GetAlignmentOffset(Memory_Arena *arena, uptr alignment) {
    uptr result = 0;

    uptr base_ptr   = cast(uptr) (arena->base + arena->used);
    uptr align_mask = alignment - 1;
    if (base_ptr & align_mask) {
        result = alignment - (base_ptr & align_mask);
    }

    return result;
}

function void *__AllocSize(Memory_Arena *arena, uptr size, u32 flags = 0, uptr alignment = 4) {
    void *result = 0;

    // Align to desired alignment
    //
    uptr align_offset = GetAlignmentOffset(arena, alignment);
    uptr align_size   = size + align_offset;

    // Make sure there is enough space in the arena
    //
    uptr total_usage = arena->used + align_size;
    if (total_usage <= arena->size) {
        // Commit more memory if it hasn't been committed already
        //
        if (total_usage > arena->committed) {
            uptr commit_size = AlignTo(align_size, MEMORY_ARENA_COMMIT_ALIGN);
            u8  *commit_base = arena->base + arena->committed;

            Memory_Allocator *alloc = arena->alloc;
            alloc->Commit(alloc->context, commit_base, commit_size);

            arena->committed += commit_size;
        }

        result = cast(void *) (arena->base + arena->used + align_offset);
        arena->used = total_usage;

        // Clear memory to zero unless otherwise specified
        //
        b32 no_clear = (flags & Allocation_NoClear);
        if (!no_clear) {
            ZeroSize(result, size);
        }
    }

    return result;
}

function void *__AllocInline(Memory_Allocator *alloc, uptr size, uptr type_size, uptr arena_offset) {
    void *result = 0;

    Memory_Arena arena;
    Initialise(&arena, alloc, size);

    result = AllocSize(&arena, type_size);
    *cast(Memory_Arena *) (cast(u8 *) result + arena_offset) = arena;

    return result;
}

// Utility functions
//
function void ZeroSize(void *base, uptr size) {
    u8 *base8 = cast(u8 *) base;
    for (uptr it = 0; it < size; ++it) {
        base8[it] = 0;
    }
}

function void CopySize(void *dst, void *src, uptr size) {
    u8 *dst8 = cast(u8 *) dst;
    u8 *src8 = cast(u8 *) src;

    for (uptr it = 0; it < size; ++it) {
        dst8[it] = src8[it];
    }
}

