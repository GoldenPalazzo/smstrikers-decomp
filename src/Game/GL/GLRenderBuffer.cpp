#include "Game/GL/GLRenderBuffer.h"

#include "NL/nlMemory.h"

bool GLRenderBuffer::m_bInstance = false;

u32 glRenderBuffer[4];

// /**
//  * Offset/Address/Size: 0x60 | 0x801E81CC | size: 0x58
//  */
// void 0x8028D548..0x8028D54C | size : 0x4
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801E816C | size: 0x60
//  */
// void nlWalkRing<DLListEntry<GLDrawableData*>, DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > > >(DLListEntry<GLDrawableData*>*, DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > >*, void (DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > >::*)(DLListEntry<GLDrawableData*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x5C | 0x801E8154 | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<GLDrawableData*> >(DLListEntry<GLDrawableData*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x3C | 0x801E8134 | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<GLDrawableData*> >(DLListEntry<GLDrawableData*>*, DLListEntry<GLDrawableData*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x801E80F8 | size: 0x3C
 * TODO: 96% match - stw LR save scheduling differs by one slot
 * (target emits first lwz from callbackFunc before stw r0,0x24(r1)).
 */
template void nlWalkDLRing<DLListEntry<GLDrawableData*>, DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > > >(
    DLListEntry<GLDrawableData*>* head,
    DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > >* callback,
    void (DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > >::*callbackFunc)(DLListEntry<GLDrawableData*>*));

// /**
//  * Offset/Address/Size: 0x0 | 0x801E80D4 | size: 0x24
//  */
// void DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > >::DeleteEntry(DLListEntry<GLDrawableData*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x801E7FB0 | size: 0x124
 */
GLRenderBuffer::~GLRenderBuffer()
{
    DLListEntry<GLDrawableData*>* head;
    DLListEntry<GLDrawableData*>* current = nlDLRingGetStart<DLListEntry<GLDrawableData*> >(m_drawableData.m_Head);
    head = m_drawableData.m_Head;

    while (current != 0)
    {
        nlFree(current->m_data);

        if (nlDLRingIsEnd<DLListEntry<GLDrawableData*> >(head, current) || current == 0)
        {
            current = 0;
        }
        else
        {
            current = current->m_next;
        }
    }

    typedef void (*WalkFn)(DLListEntry<GLDrawableData*>*, DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > >*, void (DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > >::*)(DLListEntry<GLDrawableData*>*));
    void (DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > >::*func)(DLListEntry<GLDrawableData*>*) = &DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > >::DeleteEntry;
    WalkFn walk = &nlWalkDLRing<DLListEntry<GLDrawableData*>, DLListContainerBase<GLDrawableData*, NewAdapter<DLListEntry<GLDrawableData*> > > >;
    walk(m_drawableData.m_Head, &m_drawableData, func);

    m_drawableData.m_Head = 0;
}
