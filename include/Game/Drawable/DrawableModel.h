#ifndef _DRAWABLEMODEL_H_
#define _DRAWABLEMODEL_H_

#include "Game/Drawable/DrawableObj.h"

#include "NL/nlMath.h"
#include "NL/nlAVLTreeSlotPool.h"
#include "NL/gl/glView.h"
#include "NL/gl/glModel.h"

class glModel;
class glModelPacket;
class GLShadowVolume;
struct BallShadowParams
{
    float fReferenceHeight;
    float fRadius0;
    float fRadius1;
    int nAlpha0;
    int nAlpha1;
    nlColour colour;
};

void CleanBoundingBoxCache();
void RenderBoundingBox(const glModel*, const nlMatrix4&);
float GetCoPlanar0Z();
void SetCoPlanarZ(float);
float GetCoPlanarZ();
void SetPlanarShadowOpacity(float);
float GetPlanarShadowOpacity();
void DrawPlanarShadow(const glModel*, const nlMatrix4&, float, bool, bool, bool, unsigned long);
void DrawCoPlanarReference(eGLView, const glModel&, const nlMatrix4&, unsigned long);
void GetShadowBoundingSquare(const glModel*, const nlMatrix4&, float&, float&, float&, float&, unsigned long);
void GetAABBDimensions(const glModel*, AABBDimensions&, unsigned long);
void Fresnelify(glModelPacket*, eGLView);
void DrawBallShadow(const nlVector3&, const BallShadowParams&, bool);

class DrawableModel : public DrawableObject
{
public:
    DrawableModel(const DrawableModel& other)
        : DrawableObject(other)
        , m_pModel(other.m_pModel)
        , m_pShadowVolume(other.m_pShadowVolume)
        , m_bVertexAnimated(other.m_bVertexAnimated)
        , m_bUnknownA5(other.m_bUnknownA5)
        , pAABBDimensions(other.pAABBDimensions) {
        };
    /* 0x04 */ virtual ~DrawableModel() { };
    /* 0x08 */ virtual DrawableObject* Clone() const;
    /* 0x0C */ virtual void Draw();
    /* 0x10 */ virtual void DrawPlanarShadow();
    /* 0x14 */ virtual bool IsDrawableModel() { return true; };
    /* 0x18 */ virtual DrawableModel* AsDrawableModel() { return this; };
    /* 0x1C */ virtual void GetAABBDimensions(AABBDimensions&, bool) const;

    void DrawModel(const nlMatrix4&);

    inline glModel* GetModel() const { return m_pModel; }

    static unsigned char sbBallShadowDisabled;

    /* 0x9C */ glModel* m_pModel;
    /* 0xA0 */ GLShadowVolume* m_pShadowVolume;
    /* 0xA4 */ bool m_bVertexAnimated;
    /* 0xA5 */ bool m_bUnknownA5;
    /* 0xA8 */ AABBDimensions* pAABBDimensions;
};

class DrawableShadow : public DrawableObject
{
public:
    /* 0x04 */ virtual ~DrawableShadow() { };
    /* 0x0C */ virtual void Draw();

    /* 0x9C */ glModel* m_pModel;
};

// class nlAVLTreeSlotPool<unsigned long, AABBDimensions, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~nlAVLTreeSlotPool();
// };

// class AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void Clear();
//     void DestroyTree(void (AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, AABBDimensions>*));
//     void PostorderTraversal(AVLTreeEntry<unsigned long, AABBDimensions>*, void (AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, AABBDimensions>*));
//     void CastUp(AVLTreeNode*) const;
//     void DeleteEntry(AVLTreeEntry<unsigned long, AABBDimensions>*);
//     void ~AVLTreeBase();
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void CompareKey(void*, AVLTreeNode*);
//     void AllocateEntry(void*, void*);
// };

#endif // _DRAWABLEMODEL_H_
