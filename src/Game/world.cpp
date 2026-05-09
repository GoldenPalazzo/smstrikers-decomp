#define NL_AVLTREE_DECLARE_ONLY
#define NL_AVLTREEBASE_DECLARE_ONLY
#include "Game/World.h"
#include "Game/LightObject.h"

#include "string.h"

#include "NL/nlString.h"
#include "NL/nlPrint.h"
#include "NL/nlDebug.h"
#include "NL/gl/gl.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glState.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glFont.h"
#include "NL/gl/glView.h"
#include "NL/gl/glUserData.h"
#include "NL/gl/glTexture.h"
#include "NL/glx/glxTexture.h"
#include "Game/Sys/debug.h"
#include "Game/Drawable/DrawableObj.h"
#include "Game/Drawable/DrawableModel.h"
#include "Game/Drawable/DrawableSkinModel.h"
#include "Game/Drawable/DrawableTmModel.h"
#include "Game/GL/GLInventory.h"
#include "Game/Effects/EmissionManager.h"

#include "NL/gl/glLightUserData.h"
#include "NL/nlFile.h"
#include "Game/SAnim.h"
#include "Game/Physics/CharacterPhysicsElement.h"
#include "ctype_api.h"

// .sdata (initialized) — order matches target world.s
static unsigned char g_bClipToFrustum = 1;
u32 World::m_uCurrentFrameCount = 0xFFFFFFFF;
float g_fTransAdjustOccluded = 1.0f;
float g_fTransAdjustNotOccluded = 0.125f;
float g_fExponentScale = 128.0f;
float g_fExponentBase = 8.0f;

// .sbss (uninitialized) — order matches target world.s
static unsigned char g_bDrawBoundingSphere;
static unsigned char g_bFreezeFrustum;
static unsigned char g_bDrawCullingInfo;
static unsigned char g_bDebugEqualsSide;
static unsigned char g_bDebugEqualsEnd;
static unsigned long WhiteTexture = glGetTexture("global/white50percent");
static unsigned long BallModelID = nlStringHash("gameplay/ball");
static unsigned long HammerModelID = nlStringHash("gameplay/hammer");
unsigned long SpecificModelID = nlStringLowerHash("The_Palace/Pod_Metal_42");
static float g_fTransMinimum = 0.0f;
bool World::sbIsHyperShootToScoreRenderingEnabled;
unsigned char sbShowPositiveXNetDuringHyperStrike__5World;
unsigned char sbStadiumRenderingDisabled__5World;
unsigned char sbSkyboxRenderingDisabled__5World;
static unsigned char g_bFreezeSideCam;
static unsigned char g_bFreezeEndCam;
static unsigned char sbAllObjectsCanBeTransparent;

static LightObject fxLightObjects[4];
static nlVector3 vLightDirection = { { 0.0f, 0.0f, -1.0f } };

/**
 * Offset/Address/Size: 0x0 | 0x80194CC4 | size: 0x5C
 */
int World::CompareNameToGenericName(const char* str1, const char* str2)
{
    size_t sVar1 = strlen(str2);
    return nlStrNCmp<char>(str1 + *(int*)((u8*)this + 0x120), str2, sVar1);
}

/**
 * Offset/Address/Size: 0x5C | 0x80194D20 | size: 0x44
 * TODO: 99.7% match - r3 vs r31 base register on lwz r5,0x120; peephole off prevents scheduling hoist but forces r31 usage
 */
#pragma push
#pragma peephole off
unsigned long World::GetHashIdForGenericName(const char* name) const
{
    int len = m_WorldNameLength;
    nlStrNCpy<char>((char*)this + len + 0xe0, name, (unsigned long)(0x40 - len));
    return nlStringLowerHash((const char*)this + 0xe0);
}
#pragma pop

/**
 * Offset/Address/Size: 0xA0 | 0x80194D64 | size: 0x1F0
 */
LightObject* World::GetShadowLight(const nlVector3& vPosition, float)
{
    u32* pStack;
    LightObject* pClosest = NULL;
    float fDistance = 999999.0f;
    AVLTreeEntry<unsigned long, LightObject*>* pNode;
    LightObject* pLight;

    pStack = (u32*)nlMalloc(8, 8, false);
    if (pStack != NULL)
    {
        u32 numElements = m_lightMap.m_NumElements;
        pNode = m_lightMap.m_Root;
        pStack[0] = (u32)nlMalloc((numElements + 1) * 4, 8, false);
        pStack[1] = 0;

        if (pNode != NULL)
        {
            while (pNode->node.left != NULL)
            {
                ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]] = pNode;
                pStack[1]++;
                pNode = (AVLTreeEntry<unsigned long, LightObject*>*)pNode->node.left;
            }
            ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]] = pNode;
            pStack[1]++;
        }
    }

    while (pStack[1] != 0)
    {
        pNode = ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1] - 1];
        pLight = pNode->value;

        if (pLight->m_emitFlags & 1)
        {
            float dx, dy, dz;
            dy = vPosition.f.y - pLight->m_worldPosition.f.y;
            dx = vPosition.f.x - pLight->m_worldPosition.f.x;
            dz = vPosition.f.z - pLight->m_worldPosition.f.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            if (distSq < fDistance)
            {
                fDistance = distSq;
                pClosest = pLight;
            }
        }

        pStack[1]--;

        {
            AVLTreeEntry<unsigned long, LightObject*>* pChild = (AVLTreeEntry<unsigned long, LightObject*>*)((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]]->node.right;
            if (pChild == NULL)
            {
                continue;
            }

            while (pChild->node.left != NULL)
            {
                ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]] = pChild;
                pStack[1]++;
                pChild = (AVLTreeEntry<unsigned long, LightObject*>*)pChild->node.left;
            }
            ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]] = pChild;
            pStack[1]++;
        }
    }

    if (pStack != NULL)
    {
        delete[] (u8*)pStack[0];
        delete (u8*)pStack;
    }
    return pClosest;
}

/**
 * Offset/Address/Size: 0x290 | 0x80194F54 | size: 0x8C
 */
bool World::AddDrawableObject(unsigned long uHashID, DrawableObject* pDrawableObject)
{
    AVLTreeNode* pExistingNode;
    AVLTreeNode** ppRoot = (AVLTreeNode**)&m_drawableMap.m_Root;

    m_drawableMap.AddAVLNode(ppRoot, &uHashID, &pDrawableObject, &pExistingNode, m_drawableMap.m_NumElements);

    DrawableObject** ppValue = nullptr;
    if (pExistingNode == nullptr)
    {
        m_drawableMap.m_NumElements++;
        ppValue = nullptr;
    }
    else
    {
        ppValue = (DrawableObject**)((u8*)pExistingNode + 0x10);
    }

    if (ppValue == nullptr)
    {
        pDrawableObject->m_pWorldContext = this;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Helper struct for inlining FindGet with bool return to match target assembly.
 * The target uses a bool found flag pattern (li r0,1 / li r0,0 / clrlwi.)
 * which the native AVLTreeBase::FindGet (returning ValueType*) does not produce.
 */
struct HelperMapFindHelper
{
    char pad[0x8];
    AVLTreeEntry<unsigned long, HelperObject*>* m_Root;

    inline bool FindGet(unsigned long key, HelperObject*** foundValue) const
    {
        AVLTreeEntry<unsigned long, HelperObject*>* node = m_Root;
        while (node != NULL)
        {
            int cmpResult;
            if (key == node->key)
                cmpResult = 0;
            else if (key < node->key)
                cmpResult = -1;
            else
                cmpResult = 1;
            if (cmpResult == 0)
            {
                if (foundValue != NULL)
                    *foundValue = &node->value;
                return true;
            }
            else
            {
                if (cmpResult < 0)
                    node = (AVLTreeEntry<unsigned long, HelperObject*>*)node->node.left;
                else
                    node = (AVLTreeEntry<unsigned long, HelperObject*>*)node->node.right;
            }
        }
        return false;
    }
};

/**
 * Offset/Address/Size: 0x31C | 0x80194FE0 | size: 0x8C
 */
HelperObject* World::FindHelperObject(unsigned long uHashID)
{
    HelperObject** foundValue;
    bool found = ((HelperMapFindHelper*)&m_helperMap)->FindGet(uHashID, &foundValue);
    if (found)
        return *foundValue;
    return nullptr;
}

/**
 * Helper struct for inlining FindGet with bool return to match target assembly.
 * The target uses a bool found flag pattern (li r0,1 / li r0,0 / clrlwi.)
 * which the native AVLTreeBase::FindGet (returning ValueType*) does not produce.
 */
struct DrawableMapFindHelper
{
    char pad[0x8];
    AVLTreeEntry<unsigned long, DrawableObject*>* m_Root;

    inline bool FindGet(unsigned long key, DrawableObject*** foundValue) const
    {
        AVLTreeEntry<unsigned long, DrawableObject*>* node = m_Root;
        while (node != NULL)
        {
            int cmpResult;
            if (key == node->key)
                cmpResult = 0;
            else if (key < node->key)
                cmpResult = -1;
            else
                cmpResult = 1;
            if (cmpResult == 0)
            {
                if (foundValue != NULL)
                    *foundValue = &node->value;
                return true;
            }
            else
            {
                if (cmpResult < 0)
                    node = (AVLTreeEntry<unsigned long, DrawableObject*>*)node->node.left;
                else
                    node = (AVLTreeEntry<unsigned long, DrawableObject*>*)node->node.right;
            }
        }
        return false;
    }
};

/**
 * Offset/Address/Size: 0x3A8 | 0x8019506C | size: 0x8C
 */
DrawableObject* World::FindDrawableObject(unsigned long uHashID)
{
    DrawableObject** foundValue;
    bool found = ((DrawableMapFindHelper*)&m_drawableMap)->FindGet(uHashID, &foundValue);
    if (found)
        return *foundValue;
    return nullptr;
}

class cGame;
class cBall;
extern cGame* g_pGame;
extern cBall* g_pBall;
extern unsigned char sSTSLighting__17DrawableCharacter;
void DoTranslucency(DrawableObject* pObject);

/**
 * Offset/Address/Size: 0x434 | 0x801950F8 | size: 0xB20
 * TODO: 70.1% match - register allocation shifts (stmw r23 vs r24)
 *       and g_pGame branch direction differences pending resolution.
 */
void World::Render()
{
    typedef AVLTreeEntry<unsigned long, DrawableObject*> Entry;
    struct NodeStack
    {
        Entry** data;
        u32 count;
    };

    int nSubmitted = 0;
    int nDrawn = 0;
    u8 bFreezeSide = g_bFreezeSideCam;
    if (bFreezeSide && g_bFreezeEndCam)
        g_bFreezeEndCam = 0;
    u8 bFreezeEnd = g_bFreezeEndCam;
    g_bDebugEqualsSide = bFreezeSide;
    g_bDebugEqualsEnd = bFreezeEnd;
    if (!g_bFreezeFrustum && !bFreezeSide && !bFreezeEnd)
        ExtractFrustumPlanes();
    u8 gameFlag = (g_pGame == NULL) ? (u8)0 : *(u8*)((u8*)g_pGame + 0x40);
    if (!gameFlag)
        sSTSLighting__17DrawableCharacter = 0;
    CreateLightUserData();

    NodeStack* iter;
    Entry* node;
    if (!sbIsHyperShootToScoreRenderingEnabled)
    {
        iter = (NodeStack*)nlMalloc(sizeof(NodeStack), 8, false);
        if (iter != NULL)
        {
            node = m_drawableMap.m_Root;
            iter->data = (Entry**)nlMalloc((m_drawableMap.m_NumElements + 1) * sizeof(Entry*), 8, false);
            iter->count = 0;
            if (node != NULL)
            {
                while (node->node.left != NULL)
                {
                    iter->data[iter->count] = node;
                    iter->count++;
                    node = (Entry*)node->node.left;
                }
                iter->data[iter->count] = node;
                iter->count++;
            }
        }
    }
    else
    {
        iter = (NodeStack*)nlMalloc(sizeof(NodeStack), 8, false);
        if (iter != NULL)
        {
            node = m_hyperSTSDrawableMap.m_Root;
            iter->data = (Entry**)nlMalloc((m_hyperSTSDrawableMap.m_NumElements + 1) * sizeof(Entry*), 8, false);
            iter->count = 0;
            if (node != NULL)
            {
                while (node->node.left != NULL)
                {
                    iter->data[iter->count] = node;
                    iter->count++;
                    node = (Entry*)node->node.left;
                }
                iter->data[iter->count] = node;
                iter->count++;
            }
        }
    }

    if (g_bClipToFrustum)
    {
        while (iter->count > 0)
        {
            DrawableObject* pObject = iter->data[iter->count - 1]->value;
            if (sbIsHyperShootToScoreRenderingEnabled)
            {
                const nlMatrix4& mat = pObject->GetWorldMatrix();
                if (mat.m[3][0] < 0.0f && sbShowPositiveXNetDuringHyperStrike__5World)
                    goto skipClip;
                const nlMatrix4& mat2 = pObject->GetWorldMatrix();
                if (mat2.m[3][0] > 0.0f && !sbShowPositiveXNetDuringHyperStrike__5World)
                    goto skipClip;
            }
            {
                u8 bHammer = 0;
                u8 bBall = 0;
                if (pObject->IsDrawableModel())
                {
                    bHammer = (pObject->AsDrawableModel()->m_pModel->id == HammerModelID);
                    bBall = (pObject->AsDrawableModel()->m_pModel->id == BallModelID);
                }
                {
                    DrawableObject* ballDrawable = ((DrawableObject**)g_pBall)[8];
                    if ((DrawableObject*)pObject->AsDrawableModel() == ballDrawable)
                        goto skipClip;
                }
                {
                    u32 objectFlags = pObject->m_uObjectFlags;
                    if (objectFlags & 0x80)
                        goto skipClip;
                    u8 bSkybox = 0;
                    if (!sbSkyboxRenderingDisabled__5World && (pObject->m_uObjectCreationFlags & 0x100))
                        bSkybox = 1;
                    if (sbStadiumRenderingDisabled__5World && !bBall && !bHammer && !bSkybox)
                        goto skipClip;
                    if (!(objectFlags & 0x1))
                        goto culledClip;
                    if (!(objectFlags & 0x10))
                    {
                        if (!IsSphereInFrustum(pObject->GetWorldMatrix(), pObject->m_fBoundingRadius))
                            goto culledClip;
                    }
                    if (pObject->m_uObjectCreationFlags & 0xF0000)
                        DoTranslucency(pObject);
                    pObject->Draw();
                    if (g_bDrawBoundingSphere)
                    {
                        f32 fRad = pObject->m_fBoundingRadius;
                        const nlMatrix4& wmDraw = pObject->GetWorldMatrix();
                        glModel* pSphere = glModelDup(glInventory.GetModel(nlStringHash("debug/sphere")), true);
                        nlMatrix4 mtx;
                        mtx.SetIdentity();
                        mtx.m[3][0] = wmDraw.m[3][0];
                        mtx.m[3][1] = wmDraw.m[3][1];
                        mtx.m[3][2] = wmDraw.m[3][2];
                        mtx.m[3][3] = 1.0f;
                        mtx.m[0][0] = fRad;
                        mtx.m[1][1] = fRad;
                        mtx.m[2][2] = fRad;
                        u32 whiteTex = WhiteTexture;
                        glModelPacket* pPkt = pSphere->packets;
                        while (pPkt < (glModelPacket*)((u8*)pSphere->packets + pSphere->numPackets * 0x4A))
                        {
                            glSetRasterState(pPkt->state.raster, (eGLState)5, 1);
                            u32 matID = glAllocMatrix();
                            if ((matID + 0x10000) != 0xFFFF)
                                glSetMatrix(matID, mtx);
                            pPkt->state.matrix = matID;
                            pPkt->state.texture[0] = whiteTex;
                            pPkt = (glModelPacket*)((u8*)pPkt + 0x4A);
                        }
                        glViewAttachModel((eGLView)7, pSphere);
                    }
                    nDrawn++;
                    goto advanceClip;
                culledClip:
                    pObject->m_translucency = 1.0f;
                    if (pObject->m_translucency < 0.0f)
                        pObject->m_translucency = 0.0f;
                    if (pObject->m_translucency > 1.0f)
                        pObject->m_translucency = 1.0f;
                }
            }
        advanceClip:
            nSubmitted++;
        skipClip:
            iter->count--;
            {
                Entry* top = iter->data[iter->count];
                Entry* right = (Entry*)top->node.right;
                if (right != NULL)
                {
                    while (right->node.left != NULL)
                    {
                        iter->data[iter->count] = right;
                        iter->count++;
                        right = (Entry*)right->node.left;
                    }
                    iter->data[iter->count] = right;
                    iter->count++;
                }
            }
        }
    }
    else
    {
        while (iter->count > 0)
        {
            DrawableObject* pObject = iter->data[iter->count - 1]->value;
            if (!(pObject->m_uObjectFlags & 0x1))
            {
                pObject->m_translucency = 1.0f;
                if (pObject->m_translucency < 0.0f)
                    pObject->m_translucency = 0.0f;
                if (pObject->m_translucency > 1.0f)
                    pObject->m_translucency = 1.0f;
            }
            else
            {
                if (pObject->m_uObjectCreationFlags & 0xF0000)
                    DoTranslucency(pObject);
                pObject->Draw();
                if (g_bDrawBoundingSphere)
                {
                    f32 fRad = pObject->m_fBoundingRadius;
                    const nlMatrix4& wmDraw = pObject->GetWorldMatrix();
                    glModel* pSphere = glModelDup(glInventory.GetModel(nlStringHash("debug/sphere")), true);
                    nlMatrix4 mtx;
                    mtx.SetIdentity();
                    mtx.m[3][0] = wmDraw.m[3][0];
                    mtx.m[3][1] = wmDraw.m[3][1];
                    mtx.m[3][2] = wmDraw.m[3][2];
                    mtx.m[3][3] = 1.0f;
                    mtx.m[0][0] = fRad;
                    mtx.m[1][1] = fRad;
                    mtx.m[2][2] = fRad;
                    u32 whiteTex = WhiteTexture;
                    glModelPacket* pPkt = pSphere->packets;
                    while (pPkt < (glModelPacket*)((u8*)pSphere->packets + pSphere->numPackets * 0x4A))
                    {
                        glSetRasterState(pPkt->state.raster, (eGLState)5, 1);
                        u32 matID = glAllocMatrix();
                        if ((matID + 0x10000) != 0xFFFF)
                            glSetMatrix(matID, mtx);
                        pPkt->state.matrix = matID;
                        pPkt->state.texture[0] = whiteTex;
                        pPkt = (glModelPacket*)((u8*)pPkt + 0x4A);
                    }
                    glViewAttachModel((eGLView)7, pSphere);
                }
                nDrawn++;
            }
            nSubmitted++;
            iter->count--;
            {
                Entry* top = iter->data[iter->count];
                Entry* right = (Entry*)top->node.right;
                if (right != NULL)
                {
                    while (right->node.left != NULL)
                    {
                        iter->data[iter->count] = right;
                        iter->count++;
                        right = (Entry*)right->node.left;
                    }
                    iter->data[iter->count] = right;
                    iter->count++;
                }
            }
        }
    }

    if (iter != NULL)
    {
        delete[] iter->data;
        delete iter;
    }

    if (g_bDrawCullingInfo)
    {
        char buf[256];
        static int x = 10;
        static int y = 0;
        f32 drawnPct = 100.0f * ((f32)nDrawn / (f32)nSubmitted);
        nlSNPrintf(buf, 255, "%d submitted, %d culled, %d ( %0.2f%% )drawn", nSubmitted, nSubmitted - nDrawn, nDrawn, drawnPct);
        nlColour color;
        *(u32*)&color.c[0] = 0xFFFFFFFF;
        glStateBundle state;
        glStateSave(state);
        glFontBegin(false);
        glFontPrint((eGLView)0x21, x, y, color, buf);
        glFontEnd();
        glStateRestore(state);
    }
}

/**
 * Offset/Address/Size: 0xF54 | 0x80195C18 | size: 0x1A0
 */
void World::HandleCameraSwitch()
{
    typedef AVLTreeEntry<unsigned long, DrawableObject*> Entry;

    struct NodeStack
    {
        Entry** data;
        u32 count;
    };

    NodeStack* stack;
    Entry* node;

    stack = (NodeStack*)nlMalloc(sizeof(NodeStack), 8, false);
    if (stack != NULL)
    {
        u32 numElements = m_drawableMap.m_NumElements;
        node = m_drawableMap.m_Root;
        stack->data = (Entry**)nlMalloc((numElements + 1) * sizeof(Entry*), 8, false);
        stack->count = 0;

        if (node != NULL)
        {
            while (node->node.left != NULL)
            {
                stack->data[stack->count] = node;
                stack->count++;
                node = (Entry*)node->node.left;
            }
            stack->data[stack->count] = node;
            stack->count++;
        }
    }

    f32 maxVal = 1.0f;
    f32 minVal = 0.0f;

    while (stack->count > 0)
    {
        DrawableObject* pObject = stack->data[stack->count - 1]->value;

        pObject->m_translucency = maxVal;
        if (pObject->m_translucency < minVal)
        {
            pObject->m_translucency = minVal;
        }
        if (pObject->m_translucency > maxVal)
        {
            pObject->m_translucency = maxVal;
        }

        stack->count--;

        Entry* right = (Entry*)stack->data[stack->count]->node.right;
        if (right != NULL)
        {
            while (right->node.left != NULL)
            {
                stack->data[stack->count] = right;
                stack->count++;
                right = (Entry*)right->node.left;
            }
            stack->data[stack->count] = right;
            stack->count++;
        }
    }

    if (stack != NULL)
    {
        delete[] stack->data;
        delete stack;
    }
}

/**
 * Offset/Address/Size: 0x10F4 | 0x80195DB8 | size: 0x24C
 */
class cRumbleFilter;

class cBaseCamera
{
public:
    virtual ~cBaseCamera() { }
    virtual int GetType() = 0;
    virtual void Update(float) = 0;
    virtual const nlMatrix4& GetViewMatrix() const = 0;
    virtual float GetFOV() const;
    virtual void Reactivate() { }
    virtual const nlVector3& GetTargetPosition() const = 0;
    virtual const nlVector3& GetCameraPosition() const = 0;

    cBaseCamera* m_next;
    cBaseCamera* m_prev;
    cRumbleFilter* m_pFilter;
    nlVector3 mUpVector;
};

class cCameraManager
{
public:
    static int m_pBeginFrameCameraType;
    static cBaseCamera* m_cameraStack;
    static unsigned char IsObjectOccludingField(const DrawableObject*);
};

struct MatrixEffectCamStub
{
    char _pad[0x13C];
    unsigned char m_transitioningOut;
};

extern unsigned char sbPretendWereNotInGameplayCam;

void DoTranslucency(DrawableObject* pObject)
{
    int cameraType = cCameraManager::m_pBeginFrameCameraType;

    if (cameraType == 4)
    {
        pObject->m_translucency = 1.0f;
        if (pObject->m_translucency < 0.0f)
        {
            pObject->m_translucency = 0.0f;
        }
        if (pObject->m_translucency > 1.0f)
        {
            pObject->m_translucency = 1.0f;
        }
        return;
    }

    float fTrans = pObject->m_translucency;
    bool inGameplayCamera = false;
    bool transitioningOutOfGameplayCamera = false;
    bool canBeTransparent = false;

    if ((cameraType == 7) || (cameraType == 10))
    {
        inGameplayCamera = true;
    }

    if (sbPretendWereNotInGameplayCam)
    {
        inGameplayCamera = false;
    }

    if (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->GetType() == 8)
    {
        if (((MatrixEffectCamStub*)nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack))->m_transitioningOut)
        {
            transitioningOutOfGameplayCamera = true;
        }
    }

    unsigned long objectCreationFlags = pObject->m_uObjectCreationFlags;

    if (objectCreationFlags & 0x2000)
    {
        if (inGameplayCamera)
        {
            pObject->m_translucency = 0.0f;
            if (pObject->m_translucency < 0.0f)
            {
                pObject->m_translucency = 0.0f;
            }
            if (pObject->m_translucency > 1.0f)
            {
                pObject->m_translucency = 1.0f;
            }
            return;
        }

        if (transitioningOutOfGameplayCamera)
        {
            canBeTransparent = true;
        }
    }

    if (objectCreationFlags & 0x8000)
    {
        if (inGameplayCamera)
        {
            pObject->m_translucency = 0.0f;
            if (pObject->m_translucency < 0.0f)
            {
                pObject->m_translucency = 0.0f;
            }
            if (pObject->m_translucency > 1.0f)
            {
                pObject->m_translucency = 1.0f;
            }
            return;
        }
    }

    if (!(objectCreationFlags & 0x1000) && !canBeTransparent && !sbAllObjectsCanBeTransparent)
    {
        pObject->m_translucency = 1.0f;
        if (pObject->m_translucency < 0.0f)
        {
            pObject->m_translucency = 0.0f;
        }
        if (pObject->m_translucency > 1.0f)
        {
            pObject->m_translucency = 1.0f;
        }
        return;
    }

    if (cCameraManager::IsObjectOccludingField(pObject))
    {
        fTrans -= g_fTransAdjustOccluded;
    }
    else
    {
        fTrans += g_fTransAdjustNotOccluded;
    }

    if (fTrans > 1.0f)
    {
        fTrans = 1.0f;
    }

    if (fTrans < g_fTransMinimum)
    {
        fTrans = g_fTransMinimum;
    }

    pObject->m_translucency = fTrans;
    if (pObject->m_translucency < 0.0f)
    {
        pObject->m_translucency = 0.0f;
    }
    if (pObject->m_translucency > 1.0f)
    {
        pObject->m_translucency = 1.0f;
    }
}

/**
 * Offset/Address/Size: 0x1340 | 0x80196004 | size: 0xE8
 * TODO: 82.8% match - f2/f4 float register swap (posX) and addic./bne vs mtctr/bdnz
 * caused by -inline deferred in scratch context (target uses -inline auto)
 */
bool World::IsSphereInFrustum(const nlMatrix4& mat, float radius)
{
    volatile u32 tz;
    volatile u32 ty;
    volatile u32 tx;
    tx = *(u32*)&mat.m[3][0];
    f32 posX = *(f32*)&tx;
    s32 numSets = 2;
    ty = *(u32*)&mat.m[3][1];
    f32* plane = (f32*)((u8*)this + 0x80);
    tz = *(u32*)&mat.m[3][2];
    s32 count = 0;
    f32 posY = *(f32*)&ty;
    f32 posZ = *(f32*)&tz;
    f32 negRadius = -radius;
    do
    {
        if ((posZ * plane[2] + (posX * plane[0] + posY * plane[1]) + *(f32*)((u8*)this + 0x8C + count * 0x18)) < negRadius)
            return false;
        if ((posZ * plane[6] + (posX * plane[4] + posY * plane[5]) + *(f32*)((u8*)this + 0x9C + count * 0x18)) < negRadius)
            return false;
        if ((posZ * plane[10] + (posX * plane[8] + posY * plane[9]) + *(f32*)((u8*)this + 0xAC + count * 0x18)) < negRadius)
            return false;
        plane += 12;
        count += 2;
    } while (--numSets != 0);
    return true;
}

/**
 * Offset/Address/Size: 0x1428 | 0x801960EC | size: 0x458
 */
void World::ExtractFrustumPlanes()
{
    nlMatrix4 viewProjection;
    nlMatrix4 projection = *glViewGetProjectionMatrix(GLV_Unshadowed);

    float m33 = projection.f.m33;
    float m34 = projection.f.m34;
    float m43 = projection.f.m43;
    projection.f.m43 = m34;
    projection.f.m34 = m43;
    projection.f.m33 = m33 - 1.0f;

    nlMultMatrices(viewProjection, *glViewGetViewMatrix(GLV_Unshadowed), projection);

    m_frustumPlane[0].f.x = viewProjection.f.m14 - viewProjection.f.m11;
    m_frustumPlane[0].f.y = viewProjection.f.m24 - viewProjection.f.m21;
    m_frustumPlane[0].f.z = viewProjection.f.m34 - viewProjection.f.m31;
    m_frustumPlane[0].f.w = viewProjection.f.m44 - viewProjection.f.m41;
    {
        float length = nlSqrt(
            m_frustumPlane[0].f.x * m_frustumPlane[0].f.x + m_frustumPlane[0].f.y * m_frustumPlane[0].f.y + m_frustumPlane[0].f.z * m_frustumPlane[0].f.z,
            true);
        m_frustumPlane[0].f.x /= length;
        m_frustumPlane[0].f.y /= length;
        m_frustumPlane[0].f.z /= length;
        m_frustumPlane[0].f.w /= length;
    }

    m_frustumPlane[1].f.x = viewProjection.f.m14 + viewProjection.f.m11;
    m_frustumPlane[1].f.y = viewProjection.f.m24 + viewProjection.f.m21;
    m_frustumPlane[1].f.z = viewProjection.f.m34 + viewProjection.f.m31;
    m_frustumPlane[1].f.w = viewProjection.f.m44 + viewProjection.f.m41;
    {
        float length = nlSqrt(
            m_frustumPlane[1].f.x * m_frustumPlane[1].f.x + m_frustumPlane[1].f.y * m_frustumPlane[1].f.y + m_frustumPlane[1].f.z * m_frustumPlane[1].f.z,
            true);
        m_frustumPlane[1].f.x /= length;
        m_frustumPlane[1].f.y /= length;
        m_frustumPlane[1].f.z /= length;
        m_frustumPlane[1].f.w /= length;
    }

    m_frustumPlane[2].f.x = viewProjection.f.m14 + viewProjection.f.m12;
    m_frustumPlane[2].f.y = viewProjection.f.m24 + viewProjection.f.m22;
    m_frustumPlane[2].f.z = viewProjection.f.m34 + viewProjection.f.m32;
    m_frustumPlane[2].f.w = viewProjection.f.m44 + viewProjection.f.m42;
    {
        float length = nlSqrt(
            m_frustumPlane[2].f.x * m_frustumPlane[2].f.x + m_frustumPlane[2].f.y * m_frustumPlane[2].f.y + m_frustumPlane[2].f.z * m_frustumPlane[2].f.z,
            true);
        m_frustumPlane[2].f.x /= length;
        m_frustumPlane[2].f.y /= length;
        m_frustumPlane[2].f.z /= length;
        m_frustumPlane[2].f.w /= length;
    }

    m_frustumPlane[3].f.x = viewProjection.f.m14 - viewProjection.f.m12;
    m_frustumPlane[3].f.y = viewProjection.f.m24 - viewProjection.f.m22;
    m_frustumPlane[3].f.z = viewProjection.f.m34 - viewProjection.f.m32;
    m_frustumPlane[3].f.w = viewProjection.f.m44 - viewProjection.f.m42;
    {
        float length = nlSqrt(
            m_frustumPlane[3].f.x * m_frustumPlane[3].f.x + m_frustumPlane[3].f.y * m_frustumPlane[3].f.y + m_frustumPlane[3].f.z * m_frustumPlane[3].f.z,
            true);
        m_frustumPlane[3].f.x /= length;
        m_frustumPlane[3].f.y /= length;
        m_frustumPlane[3].f.z /= length;
        m_frustumPlane[3].f.w /= length;
    }

    m_frustumPlane[4].f.x = viewProjection.f.m13;
    m_frustumPlane[4].f.y = viewProjection.f.m23;
    m_frustumPlane[4].f.z = viewProjection.f.m33;
    m_frustumPlane[4].f.w = viewProjection.f.m43;
    {
        float length = nlSqrt(
            m_frustumPlane[4].f.x * m_frustumPlane[4].f.x + m_frustumPlane[4].f.y * m_frustumPlane[4].f.y + m_frustumPlane[4].f.z * m_frustumPlane[4].f.z,
            true);
        m_frustumPlane[4].f.x /= length;
        m_frustumPlane[4].f.y /= length;
        m_frustumPlane[4].f.z /= length;
        m_frustumPlane[4].f.w /= length;
    }

    m_frustumPlane[5].f.x = viewProjection.f.m14 - viewProjection.f.m13;
    m_frustumPlane[5].f.y = viewProjection.f.m24 - viewProjection.f.m23;
    m_frustumPlane[5].f.z = viewProjection.f.m34 - viewProjection.f.m33;
    m_frustumPlane[5].f.w = viewProjection.f.m44 - viewProjection.f.m43;
    {
        float length = nlSqrt(
            m_frustumPlane[5].f.x * m_frustumPlane[5].f.x + m_frustumPlane[5].f.y * m_frustumPlane[5].f.y + m_frustumPlane[5].f.z * m_frustumPlane[5].f.z,
            true);
        m_frustumPlane[5].f.x /= length;
        m_frustumPlane[5].f.y /= length;
        m_frustumPlane[5].f.z /= length;
        m_frustumPlane[5].f.w /= length;
    }
}

/**
 * Offset/Address/Size: 0x1880 | 0x80196544 | size: 0x4
 */
void World::UpdateInReplay(float)
{
}

/**
 * Offset/Address/Size: 0x1884 | 0x80196548 | size: 0x98
 */
void World::Update(float fDeltaT)
{
    DLListEntry<WorldAnimController*>* start = nlDLRingGetStart(m_animControllerList.m_Head);
    DLListEntry<WorldAnimController*>* head = m_animControllerList.m_Head;
    DLListEntry<WorldAnimController*>* current = start;

    while (current != NULL)
    {
        current->m_data->Update(fDeltaT);

        if ((nlDLRingIsEnd(head, current) != 0) || (current == NULL))
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
 * Offset/Address/Size: 0x191C | 0x801965E0 | size: 0x260
 */
class FlareHandler
{
public:
    static FlareHandler instance;
    void AddHalo(const nlMatrix4&);
    void AddGlow(const nlMatrix4&);

    char _pad[0x70];
};

void World::CreateHelperObjFromChunk(nlChunk* chunk)
{
    static int flareLen;
    static signed char init;

    HelperObject* pHelper;
    WorldHelperChunkData* pWorldHelperChunkData;
    char* substring;
    const char* flashString;
    const char* flareTag;
    char flareName[64];

    u32 chunkFlags = *(u32*)chunk;
    u32 alignment = chunkFlags & 0x7F000000;

    if ((((u32)(-(s32)alignment) | alignment) >> 31) != 0)
    {
        u32 shift = alignment >> 24;
        u32 alignBytes = 1 << shift;
        u8* pData = (u8*)chunk;
        pData = pData + alignBytes;
        pData = pData + 7;
        pWorldHelperChunkData = (WorldHelperChunkData*)((u32)pData & ~(alignBytes - 1));
    }
    else
    {
        pWorldHelperChunkData = (WorldHelperChunkData*)((u8*)chunk + 8);
    }

    pHelper = (HelperObject*)nlMalloc(sizeof(HelperObject), 8, false);
    pHelper->m_uHashID = pWorldHelperChunkData->m_uHashID;
    pHelper->m_worldMatrix = pWorldHelperChunkData->m_worldMatrix;

    substring = nlStrChr<char>(pWorldHelperChunkData->m_szName, '/');
    flashString = "fx_camera_flash";
    if (nlStrNICmp<char>(substring + 1, flashString, nlStrLen<char>(flashString)) == 0)
    {
        nlStrNCpy<char>(pHelper->m_szName, substring + 1, 0x40);
    }
    else
    {
        flareTag = "flare_";

        if (!init)
        {
            flareLen = nlStrLen<char>(flareTag);
            init = 1;
        }

        substring = strstr(pWorldHelperChunkData->m_szName, flareTag);
        if (substring != NULL)
        {
            nlStrNCpy<char>(flareName, substring + flareLen, 0x40);

            substring = strstr(flareName, "_");
            if (substring != NULL)
            {
                *substring = '\0';
            }

            if (nlToLower<char>(flareName[0]) == 'h')
            {
                FlareHandler::instance.AddHalo(pWorldHelperChunkData->m_worldMatrix);
            }
            else
            {
                FlareHandler::instance.AddGlow(pWorldHelperChunkData->m_worldMatrix);
            }

            delete pHelper;
            return;
        }

        nlStrNCpy<char>(pHelper->m_szName, pWorldHelperChunkData->m_szName, 0x40);
    }

    AVLTreeNode* pExistingNode;
    m_helperMap.AddAVLNode((AVLTreeNode**)&m_helperMap.m_Root, pHelper, &pHelper, &pExistingNode, m_helperMap.m_NumElements);
    if (pExistingNode == NULL)
    {
        m_helperMap.m_NumElements++;
    }
}

static inline void* getUserData(void* p)
{
    return glUserGetData(p);
}

/**
 * Offset/Address/Size: 0x1B7C | 0x80196840 | size: 0x12C
 * TODO: 98.67% match - remaining r29/r31 register swap; current World layout is missing m_pPlayerNISLightData so m_pSpecularData aliases target m_pSTSIntensity offset
 */
void* World::GetCustomSpecularData(glModelPacket* pPacket, bool bPerm)
{
    u32 allocSize;
    GLSpecularUserData* pEntries;
    int numLights;
    void* pSTSIntensity;
    pSTSIntensity = m_pSpecularData;

    u8 glossLevel = (u8)glGetTextureState(pPacket->state.texturestate, GLTS_GlossLevel);

    f32 fGloss = (f32)glossLevel;
    f32 fNorm = fGloss * 0.015873017f;
    f32 fInv = 1.0f - fNorm;
    f32 fExponent = g_fExponentScale * fInv + g_fExponentBase;

    numLights = *(int*)glUserGetData(pSTSIntensity);
    allocSize = numLights * sizeof(GLSpecularUserData) + 4;
    void* pNewData = glUserAlloc(GLUD_Specular, allocSize, bPerm);

    void* pSpec = getUserData(pNewData);
    pEntries = (GLSpecularUserData*)((int*)pSpec + 1);
    pSTSIntensity = glUserGetData(pSTSIntensity);

    memcpy(pSpec, pSTSIntensity, allocSize);

    for (int i = numLights; i > 0; i--)
    {
        pEntries->exponent = fExponent;
        pEntries++;
    }

    return pNewData;
}

/**
 * Offset/Address/Size: 0x1CA8 | 0x8019696C | size: 0x9A4
 */
void World::CreateLightUserData()
{
    struct WorldLightUserDataFields
    {
        char _pad0[0x30];
        void* m_pLightData;
        void* m_pPlayerNISLightData;
        void* m_pIntensityPerm;
        void* m_pIntensityData;
        void* m_pSTSIntensity;
    };

    WorldLightUserDataFields* fields = (WorldLightUserDataFields*)this;
    ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*> > > lightList;
    ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*> > > specList;
    int numLights = 0;
    int numSpecLights = 0;
    u32* pStack = (u32*)nlMalloc(8, 8, false);

    if (pStack != NULL)
    {
        u32 numElements = m_lightMap.m_NumElements;
        AVLTreeEntry<unsigned long, LightObject*>* pNode = m_lightMap.m_Root;
        pStack[0] = (u32)nlMalloc((numElements + 1) * 4, 8, false);
        pStack[1] = 0;

        if (pNode != NULL)
        {
            while (pNode->node.left != NULL)
            {
                ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]] = pNode;
                pStack[1]++;
                pNode = (AVLTreeEntry<unsigned long, LightObject*>*)pNode->node.left;
            }
            ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]] = pNode;
            pStack[1]++;
        }
    }

    while (pStack[1] != 0)
    {
        AVLTreeEntry<unsigned long, LightObject*>* pNode = ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1] - 1];
        LightObject* pLight = pNode->value;

        if ((pLight->m_emitFlags & 0x4) == 0)
        {
            ListEntry<LightObject*>* entry = (ListEntry<LightObject*>*)nlMalloc(8, 8, false);
            numLights++;
            if (entry != NULL)
            {
                entry->next = NULL;
                entry->data = pLight;
            }
            nlListAddStart(&lightList.m_Head, entry, &lightList.m_Tail);
        }

        if ((pLight->m_emitFlags & 0x2) != 0)
        {
            ListEntry<LightObject*>* entry = (ListEntry<LightObject*>*)nlMalloc(8, 8, false);
            numSpecLights++;
            if (entry != NULL)
            {
                entry->next = NULL;
                entry->data = pLight;
            }
            nlListAddStart(&specList.m_Head, entry, &specList.m_Tail);
        }

        pStack[1]--;
        {
            AVLTreeEntry<unsigned long, LightObject*>* pChild = (AVLTreeEntry<unsigned long, LightObject*>*)(((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]])->node.right;

            if (pChild == NULL)
            {
                continue;
            }

            while (pChild->node.left != NULL)
            {
                ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]] = pChild;
                pStack[1]++;
                pChild = (AVLTreeEntry<unsigned long, LightObject*>*)pChild->node.left;
            }
            ((AVLTreeEntry<unsigned long, LightObject*>**)pStack[0])[pStack[1]] = pChild;
            pStack[1]++;
        }
    }

    if (pStack != NULL)
    {
        delete[] (u8*)pStack[0];
        delete (u8*)pStack;
    }

    int numExtra = 0;
    LightObject* pFX = fxLightObjects;
    int i = 0;
    while (i < EmissionManager::GetNumLights())
    {
        const EffectsLight* pLight = EmissionManager::GetLight(i);
        numExtra++;
        i++;

        pFX->m_worldPosition = pLight->m_v3Position;
        pFX->m_fIntensity = (2.0f * (f32)pLight->m_Colour.c[3]) / 255.0f;
        pFX->m_fFarAttenuationStart = 0.0f;
        pFX->m_fFarAttenuationEnd = pLight->m_fRadius;
        pFX->m_colour.c[0] = 0.003921569f * (f32)pLight->m_Colour.c[0];
        pFX->m_colour.c[1] = 0.003921569f * (f32)pLight->m_Colour.c[1];
        pFX->m_colour.c[2] = 0.003921569f * (f32)pLight->m_Colour.c[2];
        pFX->m_colour.c[3] = 0.003921569f * (f32)pLight->m_Colour.c[3];
        pFX->m_fRadiusSquared = pLight->m_fRadius * pLight->m_fRadius;
        pFX = (LightObject*)((u8*)pFX + 0x38);
    }

    if (numLights == 0)
    {
        fields->m_pLightData = NULL;
        fields->m_pIntensityPerm = NULL;
    }
    else
    {
        int totalLights = numLights + numExtra;
        unsigned long size = totalLights * sizeof(GLLightUserData) + 4;
        fields->m_pLightData = glUserAlloc(GLUD_Light, size, false);

        u32* p32 = (u32*)glUserGetData(fields->m_pLightData);
        *p32 = totalLights;
        GLLightUserData* glLight = (GLLightUserData*)(p32 + 1);

        ListEntry<LightObject*>* entry = lightList.m_Head;
        while (entry != NULL)
        {
            LightObject* pLight = entry->data;
            glLight->colour = pLight->m_colour;
            glLight->worldPosition = pLight->m_worldPosition;
            glLight->intensity = pLight->m_fIntensity;
            if (pLight->m_emitFlags & 0x8)
            {
                glLight->innerRadius = 0.0f;
                glLight->outerRadius = 0.0f;
            }
            else
            {
                glLight->innerRadius = pLight->m_fFarAttenuationStart;
                glLight->outerRadius = pLight->m_fFarAttenuationEnd;
            }
            entry = entry->next;
            glLight++;
        }

        if (numExtra > 0)
        {
            LightObject* pLight = fxLightObjects;
            i = numExtra;
            while (i > 0)
            {
                glLight->colour = pLight->m_colour;
                glLight->worldPosition = pLight->m_worldPosition;
                glLight->intensity = pLight->m_fIntensity;
                glLight->innerRadius = pLight->m_fFarAttenuationStart;
                glLight->outerRadius = pLight->m_fFarAttenuationEnd;
                pLight = (LightObject*)((u8*)pLight + 0x38);
                glLight++;
                i--;
            }
        }

        fields->m_pIntensityPerm = glUserAlloc(GLUD_Light, size, false);
        p32 = (u32*)glUserGetData(fields->m_pIntensityPerm);
        *p32 = totalLights;
        void* pIntensityPermData = p32;
        glLight = (GLLightUserData*)(p32 + 1);
        float fBlueWeight = 0.11f;
        float fRedWeight = 0.3f;
        float fGreenWeight = 0.59f;
        float fZero = 0.0f;
        float fOne = 1.0f;

        entry = lightList.m_Head;
        while (entry != NULL)
        {
            LightObject* pLight = entry->data;
            float fIntensity = fGreenWeight * pLight->m_colour.c[1];
            fIntensity = fRedWeight * pLight->m_colour.c[0] + fIntensity;
            fIntensity = fBlueWeight * pLight->m_colour.c[2] + fIntensity;

            nlFloatColour intensityColour;
            intensityColour.c[1] = fZero;
            intensityColour.c[2] = fZero;
            intensityColour.c[3] = fOne;
            intensityColour.c[0] = fIntensity;

            glLight->colour = intensityColour;
            glLight->worldPosition = pLight->m_worldPosition;
            glLight->intensity = pLight->m_fIntensity;
            if (pLight->m_emitFlags & 0x8)
            {
                glLight->innerRadius = 0.0f;
                glLight->outerRadius = 0.0f;
            }
            else
            {
                glLight->innerRadius = pLight->m_fFarAttenuationStart;
                glLight->outerRadius = pLight->m_fFarAttenuationEnd;
            }
            entry = entry->next;
            glLight++;
        }

        if (numExtra > 0)
        {
            LightObject* pLight = fxLightObjects;
            i = numExtra;
            while (i > 0)
            {
                glLight->colour = pLight->m_colour;
                glLight->worldPosition = pLight->m_worldPosition;
                glLight->intensity = pLight->m_fIntensity;
                glLight->innerRadius = pLight->m_fFarAttenuationStart;
                glLight->outerRadius = pLight->m_fFarAttenuationEnd;
                pLight = (LightObject*)((u8*)pLight + 0x38);
                glLight++;
                i--;
            }
        }

        if (fields->m_pPlayerNISLightData == NULL)
        {
            size = numLights * sizeof(GLLightUserData) + 4;
            fields->m_pPlayerNISLightData = glUserAlloc(GLUD_Light, size, true);
            void* pPlayerNISData = glUserGetData(fields->m_pPlayerNISLightData);
            memcpy(pPlayerNISData, pIntensityPermData, size);
        }
    }

    if (numSpecLights == 0)
    {
        fields->m_pSTSIntensity = NULL;
    }
    else
    {
        const nlVector3 origin = { { 0.0f, 0.0f, 0.0f } };
        unsigned long size = numSpecLights * sizeof(GLSpecularUserData) + 4;
        fields->m_pSTSIntensity = glUserAlloc(GLUD_Specular, size, false);
        u32* p32 = (u32*)glUserGetData(fields->m_pSTSIntensity);
        *p32 = numSpecLights;
        GLSpecularUserData* pSpec = (GLSpecularUserData*)(p32 + 1);
        ListEntry<LightObject*>* entry = specList.m_Head;
        while (entry != NULL)
        {
            LightObject* pLight = entry->data;
            pSpec->colour = pLight->m_colour;
            pSpec->exponent = 64.0f;
            pSpec->intensity = pLight->m_fIntensity;
            pSpec->worldDirection.f.x = origin.f.x - pLight->m_worldPosition.f.x;
            pSpec->worldDirection.f.y = origin.f.y - pLight->m_worldPosition.f.y;
            pSpec->worldDirection.f.z = origin.f.z - pLight->m_worldPosition.f.z;
            pSpec++;
            entry = entry->next;
        }
    }

    fields->m_pIntensityData = glUserAlloc(GLUD_Light, 0x2C, false);
    {
        u32* p32 = (u32*)glUserGetData(fields->m_pIntensityData);
        *p32 = 1;
        GLLightUserData* glLight = (GLLightUserData*)(p32 + 1);
        nlZeroMemory(glLight, sizeof(GLLightUserData));
        glLight->worldPosition = vLightDirection;
        glLight->colour.c[0] = 1.0f;
        glLight->colour.c[1] = 1.0f;
        glLight->colour.c[2] = 1.0f;
        glLight->colour.c[3] = 1.0f;
        glLight->intensity = 1.0f;
        glLight->innerRadius = 0.0f;
        glLight->outerRadius = 0.0f;
    }

    nlWalkList(specList.m_Head, &specList, &ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*> > >::DeleteEntry);
    specList.m_Head = NULL;
    specList.m_Tail = NULL;
    nlWalkList(lightList.m_Head, &lightList, &ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*> > >::DeleteEntry);
    lightList.m_Head = NULL;
    lightList.m_Tail = NULL;
}

/**
 * Offset/Address/Size: 0x264C | 0x80197310 | size: 0x9D4
 * TODO: 74.59% match - reversed register allocation order (MWCC top-down vs bottom-up),
 *       phantom r21 callee-save, 6 missing switch tree guard instructions
 */
extern u32 __vt__20CharacterPhysicsData[];
static const int LF_NOLIGHT = 4;

static inline void* nlGetChunkData(nlChunk* pChunk)
{
    u32 alignField = pChunk->m_ID & 0x7F000000;
    u32 isAligned = ((-alignField) | alignField) >> 31;
    if (isAligned != 0)
    {
        u32 alignment = 1u << (alignField >> 24);
        u32 addr = (u32)pChunk + alignment;
        u32 mask = alignment - 1;
        addr = (addr + 7) & ~mask;
        return (void*)addr;
    }
    return (void*)((u8*)pChunk + 8);
}

static inline nlChunk* nlGetNextChunk(nlChunk* pChunk)
{
    return (nlChunk*)((u8*)pChunk + pChunk->m_Size + 8);
}

bool World::LoadObjectData(const char* szWorldName)
{
    typedef AVLTreeEntry<unsigned long, LightObject*> LightEntry;

    char szFullFileName[255];
    nlSNPrintf(szFullFileName, 255, "art/%s.wld", szWorldName);
    tDebugPrintManager::Print(DC_RENDER, "Loading world object file: %s\n", szFullFileName);

    void* pWorldData = nlLoadEntireFile(szFullFileName, NULL, 0x20, AllocateEnd);
    if (pWorldData == NULL)
    {
        nlPrintf("Error: Failed to load world object data '%s'\n", szFullFileName);
        return false;
    }

    nlChunk* pChunk = (nlChunk*)((u8*)pWorldData + 8);
    AVLTreeNode** ppLightRoot = (AVLTreeNode**)&m_lightMap.m_Root;
    nlChunk* pEnd = (nlChunk*)((u8*)pWorldData + ((nlChunk*)pWorldData)->m_Size + 8);
    AVLTreeNode** ppHelperRoot = (AVLTreeNode**)&m_helperMap.m_Root;

    while (pChunk != pEnd)
    {
        int chunkType = (int)(pChunk->m_ID & 0x80FFFFFF);

        switch (chunkType)
        {
        case 0x19001:
            break;
        case 0x19003:
        {
            struct WorldObjectDataLocal
            {
                unsigned long m_uObjectCreationFlags;
                unsigned long m_uHashID;
                unsigned long m_uModelID;
                unsigned long m_uShadowHashID;
                unsigned long m_uRenderLayer;
                nlMatrix4 m_worldMatrix;
                float m_fRadius;
                nlVector3 m_v3Offset;
            };

            WorldObjectDataLocal local;
            memset(&local, 0, sizeof(WorldObjectDataLocal));

            u8* pData = (u8*)nlGetChunkData(pChunk);

            local.m_uObjectCreationFlags = *(u32*)(pData + 0x90);
            local.m_uHashID = *(u32*)(pData + 0x80);

            u32* pSrcMatrix = (u32*)(pData + 0xA0);
            u32* pDstMatrix = (u32*)&local.m_worldMatrix;
            pDstMatrix[0] = pSrcMatrix[0];
            pDstMatrix[1] = pSrcMatrix[1];
            pDstMatrix[2] = pSrcMatrix[2];
            pDstMatrix[3] = pSrcMatrix[3];
            pDstMatrix[4] = pSrcMatrix[4];
            pDstMatrix[5] = pSrcMatrix[5];
            pDstMatrix[6] = pSrcMatrix[6];
            pDstMatrix[7] = pSrcMatrix[7];
            pDstMatrix[8] = pSrcMatrix[8];
            pDstMatrix[9] = pSrcMatrix[9];
            pDstMatrix[10] = pSrcMatrix[10];
            pDstMatrix[11] = pSrcMatrix[11];
            pDstMatrix[12] = pSrcMatrix[12];
            pDstMatrix[13] = pSrcMatrix[13];
            pDstMatrix[14] = pSrcMatrix[14];
            pDstMatrix[15] = pSrcMatrix[15];

            local.m_uShadowHashID = *(u32*)(pData + 0x8C);
            local.m_uRenderLayer = *(u32*)(pData + 0x94);
            local.m_uModelID = *(u32*)(pData + 0x84);
            local.m_fRadius = *(float*)(pData + 0x98);

            HandleObjectCreation((WorldObjectData*)&local);
            break;
        }
        case 0x19005:
        {
            u8* pData = (u8*)nlGetChunkData(pChunk);

            LightObject* pLight = (LightObject*)nlMalloc(sizeof(LightObject), 8, false);

            pLight->m_uHashID = *(u32*)(pData + 0x40);

            pLight->m_worldPosition.as_u32[0] = *(u32*)(pData + 0x90);
            pLight->m_worldPosition.as_u32[1] = *(u32*)(pData + 0x94);
            pLight->m_worldPosition.as_u32[2] = *(u32*)(pData + 0x98);

            pLight->m_fIntensity = *(float*)(pData + 0x44);
            pLight->m_fFarAttenuationStart = *(float*)(pData + 0x48);
            pLight->m_fFarAttenuationEnd = *(float*)(pData + 0x4C);
            pLight->m_emitFlags = *(u32*)(pData + 0x5C);

            if (pLight->m_fIntensity < 0.01f)
            {
                pLight->m_emitFlags |= LF_NOLIGHT;
            }

            pLight->m_colour.c[0] = *(float*)(pData + 0x50);
            pLight->m_colour.c[1] = *(float*)(pData + 0x54);
            pLight->m_colour.c[2] = *(float*)(pData + 0x58);
            pLight->m_colour.c[3] = 0.0f;
            pLight->m_bit = 0;

            AVLTreeNode* pExistingNode;
            m_lightMap.AddAVLNode(ppLightRoot, pLight, &pLight, &pExistingNode, m_lightMap.m_NumElements);
            if (pExistingNode == NULL)
            {
                m_lightMap.m_NumElements++;
            }
            break;
        }
        case 0x19101:
        {
            u8* pData = (u8*)nlGetChunkData(pChunk);

            const char* persistentHeader = "fx_persistent_";
            static int persistentLen = nlStrLen<char>(persistentHeader);

            char* found = strstr((char*)pData, persistentHeader);
            if (found != NULL)
            {
                char szBuffer[256];
                nlStrNCpy<char>(szBuffer, found + persistentLen, 256);

                int len = strlen(szBuffer);
                if (__ctype_map[(unsigned char)szBuffer[len - 1]] & __digit)
                {
                    len = strlen(szBuffer);
                    char* p = szBuffer + len;
                    while (len > 0)
                    {
                        if (*p == '_')
                        {
                            szBuffer[len] = '\0';
                            break;
                        }
                        p--;
                        len--;
                    }
                }

                EffectsGroup* pGroup = fxGetGroup(szBuffer);
                if (pGroup != NULL)
                {
                    EmissionController* pEmitter = EmissionManager::Create(pGroup, 0);
                    pEmitter->SetPosition(*(const nlVector3*)(pData + 0x90));
                    pEmitter->m_uUserData = 0xDEADBEEF;
                }
            }
            else
            {
                HelperObject* pHelper = (HelperObject*)nlMalloc(sizeof(HelperObject), 8, false);

                pHelper->m_uHashID = *(u32*)(pData + 0x40);

                u32* pSrcMatrix = (u32*)(pData + 0x60);
                u32* pDstMatrix = (u32*)&pHelper->m_worldMatrix;
                pDstMatrix[0] = pSrcMatrix[0];
                pDstMatrix[1] = pSrcMatrix[1];
                pDstMatrix[2] = pSrcMatrix[2];
                pDstMatrix[3] = pSrcMatrix[3];
                pDstMatrix[4] = pSrcMatrix[4];
                pDstMatrix[5] = pSrcMatrix[5];
                pDstMatrix[6] = pSrcMatrix[6];
                pDstMatrix[7] = pSrcMatrix[7];
                pDstMatrix[8] = pSrcMatrix[8];
                pDstMatrix[9] = pSrcMatrix[9];
                pDstMatrix[10] = pSrcMatrix[10];
                pDstMatrix[11] = pSrcMatrix[11];
                pDstMatrix[12] = pSrcMatrix[12];
                pDstMatrix[13] = pSrcMatrix[13];
                pDstMatrix[14] = pSrcMatrix[14];
                pDstMatrix[15] = pSrcMatrix[15];

                nlStrNCpy<char>(pHelper->m_szName, (const char*)pData, 64);

                AVLTreeNode* pExistingNode;
                m_helperMap.AddAVLNode(ppHelperRoot, pHelper, &pHelper, &pExistingNode, m_helperMap.m_NumElements);
                if (pExistingNode == NULL)
                {
                    m_helperMap.m_NumElements++;
                }
            }
            break;
        }
        case 0x19100:
            break;
        case 0x19200:
            break;
        case 0x19201:
        {
            CreateHelperObjFromChunk(pChunk);
            break;
        }
        case (int)0x8001D000:
        {
            nlChunk* pInnerChunk = (nlChunk*)((u8*)pChunk + 8);
            nlChunk* pInnerEnd = (nlChunk*)((u8*)pChunk + pChunk->m_Size + 8);

            while (pInnerChunk != pInnerEnd)
            {
                int innerType = (int)(pInnerChunk->m_ID & 0x80FFFFFF);

                switch (innerType)
                {
                case 0x1D001:
                {
                    CharacterPhysicsData* pPhysData = (CharacterPhysicsData*)nlMalloc(sizeof(CharacterPhysicsData), 8, false);
                    if (pPhysData != NULL)
                    {
                        *(u32*)pPhysData = (u32)__vt__20CharacterPhysicsData;
                    }
                    *(CharacterPhysicsData**)((u8*)this + 0x134) = pPhysData;

                    u8* pInnerData = (u8*)nlGetChunkData(pInnerChunk);
                    s32 count = *(s32*)pInnerData;

                    pPhysData = *(CharacterPhysicsData**)((u8*)this + 0x134);
                    pPhysData->physicsElementCount = count;

                    pPhysData = *(CharacterPhysicsData**)((u8*)this + 0x134);
                    pPhysData->pPhysicsElements = (CharacterPhysicsElement*)nlMalloc(pPhysData->physicsElementCount * sizeof(CharacterPhysicsElement), 8, false);
                    break;
                }
                case 0x1D002:
                {
                    u8* pInnerData = (u8*)nlGetChunkData(pInnerChunk);
                    CharacterPhysicsData* pPhysData = *(CharacterPhysicsData**)((u8*)this + 0x134);
                    for (s32 i = 0; (u32)i < (u32)pPhysData->physicsElementCount; i++)
                    {
                        pPhysData->pPhysicsElements[i] = *(CharacterPhysicsElement*)pInnerData;
                        pInnerData += sizeof(CharacterPhysicsElement);
                    }
                    break;
                }
                }

                pInnerChunk = nlGetNextChunk(pInnerChunk);
            }
            break;
        }
        }

        pChunk = nlGetNextChunk(pChunk);
    }

    operator delete(pWorldData);

    // Iterative inorder AVL traversal to assign light render bits
    struct NodeStack
    {
        LightEntry** data;
        u32 count;
    };

    NodeStack* stack;
    LightEntry* node;

    stack = (NodeStack*)nlMalloc(sizeof(NodeStack), 8, false);
    if (stack != NULL)
    {
        u32 numElements = m_lightMap.m_NumElements;
        node = m_lightMap.m_Root;
        stack->data = (LightEntry**)nlMalloc((numElements + 1) * sizeof(LightEntry*), 8, false);
        stack->count = 0;

        if (node != NULL)
        {
            while (node->node.left != NULL)
            {
                stack->data[stack->count] = node;
                stack->count++;
                node = (LightEntry*)node->node.left;
            }
            stack->data[stack->count] = node;
            stack->count++;
        }
    }

    u32 bit = 1;
    while (stack->count > 0)
    {
        LightObject* pLight = stack->data[stack->count - 1]->value;
        pLight->m_bit = bit;
        bit <<= 1;

        stack->count--;

        LightEntry* right = (LightEntry*)stack->data[stack->count]->node.right;
        if (right != NULL)
        {
            while (right->node.left != NULL)
            {
                stack->data[stack->count] = right;
                stack->count++;
                right = (LightEntry*)right->node.left;
            }
            stack->data[stack->count] = right;
            stack->count++;
        }
    }

    if (stack != NULL)
    {
        delete[] stack->data;
        delete stack;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x3020 | 0x80197CE4 | size: 0x64
 */
void World::AddToHyperSTSDrawables(unsigned long key, DrawableModel* pDrawableModel)
{
    AVLTreeNode* pExistingNode;
    AVLTreeNode** ppRoot = (AVLTreeNode**)&m_hyperSTSDrawableMap.m_Root;

    m_hyperSTSDrawableMap.AddAVLNode(ppRoot, &key, &pDrawableModel, &pExistingNode, m_hyperSTSDrawableMap.m_NumElements);

    if (pExistingNode == nullptr)
    {
        m_hyperSTSDrawableMap.m_NumElements++;
    }
}

/**
 * Offset/Address/Size: 0x3084 | 0x80197D48 | size: 0x39C
 */
u8 World::HandleObjectCreation(WorldObjectData* pObjectData)
{
    struct WorldObjectDataLocal
    {
        unsigned long m_uObjectCreationFlags;
        unsigned long m_uHashID;
        unsigned long m_uModelID;
        unsigned long m_uShadowHashID;
        unsigned long m_uRenderLayer;
        nlMatrix4 m_worldMatrix;
        float m_fRadius;
        nlVector3 m_v3Offset;
    };

    extern unsigned long eOC_SHINY;

    WorldObjectDataLocal* pData = (WorldObjectDataLocal*)pObjectData;
    DrawableObject* pDrawable = NULL;
    glModel* pModel = glInventory.GetModel(pData->m_uModelID);

    if (pModel == NULL)
    {
        return 1;
    }

    if ((pData->m_uObjectCreationFlags & 0x10) == 0)
    {
        glModelPacket* pPacketEnd;
        glModelPacket* pPacket = pModel->packets;
        pPacketEnd = pPacket + pModel->numPackets;

        while (pPacket < pPacketEnd)
        {
            if (pPacket->state.texconfig & 0x10)
            {
                pData->m_uObjectCreationFlags |= eOC_SHINY;
                break;
            }
            pPacket++;
        }
    }

    if (pData->m_uObjectCreationFlags & 0x2)
    {
        DrawableSkinModel* pSkin = new (nlMalloc(sizeof(DrawableSkinModel), 8, false)) DrawableSkinModel();
        pSkin->m_pModel = pModel;
        pSkin->m_pShadowVolume = glInventory.GetShadowVolume(pData->m_uShadowHashID);
        pSkin->m_uObjectFlags |= 0x4;
        pDrawable = pSkin;
    }
    else if (pData->m_uObjectCreationFlags & 0x20)
    {
        DrawableTmModel* pTm = new (nlMalloc(sizeof(DrawableTmModel), 8, false)) DrawableTmModel();
        pTm->m_pModel = pModel;
        pTm->m_pShadowVolume = glInventory.GetShadowVolume(pData->m_uShadowHashID);
        pTm->m_uObjectFlags |= 0x4;
        pDrawable = pTm;
    }
    else if (pData->m_uObjectCreationFlags & 0x1)
    {
        DrawableShadow* pShadow = new (nlMalloc(sizeof(DrawableShadow), 8, false)) DrawableShadow();
        pShadow->m_pModel = pModel;
        pShadow->m_uObjectFlags &= ~0x4;
        pDrawable = pShadow;
    }
    else
    {
        DrawableModel* pModelDrawable = new (nlMalloc(sizeof(DrawableModel), 8, false)) DrawableModel();
        pModelDrawable->m_pModel = pModel;
        pModelDrawable->m_pShadowVolume = glInventory.GetShadowVolume(pData->m_uShadowHashID);
        pModelDrawable->m_bVertexAnimated = glInventory.GetVertexAnim(pModel->id) ? true : false;
        pModelDrawable->m_uObjectFlags &= ~0x4;
        pDrawable = pModelDrawable;
    }

    if (pDrawable == NULL)
    {
        return 0;
    }

    pDrawable->m_pWorldContext = this;
    pDrawable->m_uObjectFlags |= 0x1;
    pDrawable->m_uHashID = pData->m_uHashID;
    pDrawable->m_uObjectCreationFlags = pData->m_uObjectCreationFlags;
    pDrawable->m_uRenderLayer = pData->m_uRenderLayer;
    pDrawable->m_worldMatrix = pData->m_worldMatrix;
    pDrawable->m_fBoundingRadius = pData->m_fRadius;

    AVLTreeNode* pExistingNode;
    AVLTreeNode** ppRoot = (AVLTreeNode**)&m_drawableMap.m_Root;

    m_drawableMap.AddAVLNode(ppRoot, &pDrawable->m_uHashID, &pDrawable, &pExistingNode, m_drawableMap.m_NumElements);

    if (pExistingNode == NULL)
    {
        m_drawableMap.m_NumElements++;
    }

    return 1;
}

/**
 * Offset/Address/Size: 0x3420 | 0x801980E4 | size: 0x274
 * TODO: 97.48% match - remaining diffs are key/register ordering in the inlined DrawableMap FindGet path
 *       and float register assignment/order in bounding-radius max selection.
 */
bool World::LoadGeometry(glModel* gModel, unsigned long uNumModels, bool bMakeDrawables, bool keepTransform, unsigned long* pDrawableObjectHashes, int* pNumObjectsLoaded, bool bVar)
{
    struct WorldObjectDataLocal
    {
        unsigned long m_uObjectCreationFlags;
        unsigned long m_uHashID;
        unsigned long m_uModelID;
        unsigned long m_uShadowHashID;
        unsigned long m_uRenderLayer;
        nlMatrix4 m_worldMatrix;
        float m_fRadius;
        nlVector3 m_v3Offset;
    };

    struct glModelPacketMatrixRef
    {
        char pad[0x24];
        unsigned long matrix;
    };

    struct DrawableModelProxy
    {
        char pad[0x9C];
        glModel* m_pModel;
    };

    extern void glGetMatrix(unsigned long, nlMatrix4&);
    extern unsigned long eOC_ENV_SHINY;

    WorldObjectDataLocal data;
    AABBDimensions aabb;
    DrawableObject** foundValue;
    DrawableObject* pObject;

    m_pModels = gModel;
    m_uNumModels = uNumModels;

    if (bMakeDrawables)
    {
        unsigned long* pHash = pDrawableObjectHashes;
        glModel* pModelStart = m_pModels;
        nlMatrix4* pWorldMtx = &data.m_worldMatrix;
        glModel* pModel = pModelStart;
        bool keep = keepTransform;
        glModel* pEndModel = pModelStart + m_uNumModels;
        bool checkEnvShiny = bVar;
        int count = 0;
        float radius = 0.0f;

        while (pModel < pEndModel)
        {
            nlZeroMemory(&data, sizeof(WorldObjectDataLocal));

            data.m_uHashID = pModel->id;
            data.m_uModelID = pModel->id;
            data.m_uShadowHashID = (unsigned long)-1;

            if (keep)
            {
                glGetMatrix(((glModelPacketMatrixRef*)pModel->packets)->matrix, *pWorldMtx);
            }
            else
            {
                pWorldMtx->SetIdentity();
            }

            data.m_fRadius = radius;
            HandleObjectCreation((WorldObjectData*)&data);

            if (((DrawableMapFindHelper*)&m_drawableMap)->FindGet(pModel->id, &foundValue))
            {
                pObject = *foundValue;
            }
            else
            {
                pObject = NULL;
            }

            pObject->m_uObjectFlags |= 0x4;

            if (checkEnvShiny)
            {
                DrawableModel* model = pObject->AsDrawableModel();
                if (model != NULL)
                {
                    unsigned long envShiny = eOC_ENV_SHINY;
                    glModelPacket* pPacket = ((DrawableModelProxy*)model)->m_pModel->packets;
                    while (pPacket < ((DrawableModelProxy*)model)->m_pModel->packets + ((DrawableModelProxy*)model)->m_pModel->numPackets)
                    {
                        if (pPacket->state.texconfig & 0x10)
                        {
                            pObject->m_uObjectCreationFlags |= envShiny;
                        }
                        pPacket++;
                    }
                }
            }

            pObject->GetAABBDimensions(aabb, false);

            /**
             * TODO: 97.83% match - r0/r4 register swap in DrawableMapFindHelper::FindGet
             * inlined BST lookup (MWCC volatile register allocation quirk, 7 diffs) and
             * ble-+b vs bgt- branch structure difference (MWCC optimizes && to || negation, 3 diffs)
             */
            {
                float dimX = aabb.mDim.f.x;
                if (!(dimX >= aabb.mDim.f.y) || !(dimX > aabb.mDim.f.z))
                {
                    dimX = aabb.mDim.f.z;
                    if (aabb.mDim.f.y >= dimX)
                    {
                        dimX = aabb.mDim.f.y;
                    }
                }
                pObject->m_fBoundingRadius = dimX;
            }

            if (pDrawableObjectHashes != NULL)
            {
                *pHash = pObject->m_uHashID;
            }

            pHash++;
            count++;
            pModel++;
        }

        if (pNumObjectsLoaded != NULL)
        {
            *pNumObjectsLoaded = count;
        }
    }

    return m_pModels != NULL;
}

/**
 * Offset/Address/Size: 0x3694 | 0x80198358 | size: 0xD8
 */
bool World::LoadGeometry(const char* szWorldName, bool bMakeDrawables, bool keepTransform, unsigned long* pDrawableObjectHashes, int* pNumObjectsLoaded)
{
    char buffer[256];

    nlSNPrintf(buffer, 0xFF, "%s.glt", szWorldName);
    tDebugPrintManager::Print(DC_RENDER, "Loading world texture file: %s\n", buffer);
    glLoadTextureBundle(buffer);

    nlSNPrintf(buffer, 0xFF, "%s.glg", szWorldName);
    tDebugPrintManager::Print(DC_RENDER, "Loading world geometry file: %s\n", buffer);

    m_pModels = glLoadModel(buffer, &m_uNumModels);

    // return LoadGeometry(m_pModels, m_uNumModels, bMakeDrawables, keepTransform, pNumObjectsLoaded, pDrawableObjectHashes, false);
    return LoadGeometry(m_pModels, m_uNumModels, bMakeDrawables, keepTransform, pDrawableObjectHashes, pNumObjectsLoaded, false);
}

/**
 * Offset/Address/Size: 0x376C | 0x80198430 | size: 0x68
 */
bool World::Load(bool forfe)
{
    bool ret = DoLoad();
    if (forfe == 0)
    {
        ret = DoInitialize();
    }
    m_Locked = 1;
    return ret;
}

/**
 * Offset/Address/Size: 0x37D4 | 0x80198498 | size: 0x580
 * TODO: 97.17% match - register swap (r29/r31 used for this/pStack are swapped)
 * and one missing duplicate addic./beq pair in auto-generated m_animControllerList
 * destructor unwinding.
 */
World::~World()
{
    typedef AVLTreeEntry<unsigned long, DrawableObject*> DrEntry;
    typedef AVLTreeEntry<unsigned long, LightObject*> LiEntry;
    typedef AVLTreeEntry<unsigned long, HelperObject*> HeEntry;

    u32* pStack = (u32*)nlMalloc(8, 8, false);
    if (pStack != NULL)
    {
        DrEntry* pNode = m_drawableMap.m_Root;
        pStack[0] = (u32)nlMalloc((m_drawableMap.m_NumElements + 1) * 4, 8, false);
        pStack[1] = 0;
        if (pNode != NULL)
        {
            while (pNode->node.left != NULL)
            {
                ((DrEntry**)pStack[0])[pStack[1]] = pNode;
                pStack[1]++;
                pNode = (DrEntry*)pNode->node.left;
            }
            ((DrEntry**)pStack[0])[pStack[1]] = pNode;
            pStack[1]++;
        }
    }
    while (pStack[1] != 0)
    {
        DrEntry* topNode = ((DrEntry**)pStack[0])[pStack[1] - 1];
        delete topNode->value;
        pStack[1]--;
        DrEntry* pRight = (DrEntry*)((DrEntry**)pStack[0])[pStack[1]]->node.right;
        if (pRight != NULL)
        {
            while (pRight->node.left != NULL)
            {
                ((DrEntry**)pStack[0])[pStack[1]] = pRight;
                pStack[1]++;
                pRight = (DrEntry*)pRight->node.left;
            }
            ((DrEntry**)pStack[0])[pStack[1]] = pRight;
            pStack[1]++;
        }
    }
    m_drawableMap.Clear();
    m_hyperSTSDrawableMap.Clear();
    if (pStack != NULL)
    {
        delete[] (u8*)pStack[0];
        delete (u8*)pStack;
    }

    pStack = (u32*)nlMalloc(8, 8, false);
    if (pStack != NULL)
    {
        LiEntry* pLightNode = m_lightMap.m_Root;
        pStack[0] = (u32)nlMalloc((m_lightMap.m_NumElements + 1) * 4, 8, false);
        pStack[1] = 0;
        if (pLightNode != NULL)
        {
            while (pLightNode->node.left != NULL)
            {
                ((LiEntry**)pStack[0])[pStack[1]] = pLightNode;
                pStack[1]++;
                pLightNode = (LiEntry*)pLightNode->node.left;
            }
            ((LiEntry**)pStack[0])[pStack[1]] = pLightNode;
            pStack[1]++;
        }
    }
    while (pStack[1] != 0)
    {
        LiEntry* topNode = ((LiEntry**)pStack[0])[pStack[1] - 1];
        delete topNode->value;
        pStack[1]--;
        LiEntry* pRight = (LiEntry*)((LiEntry**)pStack[0])[pStack[1]]->node.right;
        if (pRight != NULL)
        {
            while (pRight->node.left != NULL)
            {
                ((LiEntry**)pStack[0])[pStack[1]] = pRight;
                pStack[1]++;
                pRight = (LiEntry*)pRight->node.left;
            }
            ((LiEntry**)pStack[0])[pStack[1]] = pRight;
            pStack[1]++;
        }
    }
    m_lightMap.Clear();
    if (pStack != NULL)
    {
        delete[] (u8*)pStack[0];
        delete (u8*)pStack;
    }

    DLListEntry<WorldAnimController*>* pCurrent = nlDLRingGetStart(m_animControllerList.m_Head);
    DLListEntry<WorldAnimController*>* pHead = m_animControllerList.m_Head;
    while (pCurrent != NULL)
    {
        delete pCurrent->m_data;
        if ((nlDLRingIsEnd(pHead, pCurrent) != 0) || (pCurrent == NULL))
        {
            pCurrent = NULL;
        }
        else
        {
            pCurrent = pCurrent->m_next;
        }
    }

    pStack = (u32*)nlMalloc(8, 8, false);
    if (pStack != NULL)
    {
        HeEntry* pHelperNode = m_helperMap.m_Root;
        pStack[0] = (u32)nlMalloc((m_helperMap.m_NumElements + 1) * 4, 8, false);
        pStack[1] = 0;
        if (pHelperNode != NULL)
        {
            while (pHelperNode->node.left != NULL)
            {
                ((HeEntry**)pStack[0])[pStack[1]] = pHelperNode;
                pStack[1]++;
                pHelperNode = (HeEntry*)pHelperNode->node.left;
            }
            ((HeEntry**)pStack[0])[pStack[1]] = pHelperNode;
            pStack[1]++;
        }
    }
    while (pStack[1] != 0)
    {
        HeEntry* topNode = ((HeEntry**)pStack[0])[pStack[1] - 1];
        delete topNode->value;
        pStack[1]--;
        HeEntry* pRight = (HeEntry*)((HeEntry**)pStack[0])[pStack[1]]->node.right;
        if (pRight != NULL)
        {
            while (pRight->node.left != NULL)
            {
                ((HeEntry**)pStack[0])[pStack[1]] = pRight;
                pStack[1]++;
                pRight = (HeEntry*)pRight->node.left;
            }
            ((HeEntry**)pStack[0])[pStack[1]] = pRight;
            pStack[1]++;
        }
    }
    if (pStack != NULL)
    {
        delete[] (u8*)pStack[0];
        delete (u8*)pStack;
    }
    m_helperMap.Clear();
    delete m_pWorldAnimManager;
    delete *(CharacterPhysicsData**)((u8*)this + 0x134);
}

/**
 * Offset/Address/Size: 0x3D54 | 0x80198A18 | size: 0x60
 */
template <>
nlAVLTree<unsigned long, LightObject*, DefaultKeyCompare<unsigned long> >::~nlAVLTree()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x3DB4 | 0x80198A78 | size: 0x19C
 */
World::World(const char* szWorldName)
    : m_pWorldAnimManager(NULL)
    , m_Locked(false)
    , m_pModels(NULL)
    , m_uNumModels(0)
{
    m_WorldNameLength = nlStrLen<char>(szWorldName);
    nlStrNCpy<char>(m_WorldNamePrefix, szWorldName, 0x40);

    m_WorldNamePrefix[m_WorldNameLength++] = '/';

    m_pWorldAnimManager = new (nlMalloc(sizeof(WorldAnimManager), 8, false)) WorldAnimManager();

    m_pIntensityPerm = NULL;

    m_LightRampTexA = glGetTexture("global/lightramp");
    m_LightRampTexB = m_LightRampTexA;

    m_PlayerLightRampTex = glGetTexture("global/playerlightramp");
    if (glTextureLoad(m_PlayerLightRampTex) == false)
    {
        m_PlayerLightRampTex = m_LightRampTexA;
    }

    m_GlobalLightRampSTSTex = m_LightRampTexA;
}

/**
 * Offset/Address/Size: 0x0 | 0x80198C14 | size: 0x4
 */
void World::FixedUpdate(float dt)
{
}

/**
 * Offset/Address/Size: 0x4 | 0x80198C18 | size: 0x4
 */
void World::HandleEvent(Event*, void*)
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80198C1C | size: 0x24
//  */
// void AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned
// long>>::DeleteEntry(AVLTreeEntry<unsigned long, DrawableObject*>*)
// {
// }

/**
 * Offset/Address/Size: 0x24 | 0x80198C40 | size: 0x5C
 */
template <>
AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*> >, DefaultKeyCompare<unsigned long> >::~AVLTreeBase()
{
    FORCE_DONT_INLINE;
    Clear();
}

/**
 * Offset/Address/Size: 0x80 | 0x80198C9C | size: 0x60
 */
template <>
nlAVLTree<unsigned long, DrawableObject*, DefaultKeyCompare<unsigned long> >::~nlAVLTree()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0xE0 | 0x80198CFC | size: 0x5C
 */
template <>
AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*> >, DefaultKeyCompare<unsigned long> >::~AVLTreeBase()
{
    FORCE_DONT_INLINE;
    Clear();
}

/**
 * Offset/Address/Size: 0x13C | 0x80198D58 | size: 0x60
 */
template <>
nlAVLTree<unsigned long, HelperObject*, DefaultKeyCompare<unsigned long> >::~nlAVLTree()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x19C | 0x80198DB8 | size: 0x5C
 */
template <>
AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*> >, DefaultKeyCompare<unsigned long> >::~AVLTreeBase()
{
    FORCE_DONT_INLINE;
    Clear();
}

// /**
//  * Offset/Address/Size: 0x1F8 | 0x80198E14 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned
// long>>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x80198E6C | size: 0x64
//  */
// void AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned
// long>>::DestroyTree(void (AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>,
// DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, HelperObject*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x2B4 | 0x80198ED0 | size: 0x758
//  */
// void AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned
// long>>::PostorderTraversal(AVLTreeEntry<unsigned long, HelperObject*>*, void (AVLTreeBase<unsigned long, HelperObject*,
// NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long,
// HelperObject*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xA0C | 0x80199628 | size: 0x8
//  */
// void AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned
// long>>::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0xA14 | 0x80199630 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned
// long>>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0xA6C | 0x80199688 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned
// long>>::DestroyTree(void (AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>,
// DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, LightObject*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xAD0 | 0x801996EC | size: 0x758
//  */
// void AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned
// long>>::PostorderTraversal(AVLTreeEntry<unsigned long, LightObject*>*, void (AVLTreeBase<unsigned long, LightObject*,
// NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, LightObject*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x1228 | 0x80199E44 | size: 0x8
//  */
// void AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned
// long>>::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x1230 | 0x80199E4C | size: 0x58
//  */
// void AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned
// long>>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x1288 | 0x80199EA4 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned
// long>>::DestroyTree(void (AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>,
// DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, DrawableObject*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x12EC | 0x80199F08 | size: 0x758
//  */
// void AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned
// long>>::PostorderTraversal(AVLTreeEntry<unsigned long, DrawableObject*>*, void (AVLTreeBase<unsigned long, DrawableObject*,
// NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long,
// DrawableObject*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x1A44 | 0x8019A660 | size: 0x8
//  */
// void AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned
// long>>::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x1A4C | 0x8019A668 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned
// long>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1A78 | 0x8019A694 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned
// long>>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1AA4 | 0x8019A6C0 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned
// long>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1AD0 | 0x8019A6EC | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned
// long>>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1AFC | 0x8019A718 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned
// long>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1B28 | 0x8019A744 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned
// long>>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1B54 | 0x8019A770 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned
// long>>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1BB8 | 0x8019A7D4 | size: 0x24
//  */
// void AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned
// long>>::DeleteEntry(AVLTreeEntry<unsigned long, LightObject*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1BDC | 0x8019A7F8 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned
// long>>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1C40 | 0x8019A85C | size: 0x64
//  */
// void AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned
// long>>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1CA4 | 0x8019A8C0 | size: 0x24
//  */
// void AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned
// long>>::DeleteEntry(AVLTreeEntry<unsigned long, HelperObject*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8019A8E4 | size: 0x24
 * ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*>>>::DeleteEntry(ListEntry<LightObject*>*)
 */

// /**
//  * Offset/Address/Size: 0x0 | 0x8019A908 | size: 0x24
//  */
// void DLListContainerBase<WorldAnimController*,
// NewAdapter<DLListEntry<WorldAnimController*>>>::DeleteEntry(DLListEntry<WorldAnimController*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8019A92C | size: 0x68
//  */
// void nlWalkList<ListEntry<LightObject*>, ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*>>>>(ListEntry<LightObject*>*,
// ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*>>>*, void (ListContainerBase<LightObject*,
// NewAdapter<ListEntry<LightObject*>>>::*)(ListEntry<LightObject*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x8019A994 | size: 0x28
//  */
// void nlListAddStart<ListEntry<LightObject*>>(ListEntry<LightObject*>**, ListEntry<LightObject*>*, ListEntry<LightObject*>**)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8019A9BC | size: 0x3C
 * TODO: 96.00% match - prologue scheduling mismatch remains.
 * Target orders `lwz r7, 0(r5)` before saving LR; current MWCC output saves LR first.
 */
template void nlWalkDLRing<DLListEntry<WorldAnimController*>, DLListContainerBase<WorldAnimController*,
                                                                  NewAdapter<DLListEntry<WorldAnimController*> > > >(
    DLListEntry<WorldAnimController*>* head,
    DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*> > >* callback,
    void (DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*> > >::*)(DLListEntry<WorldAnimController*>*));

// /**
//  * Offset/Address/Size: 0x3C | 0x8019A9F8 | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<WorldAnimController*>>(DLListEntry<WorldAnimController*>*, DLListEntry<WorldAnimController*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x5C | 0x8019AA18 | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<WorldAnimController*>>(DLListEntry<WorldAnimController*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8019AA30 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<WorldAnimController*>, DLListContainerBase<WorldAnimController*,
// NewAdapter<DLListEntry<WorldAnimController*>>>>(DLListEntry<WorldAnimController*>*, DLListContainerBase<WorldAnimController*,
// NewAdapter<DLListEntry<WorldAnimController*>>>*, void (DLListContainerBase<WorldAnimController*,
// NewAdapter<DLListEntry<WorldAnimController*>>>::*)(DLListEntry<WorldAnimController*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8019AA90 | size: 0x2C
//  */
// void nlStrChr<char>(const char*, char)
// {
// }

// /**
//  * Offset/Address/Size: 0x2C | 0x8019AABC | size: 0x5C
//  */
// void 0x8028D34C..0x8028D350 | size: 0x4
// {
// }

// REMOVE once real callers exist.
void World_stub()
{
    nlListContainer<LightObject*> list;
}

template void nlListAddStart<ListEntry<LightObject*> >(ListEntry<LightObject*>**, ListEntry<LightObject*>*, ListEntry<LightObject*>**);
