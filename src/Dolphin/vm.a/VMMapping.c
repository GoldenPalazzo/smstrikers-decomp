#include <stdio.h>

#include <dolphin/PPCArch.h>
#include <dolphin/types.h>

static u32* g_baseARAMtoVM;
static u32* g_baseVMtoARAM;
static u32 g_totalAllocatedVM;
static u32 g_nextARAMPageToCheck_233;

static const char lbl_80307810[] = "Virtual address (%x) has not been allocated. Call VMAlloc on virtual address ranges before using them.";

void* OSGetArenaLo(void);
void OSSetArenaLo(void* newLo);
u32 VMGetARAMBase(void);
u32 VMGetARAMSize(void);
void __VMMappingErrorAlert(u32 virtualPage);

static inline u32 VMToARAMOffset(u32 virtualAddr)
{
    return (virtualAddr >> 10) & 0x1FFC;
}

BOOL VMAlloc(u32 address, u32 size)
{
    u32 i;
    u32 startAramPage = VMGetARAMBase() >> 12;
    u32 endAramPage = startAramPage + (VMGetARAMSize() >> 12);

    if (g_nextARAMPageToCheck_233 < startAramPage)
    {
        g_nextARAMPageToCheck_233 = startAramPage;
    }

    if ((g_totalAllocatedVM + size) > VMGetARAMSize())
    {
        return FALSE;
    }

    /**
     * Offset/Address/Size: 0x0 | 0x8025F83C | size: 0x100
     * TODO: 99.45% match - remaining i-diffs are on g_nextARAMPageToCheck$233
     * SDA relocation references only.
     */
    for (i = 0; i < size; i += 0x1000)
    {
        u32 virtualPage = address + i;

        while (TRUE)
        {
            u32 next = g_nextARAMPageToCheck_233 + 1;
            g_nextARAMPageToCheck_233 = next;
            if (next >= endAramPage)
            {
                g_nextARAMPageToCheck_233 = startAramPage;
            }

            if (g_baseARAMtoVM[g_nextARAMPageToCheck_233] == 0)
            {
                break;
            }
        }

        g_baseARAMtoVM[g_nextARAMPageToCheck_233] = virtualPage;
        g_baseVMtoARAM[((virtualPage >> 10) & 0x7FFC) >> 2] = g_nextARAMPageToCheck_233 << 12;

        g_totalAllocatedVM += 0x1000;
    }

    return TRUE;
}

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

BOOL __VMDoesMappingExist(u32 virtualPage)
{
    return (g_baseVMtoARAM[(virtualPage >> 12) & 0x1FFF] & 0x7FFFFFFF) != 0;
}

void __VMMappingErrorAlert(u32 virtualPage)
{
    char msg[0x408];
    sprintf(msg, lbl_80307810, virtualPage);
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

void __VMAllocVirtualToARAMLUT(void)
{
    s32 i;
    s32 j;

    g_baseVMtoARAM = (u32*)OSGetArenaLo();
    OSSetArenaLo((void*)((u8*)g_baseVMtoARAM + 0x8000));

    j = 0;
    for (i = 0; i < 0x400; i++)
    {
        g_baseVMtoARAM[j++] = 0;
        g_baseVMtoARAM[j++] = 0;
        g_baseVMtoARAM[j++] = 0;
        g_baseVMtoARAM[j++] = 0;
        g_baseVMtoARAM[j++] = 0;
        g_baseVMtoARAM[j++] = 0;
        g_baseVMtoARAM[j++] = 0;
        g_baseVMtoARAM[j++] = 0;
    }
}

/**
 * Offset/Address/Size: 0x60 | 0x8025FAA8 | size: 0xA0
 * TODO: 88.9% match - register allocation differs: compiler puts byte offset
 * in r3 (target: r5) and zero value inside loop as r0 (target: r4 before loop).
 * Requires mwcc_242_81 with -O4,s. vm.a uses a different MWCC version than the
 * rest of DolphinLib (stwu-first prologue confirms mwcc_242_81).
 */
void __VMAllocARAMToVirtualLUT(void)
{
    u32 i;
    u32 j;

    g_baseARAMtoVM = (u32*)OSGetArenaLo();
    OSSetArenaLo((void*)((u8*)g_baseARAMtoVM + 0x4000));

    j = 0;
    for (i = 0; i < 0x200; i++)
    {
        g_baseARAMtoVM[j++] = 0;
        g_baseARAMtoVM[j++] = 0;
        g_baseARAMtoVM[j++] = 0;
        g_baseARAMtoVM[j++] = 0;
        g_baseARAMtoVM[j++] = 0;
        g_baseARAMtoVM[j++] = 0;
        g_baseARAMtoVM[j++] = 0;
        g_baseARAMtoVM[j++] = 0;
    }
}
