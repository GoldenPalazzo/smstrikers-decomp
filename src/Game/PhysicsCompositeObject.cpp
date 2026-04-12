#include "Game/Physics/PhysicsCompositeObject.h"

#include "Game/Physics/PhysicsWorld.h"
#include "Game/Physics/PhysicsObject.h"

#include "NL/nlMemory.h"

/**
 * Offset/Address/Size: 0x0 | 0x801FF6A8 | size: 0xA8
 */
void PhysicsCompositeObject::AdjustTransform(int i, nlMatrix4& m)
{
    u32 idx;
    DLListEntry<PhysicsTransform*>* head;
    DLListEntry<PhysicsTransform*>* pStart = nlDLRingGetStart<DLListEntry<PhysicsTransform*> >(m_Components.m_Head);

    head = m_Components.m_Head;
    idx = 0;
    DLListEntry<PhysicsTransform*>* current = pStart;

    PhysicsTransform* transformObj;

    for (; idx < (u32)i; idx++)
    {
        if (nlDLRingIsEnd<DLListEntry<PhysicsTransform*> >(head, current))
        {
            transformObj = 0;
            goto call_transform;
        }

        if (nlDLRingIsEnd<DLListEntry<PhysicsTransform*> >(head, current) || current == 0)
        {
            current = 0;
        }
        else
        {
            current = current->m_next;
        }
    }

    transformObj = (PhysicsTransform*)current->m_data;
call_transform:
    transformObj->SetSubObjectTransform(m, PhysicsObject::RELATIVE_TO_PARENT);
}

/**
 * Offset/Address/Size: 0xA8 | 0x801FF750 | size: 0xBC
 */
int PhysicsCompositeObject::AddObject(PhysicsObject* object)
{
    object->MakeStatic();
    PhysicsTransform* transform = new (nlMalloc(0x30, 8, false)) PhysicsTransform();

    transform->Attach(object, this);

    DLListEntry<PhysicsTransform*>* entry = (DLListEntry<PhysicsTransform*>*)nlMalloc(0xC, 8, 0);

    if (entry != nullptr)
    {
        entry->m_next = nullptr;
        entry->m_prev = nullptr;
        entry->m_data = transform;
    }

    nlDLRingAddEnd<DLListEntry<PhysicsTransform*> >(&m_Components.m_Head, entry);

    numComponents++;

    return numComponents - 1;
}

/**
 * Offset/Address/Size: 0x164 | 0x801FF80C | size: 0x110
 * TODO: 96.54% match - remaining mismatch is in auto-generated
 * m_Components cleanup (nlWalkDLRing vs nlWalkRing call shape/register order).
 */
PhysicsCompositeObject::~PhysicsCompositeObject()
{
    DLListEntry<PhysicsTransform*>* start;
    DLListEntry<PhysicsTransform*>* head;
    DLListEntry<PhysicsTransform*>* current;

    start = nlDLRingGetStart<DLListEntry<PhysicsTransform*> >(m_Components.m_Head);
    head = m_Components.m_Head;
    current = start;

    while (current != NULL)
    {
        PhysicsTransform* physObj = (PhysicsTransform*)current->m_data;

        physObj->m_bodyID = NULL;
        delete physObj;

        if (nlDLRingIsEnd<DLListEntry<PhysicsTransform*> >(head, current) || current == NULL)
        {
            current = NULL;
        }
        else
        {
            current = current->m_next;
        }
    }
}

/**
 * Offset/Address/Size: 0x274 | 0x801FF91C | size: 0x54
 */
PhysicsCompositeObject::PhysicsCompositeObject(PhysicsWorld* physicsWorld)
    : PhysicsObject(physicsWorld)
{
    m_Components.m_Head = nullptr;
    numComponents = 0;
    dBodySetData(m_bodyID, this);
}

/**
 * Offset/Address/Size: 0x0 | 0x801FF970 | size: 0x8
 */
// int PhysicsCompositeObject::GetObjectType() const
// {
//     return 0x9;
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801FF978 | size: 0x24
//  */
// void DLListContainerBase<PhysicsTransform*, NewAdapter<DLListEntry<PhysicsTransform*>>>::DeleteEntry(DLListEntry<PhysicsTransform*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x801FF99C | size: 0x3C
 * TODO: 96.00% match - prologue scheduling mismatch remains.
 * Target orders `lwz r7, 0(r5)` before saving LR; current MWCC output saves LR first.
 */
typedef DLListEntry<PhysicsTransform*> Entry;
typedef DLListContainerBase<PhysicsTransform*, NewAdapter<DLListEntry<PhysicsTransform*> > > Container;

template void nlWalkDLRing<Entry, Container>(
    Entry* head,
    Container* callback,
    void (Container::*callbackFunc)(Entry*));

// /**
//  * Offset/Address/Size: 0x3C | 0x801FF9D8 | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<PhysicsTransform*>>(DLListEntry<PhysicsTransform*>*, DLListEntry<PhysicsTransform*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x5C | 0x801FF9F8 | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<PhysicsTransform*>>(DLListEntry<PhysicsTransform*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x74 | 0x801FFA10 | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<PhysicsTransform*>>(DLListEntry<PhysicsTransform*>**, DLListEntry<PhysicsTransform*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xB0 | 0x801FFA4C | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<PhysicsTransform*>>(DLListEntry<PhysicsTransform*>**, DLListEntry<PhysicsTransform*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801FFA84 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<PhysicsTransform*>, DLListContainerBase<PhysicsTransform*,
// NewAdapter<DLListEntry<PhysicsTransform*>>>>(DLListEntry<PhysicsTransform*>*, DLListContainerBase<PhysicsTransform*,
// NewAdapter<DLListEntry<PhysicsTransform*>>>*, void (DLListContainerBase<PhysicsTransform*,
// NewAdapter<DLListEntry<PhysicsTransform*>>>::*)(DLListEntry<PhysicsTransform*>*))
// {
// }
