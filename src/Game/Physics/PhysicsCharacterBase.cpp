#include "Game/Physics/PhysicsCharacterBase.h"
#include "Game/PoseAccumulator.h"

void dBodySetUpdateMode(dxBody*, int, int);
dxJoint* dJointCreateCharacter(dxWorld*, dxJointGroup*);
void dJointSetCharacterNoMotionDirection(dxJoint*, float*);

// /**
//  * Offset/Address/Size: 0x68 | 0x801FF680 | size: 0x28
//  */
// void nlListAddStart<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*> >(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>**, ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*, ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801FF618 | size: 0x68
//  */
// void nlWalkList<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>, ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*> > > >(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*, ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*> > >*, void (ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*> > >::*)(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*))
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x801FF5A8 | size: 0x70
 */
PhysicsCharacterBase::PhysicsCapsuleBone::~PhysicsCapsuleBone()
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x801FF584 | size: 0x24
//  */
// void ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*> > >::DeleteEntry(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x801FF57C | size: 0x8
//  */
// void PhysicsCharacterBase::PhysicsSphereBone::GetObjectType() const
// {
// }

/**
 * Offset/Address/Size: 0x10 | 0x801FF50C | size: 0x70
 */
PhysicsCharacterBase::PhysicsSphereBone::~PhysicsSphereBone()
{
}

// /**
//  * Offset/Address/Size: 0x8 | 0x801FF504 | size: 0x8
//  */
// void PhysicsCharacterBase::PhysicsCapsuleBone::GetObjectType() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801FF4FC | size: 0x8
//  */
// void PhysicsCharacterBase::GetObjectType() const
// {
// }

/**
 * Offset/Address/Size: 0x9D0 | 0x801FF4CC | size: 0x30
 */
int PhysicsCharacterBase::GetNumBoneVolumePoints(bool includeEndpoints) const
{
    int count = 0;
    for (ListEntry<PhysicsBoneVolume*>* entry = m_BoneVolumes.m_Head; entry != NULL; entry = entry->next)
    {
        count++;
    }
    if (includeEndpoints)
    {
        return count * 2;
    }
    return count * 6;
}

/**
 * Offset/Address/Size: 0x864 | 0x801FF360 | size: 0x16C
 */
void PhysicsCharacterBase::GetBoneVolumePoints(nlVector3* points, bool includeEndpoints)
{
    for (ListEntry<PhysicsBoneVolume*>* entry = m_BoneVolumes.m_Head; entry != NULL; entry = entry->next)
    {
        PhysicsBoneVolume* boneVolume = entry->data;
        nlVector3 pos;
        boneVolume->m_pObject->GetPosition(&pos);

        if (includeEndpoints)
        {
            f32 radius = 1.0f;
            nlVec3Set(points[0], radius + pos.f.x, radius + pos.f.y, radius + pos.f.z);
            nlVec3Set(points[1], pos.f.x - radius, pos.f.y - radius, pos.f.z - radius);
            points += 2;
        }
        else
        {
            f32 radius = 1.0f;
            nlVec3Set(points[0], radius + pos.f.x, pos.f.y, pos.f.z);
            nlVec3Set(points[1], pos.f.x - radius, pos.f.y, pos.f.z);
            nlVec3Set(points[2], pos.f.x, radius + pos.f.y, pos.f.z);
            nlVec3Set(points[3], pos.f.x, pos.f.y - radius, pos.f.z);
            nlVec3Set(points[4], pos.f.x, pos.f.y, radius + pos.f.z);
            nlVec3Set(points[5], pos.f.x, pos.f.y, pos.f.z - radius);
            points += 6;
        }
    }
}

/**
 * Offset/Address/Size: 0x784 | 0x801FF280 | size: 0xE0
 */
PhysicsCharacterBase::PhysicsCharacterBase(CollisionSpace* collisionSpace, PhysicsWorld* physicsWorld, float centreOfMassHeight)
    : PhysicsCompositeObject(physicsWorld)
    , m_CentreOfMassHeight(centreOfMassHeight)
{
    dBodySetUpdateMode(m_bodyID, 1, 0);
    dBodySetGravityMode(m_bodyID, 0);

    m_CharMoveJoint = dJointCreateCharacter(physicsWorld->m_World, NULL);
    dJointAttach(m_CharMoveJoint, m_bodyID, NULL);

    float norm[4];
    norm[0] = 0.0f;
    norm[1] = 0.0f;
    norm[2] = 1.0f;
    dJointSetCharacterNoMotionDirection(m_CharMoveJoint, norm);

    dBodySetAutoDisableFlag(m_bodyID, 0);
}

/**
 * Offset/Address/Size: 0x69C | 0x801FF198 | size: 0xE8
 */
PhysicsCharacterBase::~PhysicsCharacterBase()
{
    dJointDestroy(m_CharMoveJoint);

    ListEntry<PhysicsBoneVolume*>* entry = m_BoneVolumes.m_Head;
    while (entry != NULL)
    {
        delete entry->data;
        entry = entry->next;
    }
}

/**
 * Offset/Address/Size: 0x654 | 0x801FF150 | size: 0x48
 */
void PhysicsCharacterBase::SetCharacterPosition(const nlVector3& pos)
{
    nlVector3 newPos;
    newPos.f.x = pos.f.x;
    newPos.f.y = pos.f.y;
    newPos.f.z = pos.f.z + m_CentreOfMassHeight;
    SetPosition(newPos, WORLD_COORDINATES);
}

/**
 * Offset/Address/Size: 0x524 | 0x801FF020 | size: 0x130
 * TODO: 98.55% match - ternary generates extra 'mr r0,r30' due to compiler codegen pattern
 */
bool PhysicsCharacterBase::BaseSetContactInfo(dContact* contact, PhysicsObject* other, bool first)
{
    int objectType = other->GetObjectType();

    if (first)
    {
        SetDefaultContactInfo(contact);

        PhysicsObject* owner = other;
        int otherType = other->GetObjectType();
        if (otherType == 0xE || otherType == 0xD)
        {
            PhysicsObject* parent;
            while ((parent = owner->m_parentObject) != NULL)
            {
                owner = parent;
            }
        }

        owner = owner->GetObjectType() == 0x8 ? owner : nullptr;
        if (owner)
        {
            contact->surface.mode &= ~(dContactBounce);
            contact->surface.bounce = 0.0f;
            contact->surface.bounce_vel = 0.0f;
        }
    }

    contact->surface.mode |= 0x18;
    contact->surface.soft_erp = 0.2f;
    contact->surface.soft_cfm = 1e-5f;
    contact->surface.mu = 0.0f;

    if (objectType == 0x6 || objectType == 0xB)
    {
        contact->surface.mode &= ~(dContactBounce);
        contact->surface.bounce = 0.0f;
        contact->surface.bounce_vel = 0.0f;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x518 | 0x801FF014 | size: 0xC
 */
void PhysicsCharacterBase::BasePreCollide()
{
    m_IsSupported = 0;
}

/**
 * Offset/Address/Size: 0x4E4 | 0x801FEFE0 | size: 0x34
 */
void PhysicsCharacterBase::ContainObject(PhysicsObject* obj)
{
    if (obj->m_parentObject != this)
    {
        m_SubObject.Attach(obj, this);
    }
}

/**
 * Offset/Address/Size: 0x4C0 | 0x801FEFBC | size: 0x24
 */
void PhysicsCharacterBase::ReleaseObject()
{
    m_SubObject.Release();
}

/**
 * Offset/Address/Size: 0x460 | 0x801FEF5C | size: 0x60
 */
void PhysicsCharacterBase::SetFacingDirection(unsigned short angle)
{
    float radians = (float)angle * (6.28318530718f / 65536.0f);
    nlMatrix4 matrix;
    nlMakeRotationMatrixZ(matrix, radians);
    SetRotation(matrix);
}

/**
 * Offset/Address/Size: 0x300 | 0x801FEDFC | size: 0x160
 */
void PhysicsCharacterBase::UpdatePose(cPoseAccumulator* pose, float heightOffset)
{
    for (ListEntry<PhysicsBoneVolume*>* entry = m_BoneVolumes.m_Head; entry != NULL; entry = entry->next)
    {
        PhysicsBoneVolume* boneVolume = entry->data;

        // Store previous position
        nlVector3& pos = boneVolume->m_pObject->GetPosition();
        boneVolume->m_PrevPosition = pos;

        // Get bone matrix from pose accumulator and multiply with transform
        nlMatrix4 worldMatrix = pose->GetNodeMatrix(boneVolume->m_BoneIndex);
        nlMultMatrices(worldMatrix, boneVolume->m_Transform, pose->GetNodeMatrix(boneVolume->m_BoneIndex));

        // Adjust Z position for center of mass
        worldMatrix.f.m43 -= m_CentreOfMassHeight;
        worldMatrix.f.m43 += heightOffset;

        // Apply the transform
        AdjustTransform(boneVolume->m_TransformHandle, worldMatrix);
    }
}

struct AddBoneVolumesElement
{
    nlMatrix4 matLocalToParent;
    s8 szName[32];
    u32 uHashID;
    s8 szParentName[32];
    u32 uParentHashID;
    u32 uPrimitiveType;
    f32 fWidth;
    f32 fLength;
    f32 fHeight;
    f32 fRadius;
    u32 uReserved;
};

extern void* __vt__Q220PhysicsCharacterBase17PhysicsSphereBone[];
extern void* __vt__Q220PhysicsCharacterBase18PhysicsCapsuleBone[];
extern "C" void __ct__13PhysicsSphereFP14CollisionSpaceP12PhysicsWorldf(PhysicsSphere*, CollisionSpace*, PhysicsWorld*, float);
extern "C" void __ct__14PhysicsCapsuleFP14CollisionSpaceP12PhysicsWorldff(PhysicsCapsule*, CollisionSpace*, PhysicsWorld*, float, float);

/**
 * Offset/Address/Size: 0x80 | 0x801FEB7C | size: 0x280
 * TODO: 95.34% match - remaining 100 diffs are register-only (pose->r31 vs target r21),
 * caused by -inline deferred vs -inline auto register allocator difference.
 */
void PhysicsCharacterBase::AddBoneVolumes(PhysicsWorld* physicsWorld, CollisionSpace* collisionSpace, cPoseAccumulator* pose, const CharacterPhysicsData* physicsData, unsigned long category, unsigned long collideMask)
{
    ListEntry<PhysicsBoneVolume*>** pTail = &m_BoneVolumes.m_Tail;
    ListEntry<PhysicsBoneVolume*>** pHead = &m_BoneVolumes.m_Head;
    unsigned long i = 0;
    unsigned long offset = 0;
    u8* elements = *(u8**)((u8*)physicsData + 8);
    unsigned long count = *(u32*)((u8*)physicsData + 4);

    while (i < count)
    {
        AddBoneVolumesElement* element = (AddBoneVolumesElement*)(elements + offset);
        PhysicsObject* obj = NULL;

        switch (element->uPrimitiveType)
        {
        case 1:
        {
            PhysicsSphereBone* sphere = (PhysicsSphereBone*)nlMalloc(0x30, 8, false);
            if (sphere != NULL)
            {
                __ct__13PhysicsSphereFP14CollisionSpaceP12PhysicsWorldf((PhysicsSphere*)sphere, collisionSpace, physicsWorld, element->fRadius);
                *(void**)sphere = __vt__Q220PhysicsCharacterBase17PhysicsSphereBone;
                sphere->m_boneVolume = NULL;
            }
            obj = sphere;
            obj->GetObjectType();
            break;
        }

        case 2:
        {
            PhysicsCapsuleBone* capsule = (PhysicsCapsuleBone*)nlMalloc(0x30, 8, false);
            if (capsule != NULL)
            {
                __ct__14PhysicsCapsuleFP14CollisionSpaceP12PhysicsWorldff((PhysicsCapsule*)capsule, collisionSpace, physicsWorld, element->fRadius, element->fHeight);
                *(void**)capsule = __vt__Q220PhysicsCharacterBase18PhysicsCapsuleBone;
                capsule->m_boneVolume = NULL;
            }
            obj = capsule;
            break;
        }

        default:
            break;
        }

        obj->SetCategory(category);
        obj->SetCollide(collideMask);

        unsigned int boneIndex = pose->m_BaseSHierarchy->GetNodeIndexByID(element->uParentHashID);
        int transformHandle = AddObject(obj);
        PhysicsBoneID boneID = ResolvePhysicsBoneIDFromName((const char*)element->szName);

        PhysicsBoneVolume* boneVolume = (PhysicsBoneVolume*)nlMalloc(sizeof(PhysicsBoneVolume), 8, false);
        if (boneVolume != NULL)
        {
            boneVolume->m_pObject = obj;
            boneVolume->m_BoneIndex = boneIndex;
            boneVolume->m_Transform = element->matLocalToParent;
            boneVolume->m_TransformHandle = transformHandle;
            boneVolume->m_ID = boneID;
        }

        ListEntry<PhysicsBoneVolume*>* listNode = (ListEntry<PhysicsBoneVolume*>*)nlMalloc(8, 8, false);
        if (listNode != NULL)
        {
            listNode->next = NULL;
            listNode->data = boneVolume;
        }

        nlListAddStart(pHead, listNode, pTail);

        switch (element->uPrimitiveType)
        {
        case 1:
            ((PhysicsSphereBone*)obj)->m_boneVolume = boneVolume;
            break;

        case 2:
            ((PhysicsCapsuleBone*)obj)->m_boneVolume = boneVolume;
            break;

        default:
            break;
        }

        offset += 0xA0;
        i++;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801FEAFC | size: 0x80
 */
PhysicsBoneID PhysicsCharacterBase::GetBoneIDForSubObject(const PhysicsObject* obj) const
{
    switch (obj->GetObjectType())
    {
    case 0xD:
    {
        const PhysicsSphereBone* bone = static_cast<const PhysicsSphereBone*>(obj);
        if (bone->m_boneVolume != NULL)
        {
            return bone->m_boneVolume->m_ID;
        }
        break;
    }
    case 0xE:
    {
        const PhysicsCapsuleBone* bone = static_cast<const PhysicsCapsuleBone*>(obj);
        if (bone->m_boneVolume != NULL)
        {
            return bone->m_boneVolume->m_ID;
        }
        break;
    }
    }
    return PHYSBONE_SINGLE_CAPSULE;
}

// // )))))))))))))))))))
// /**
//  * Offset/Address/Size: 0x0 | 0x801FEAFC | size: 0x80
//  */
// void PhysicsCharacterBase::GetBoneIDForSubObject(const PhysicsObject*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x801FEB7C | size: 0x280
//  */
// void PhysicsCharacterBase::AddBoneVolumes(PhysicsWorld*, CollisionSpace*, cPoseAccumulator*, const CharacterPhysicsData*, unsigned long,
//     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x300 | 0x801FEDFC | size: 0x160
//  */
// void PhysicsCharacterBase::UpdatePose(cPoseAccumulator*, float)
// {
// }

// /**
//  * Offset/Address/Size: 0x460 | 0x801FEF5C | size: 0x60
//  */
// void PhysicsCharacterBase::SetFacingDirection(unsigned short)
// {
// }

// /**
//  * Offset/Address/Size: 0x4C0 | 0x801FEFBC | size: 0x24
//  */
// void PhysicsCharacterBase::ReleaseObject()
// {
// }

// /**
//  * Offset/Address/Size: 0x4E4 | 0x801FEFE0 | size: 0x34
//  */
// void PhysicsCharacterBase::ContainObject(PhysicsObject*)
// {
// }

// /**
//  * Offset/Address/Size: 0x518 | 0x801FF014 | size: 0xC
//  */
// void PhysicsCharacterBase::BasePreCollide()
// {
// }

// /**
//  * Offset/Address/Size: 0x524 | 0x801FF020 | size: 0x130
//  */
// bool PhysicsCharacterBase::BaseSetContactInfo(dContact*, PhysicsObject*, bool)
// {
//     return false;
// }

// /**
//  * Offset/Address/Size: 0x654 | 0x801FF150 | size: 0x48
//  */
// void PhysicsCharacterBase::SetCharacterPosition(const nlVector3&)
// {
// }

// /**
//  * Offset/Address/Size: 0x69C | 0x801FF198 | size: 0xE8
//  */
// PhysicsCharacterBase::~PhysicsCharacterBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x784 | 0x801FF280 | size: 0xE0
//  */
// PhysicsCharacterBase::PhysicsCharacterBase(CollisionSpace* collisionSpace, PhysicsWorld* physicsWorld, float)
//     : PhysicsCompositeObject(physicsWorld)
// {
// }

// /**
//  * Offset/Address/Size: 0x864 | 0x801FF360 | size: 0x16C
//  */
// void PhysicsCharacterBase::GetBoneVolumePoints(nlVector3*, bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x9D0 | 0x801FF4CC | size: 0x30
//  */
// void PhysicsCharacterBase::GetNumBoneVolumePoints(bool) const
// {
// }

// // /**
// //  * Offset/Address/Size: 0x0 | 0x801FF4FC | size: 0x8
// //  */
// // int PhysicsCharacterBase::GetObjectType() const
// // {
// //     return 0x08;
// // }

// /**
//  * Offset/Address/Size: 0x8 | 0x801FF504 | size: 0x8
//  */
// // void PhysicsCharacterBase::PhysicsCapsuleBone::GetObjectType() const
// // {
// // }

// /**
//  * Offset/Address/Size: 0x10 | 0x801FF50C | size: 0x70
//  */
// // void PhysicsCharacterBase::PhysicsSphereBone::~PhysicsSphereBone()
// // {
// // }

// /**
//  * Offset/Address/Size: 0x80 | 0x801FF57C | size: 0x8
//  */
// // void PhysicsCharacterBase::PhysicsSphereBone::GetObjectType() const
// // {
// // }

// /**
//  * Offset/Address/Size: 0x0 | 0x801FF584 | size: 0x24
//  */
// // void ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>>>::
// //     DeleteEntry(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*)
// // {
// // }

// /**
//  * Offset/Address/Size: 0x0 | 0x801FF5A8 | size: 0x70
//  */
// // void PhysicsCharacterBase::PhysicsCapsuleBone::~PhysicsCapsuleBone()
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x0 | 0x801FF618 | size: 0x68
// //  */
// // void nlWalkList<
// //     ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>,
// //     ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>>>>(
// //     ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*,
// //     ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>>>*,
// //     void (ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>>>::*)(
// //         ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*))
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x68 | 0x801FF680 | size: 0x28
// //  */
// // void nlListAddStart<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>>(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>**,
// //                                                                          ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*,
// //                                                                          ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>**)
// // {
// // }
