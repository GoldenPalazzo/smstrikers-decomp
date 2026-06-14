#include <stdio.h>

#include <dolphin/PPCArch.h>
#include <dolphin/types.h>

static u32* g_baseARAMtoVM;
static u32* g_baseVMtoARAM;
static u32 g_totalAllocatedVM;

void* OSGetArenaLo(void);
void OSSetArenaLo(void* newLo);
u32 VMGetARAMBase(void);
u32 VMGetARAMSize(void);
void __VMMappingErrorAlert(u32 virtualPage);

static inline u32 VMToARAMOffset(u32 virtualAddr)
{
    return (virtualAddr >> 10) & 0x1FFC;
}

#pragma optimize_for_size off
BOOL VMAlloc(u32 address, u32 size)
{
    static u32 g_nextARAMPageToCheck;
    u32 i;
    u32 startAramPage = VMGetARAMBase() >> 12;
    u32 endAramPage = startAramPage + (VMGetARAMSize() >> 12);

    if (g_nextARAMPageToCheck < startAramPage)
    {
        g_nextARAMPageToCheck = startAramPage;
    }

    if ((g_totalAllocatedVM + size) > VMGetARAMSize())
    {
        return FALSE;
    }

    /**
     * Offset/Address/Size: 0x0 | 0x8025F83C | size: 0x100
     * Matches at 100%. The function-local `static` matches the original's
     * block-scope mangling (target symbol g_nextARAMPageToCheck$233 -- the
     * $NNN form is MWCC's mangling for a block-scope static; our build emits
     * g_nextARAMPageToCheck$NN with a different per-TU counter value, but the
     * SDA relocation symbol-resolves either way).
     *
     * `#pragma optimize_for_size off` around VMAlloc prevents MWCC's -O4,s
     * from swapping the inline r28-r31 saves/restores for _savegpr_28 /
     * _restgpr_28 helper calls. The LUT functions below still need -O4,s
     * to avoid the clear-loop unrolling, so the pragma is scoped to VMAlloc.
     */
    for (i = 0; i < size; i += 0x1000)
    {
        u32 virtualPage = address + i;

        while (TRUE)
        {
            u32 next = g_nextARAMPageToCheck + 1;
            g_nextARAMPageToCheck = next;
            if (next >= endAramPage)
            {
                g_nextARAMPageToCheck = startAramPage;
            }

            if (g_baseARAMtoVM[g_nextARAMPageToCheck] == 0)
            {
                break;
            }
        }

        g_baseARAMtoVM[g_nextARAMPageToCheck] = virtualPage;
        g_baseVMtoARAM[((virtualPage >> 10) & 0x7FFC) >> 2] = g_nextARAMPageToCheck << 12;

        g_totalAllocatedVM += 0x1000;
    }

    return TRUE;
}
#pragma optimize_for_size reset

u32 __VMTranslateVMPageToARAMPage(u32 virtualPage)
{
    u32 aramPage = g_baseVMtoARAM[(virtualPage >> 12) & 0x1FFF] & 0x7FFFFFFF;

    if (aramPage != 0)
    {
        return aramPage;
    }

    __VMMappingErrorAlert(virtualPage);
    return 0;
}

/**
 * Offset/Address/Size: 0x0 | 0x8025F97C | size: 0x20
 *
 * `#pragma optimize_for_size off` selects the neg/or/srwi nonzero-test
 * sequence; under the unit's -O4,s default MWCC emits the shorter
 * subic/subfe form, which diverges from the target.
 */
#pragma optimize_for_size off
BOOL __VMDoesMappingExist(u32 virtualPage)
{
    return (g_baseVMtoARAM[(virtualPage >> 12) & 0x1FFF] & 0x7FFFFFFF) != 0;
}
#pragma optimize_for_size reset

void __VMMappingErrorAlert(u32 virtualPage)
{
    char msg[0x408];
    sprintf(msg, "Virtual address (%x) has not been allocated. Call VMAlloc on virtual address ranges before using them.", virtualPage);
    PPCHalt();
}

void __VMSetARAMPageAsDirty(u32 virtualPage)
{
    g_baseVMtoARAM[(virtualPage >> 12) & 0x1FFF] |= 0x80000000;
}

BOOL __VMIsARAMPageDirty(u32 virtualPage)
{
    return g_baseVMtoARAM[(virtualPage >> 12) & 0x1FFF] >> 31;
}

static inline void ClearLUT(u32** base, s32 count)
{
    s32 i;
    s32 j;

    j = 0;
    for (i = 0; i < count; i++)
    {
        (*base)[j++] = 0;
        (*base)[j++] = 0;
        (*base)[j++] = 0;
        (*base)[j++] = 0;
        (*base)[j++] = 0;
        (*base)[j++] = 0;
        (*base)[j++] = 0;
        (*base)[j++] = 0;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8025FA00 | size: 0xA8
 * TODO: 87.4% match - the fill value 0 is rematerialized inside the loop (r0)
 * instead of held before the loop (target r4), which shifts the byte-offset
 * register (r4 vs target r5). Prologue also keeps the arena base in r3 rather
 * than copying it to r4 first.
 */
void __VMAllocVirtualToARAMLUT(void)
{
    g_baseVMtoARAM = (u32*)OSGetArenaLo();
    OSSetArenaLo((void*)((u8*)g_baseVMtoARAM + 0x8000));

    ClearLUT(&g_baseVMtoARAM, 0x400);
}

/**
 * Offset/Address/Size: 0x60 | 0x8025FAA8 | size: 0xA0
 * TODO: 90.9% match - the fill value 0 is rematerialized inside the loop (r0)
 * instead of held before the loop (target r4), which shifts the byte-offset
 * register (r4 vs target r5).
 */
void __VMAllocARAMToVirtualLUT(void)
{
    g_baseARAMtoVM = (u32*)OSGetArenaLo();
    OSSetArenaLo((void*)((u8*)g_baseARAMtoVM + 0x4000));

    ClearLUT(&g_baseARAMtoVM, 0x200);
}
