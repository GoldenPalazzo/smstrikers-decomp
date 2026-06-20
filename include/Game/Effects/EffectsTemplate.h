#ifndef _EFFECTSTEMPLATE_H_
#define _EFFECTSTEMPLATE_H_

#include "types.h"

#include "NL/nlColour.h"
#include "Game/Sys/simpleparser.h"

class SimpleParser;

struct fxRange
{
    /* 0x00 */ float base;
    /* 0x04 */ float range;
}; // size 0x8

enum eEffectsEmitter
{
    Emitter_Circle = 0,
    Emitter_Sphere = 1,
    Emitter_Spindle = 2,
    Emitter_Hemisphere = 3,
    Emitter_Num = 4,
};
enum eEffectsBlend
{
    EfBlend_Normal = 0,
    EfBlend_Additive = 1,
    EfBlend_Subtractive = 2,
    EfBlend_Num = 3,
};
enum eEffectsBillboard
{
    EfBill_Billboard = 0,
    EfBill_Groundboard = 1,
    EfBill_SoftwareControlled = 2,
    EfBill_Num = 3,
};

class EffectsTemplate
{
public:
    /* 0x00 */ u32 m_uHashID;
    /* 0x04 */ f32 m_fFountainLife;
    /* 0x08 */ fxRange m_rNumber;
    /* 0x10 */ fxRange m_rSizeBegin;
    /* 0x18 */ fxRange m_rSizeEnd;
    /* 0x20 */ fxRange m_rMass;
    /* 0x28 */ fxRange m_rParticleLife;
    /* 0x30 */ fxRange m_rRotation;
    /* 0x38 */ fxRange m_rRadius;
    /* 0x40 */ fxRange m_rInheritVelocity;
    /* 0x48 */ fxRange m_rVelocity;
    /* 0x50 */ fxRange m_rAcceleration;
    /* 0x58 */ fxRange m_rAngle;
    /* 0x60 */ fxRange m_rTilt;
    /* 0x68 */ bool m_bBounce;
    /* 0x69 */ bool m_bInFront;
    /* 0x6A */ bool m_bLocalSpace;
    /* 0x6B */ bool m_bLit;
    /* 0x6C */ eEffectsEmitter m_eEmitter;
    /* 0x70 */ eEffectsBlend m_eBlend;
    /* 0x74 */ eEffectsBillboard m_eBillboard;
    /* 0x78 */ u32 m_hTexture;
    /* 0x7C */ s32 m_nFrames;
    /* 0x80 */ fxRange m_rFPS;
    /* 0x88 */ nlColour m_cColour[25];
    /* 0xEC */ u32 m_uModelID;
    /* 0xF0 */ bool m_bMatchLifespan;
};

EffectsTemplate* fxGetTemplate(unsigned long);
bool fxUnloadTemplates();
bool fxLoadTemplateBundle(void*, unsigned long);
bool fxLoadTemplateBundle(const char*);
EffectsTemplate* parse_template(SimpleParser*, bool);
float RandomizedValue(float, float);
float RandomizedValue(float);

// void nlDLRingIsEnd<DLListEntry<ColourKey>>(DLListEntry<ColourKey>*, DLListEntry<ColourKey>*);
// void nlDLRingGetStart<DLListEntry<ColourKey>>(DLListEntry<ColourKey>*);
// void nlDLRingAddEnd<DLListEntry<ColourKey>>(DLListEntry<ColourKey>**, DLListEntry<ColourKey>*);
// void nlDLRingAddStart<DLListEntry<ColourKey>>(DLListEntry<ColourKey>**, DLListEntry<ColourKey>*);

// class nlAVLTree<unsigned long, EffectsTemplate*, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~nlAVLTree();
// };

// class AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*>>, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void AllocateEntry(void*, void*);
//     void CompareKey(void*, AVLTreeNode*);
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void DeleteValues();
//     void CastUp(AVLTreeNode*) const;
//     void PostorderTraversal(AVLTreeEntry<unsigned long, EffectsTemplate*>*, void (AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, EffectsTemplate*>*));
//     void DestroyTree(void (AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, EffectsTemplate*>*));
//     void Clear();
//     void ~AVLTreeBase();
//     void DeleteEntry(AVLTreeEntry<unsigned long, EffectsTemplate*>*);
//     void DeleteValue(AVLTreeEntry<unsigned long, EffectsTemplate*>*);
// };

// class NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*>>
// {
// public:
//     void Delete(AVLTreeEntry<unsigned long, EffectsTemplate*>*);
// };

// class DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey>>>
// {
// public:
//     void DeleteEntry(DLListEntry<ColourKey>*);
// };

// class nlWalkDLRing<DLListEntry<ColourKey>, DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey>>>>(DLListEntry<ColourKey>*, DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey>>>*, void (DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey>>>
// {
// public:
//     void *)(DLListEntry<ColourKey>*));
// };

// class nlWalkRing<DLListEntry<ColourKey>, DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey>>>>(DLListEntry<ColourKey>*, DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey>>>*, void (DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey>>>
// {
// public:
//     void *)(DLListEntry<ColourKey>*));
// };

#endif // _EFFECTSTEMPLATE_H_
