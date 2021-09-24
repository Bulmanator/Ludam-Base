Scratch_Memory::~Scratch_Memory() {
    ReleaseScratch(this);
}

function Scratch_Memory CreateScratch(Memory_Arena *arena) {
    Scratch_Memory result;
    result.arena = arena;
    result.used  = arena->used;

    return result;
}

function void ReleaseScratch(Scratch_Memory *scratch) {
    Memory_Arena *arena = scratch->arena;
    if (arena->used > scratch->used) {
        arena->used = scratch->used;
    }
}

function Scratch_Memory GetScratch(Memory_Arena **conflicting_arenas, u32 count) {
    Scratch_Memory result = {};

    Thread_Context *tctx = Platform->GetThreadContext();

    for (u32 it = 0; it < ArraySize(tctx->scratch); ++it) {
        Memory_Arena *arena = &tctx->scratch[it];

        result.arena = arena;
        for (u32 c = 0; c < count; ++c) {
            Memory_Arena *conflict = conflicting_arenas[c];
            if (conflict == arena) {
                result.arena = 0;
                break;
            }
        }

        if (result.arena) {
            result.used = result.arena->used;
            break;
        }
    }

    return result;
}

function Scratch_Memory GetScratch() {
    Scratch_Memory result = GetScratch(0, 0);
    return result;
}

