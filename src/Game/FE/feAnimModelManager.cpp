#include "Game/FE/feAnimModelManager.h"
#include "Game/FE/feBasic3dModel.h"
#include "Game/SAnim.h"
#include "NL/globalpad.h"
#include "NL/nlColour.h"
#include "NL/nlPrint.h"
#include "NL/gl/gl.h"
#include "NL/gl/glFont.h"
#include "NL/gl/glState.h"

void DrawTextRectangle(int, float, float, float, float, float, const nlColour&, bool);

FEAnimModelManager* nlSingleton<FEAnimModelManager>::s_pInstance = NULL;

// Layout-only stub (unreferenced -> mwld drops it). The ~cInventory<cSAnim>
// dtor mints the two DeleteEntry PTMF consts in the wrong .data order
// (cSAnim*-list before char*-list) because its body walks the item list first.
// Minting them here in target order -- char* (mem) list first, then cSAnim*
// (item) list -- fixes the .data anon sequence; the real uses pool to these
// slots. Same trick as CharacterTemplate_stub.
typedef ListContainerBase<char*, NewAdapter<ListEntry<char*> > > FEMemListBase;
typedef ListContainerBase<cSAnim*, NewAdapter<ListEntry<cSAnim*> > > FEItemListBase;

void feAnimModelManager_stub()
{
    void (FEMemListBase::*forceMemDelete)(ListEntry<char*>*) = FEMemListBase::DeleteEntryFunc();
    void (FEItemListBase::*forceItemDelete)(ListEntry<cSAnim*>*) = FEItemListBase::DeleteEntryFunc();
    (void)forceMemDelete;
    (void)forceItemDelete;
}

/**
 * Offset/Address/Size: 0x3E8 | 0x80094B94 | size: 0xB0
 */
FEAnimModelManager::FEAnimModelManager()
    : mLightData(NULL)
    , mFEAnimModelTweakList(NULL)
    , mCurrentTweakModel(NULL)
    , mTweak3dModels(false)
{
    for (int i = 0; i < 22; i++)
    {
        mLoadedModels[i] = NULL;
    }
}

/**
 * Offset/Address/Size: 0x214 | 0x800949C0 | size: 0x1D4
 */
FEAnimModelManager::~FEAnimModelManager()
{
    for (int i = 0; i < 22; i++)
    {
        if (mLoadedModels[i] != NULL)
        {
            delete (FEBasic3dModel*)mLoadedModels[i];
            mLoadedModels[i] = NULL;
        }
    }
}

/**
 * Offset/Address/Size: 0x210 | 0x800949BC | size: 0x4
 */
void FEAnimModelManager::Initialize()
{
}

inline void FEAnimModelManager::CheckTweakModelSwitch()
{
    cGlobalPad* pad = cPadManager::GetPad(1);
    if (pad != NULL)
    {
        if (pad->JustPressed(8, true))
        {
            if (pad->GetPressure(0x15, true) > 0.8f)
            {
                FEAnimModelManager* inst = nlSingleton<FEAnimModelManager>::s_pInstance;
                if (inst->mCurrentTweakModel == NULL)
                {
                    inst->mCurrentTweakModel = inst->mFEAnimModelTweakList;
                }
                else
                {
                    inst->mCurrentTweakModel = inst->mCurrentTweakModel->next;
                }
            }
        }
    }
}

inline void FEAnimModelManager::TweakModelPosition()
{
    FEBasic3dModel* model = mCurrentTweakModel;
    if (model == NULL)
        return;

    cGlobalPad* pad = cPadManager::GetPad(1);
    if (pad == NULL)
        return;

    model->mPosition.f.x += -0.05f * pad->AnalogLeftX();
    model->mPosition.f.z += 0.05f * pad->AnalogLeftY();
    model->mPosition.f.y += 0.05f * pad->AnalogRightY();
    model->mRotation.f.z -= pad->GetPressure(1, true);
    model->mRotation.f.z += pad->GetPressure(0, true);

    DrawTextRectangle(GLV_Debug, 0.0f, 0.0f, 45.0f, 1.0f, 0.0f, (nlColour) { 0x00, 0x00, 0x00, 0xC8 }, true);

    glStateBundle state;
    glStateSave(state);
    glFontBegin(false);

    char tempbuffer[128];
    nlSNPrintf(tempbuffer, 128, "pos: %3.2f, %3.2f, %3.2f -- rot-z: %3.2f", model->mPosition.f.x, model->mPosition.f.y, model->mPosition.f.z, model->mRotation.f.z);

    glFontPrint(GLV_Debug, 0, 0, (nlColour) { 0xFF, 0xFF, 0xFF, 0xFF }, tempbuffer);
    glFontEnd();
    glStateRestore(state);
}

/**
 * Offset/Address/Size: 0x0 | 0x800947AC | size: 0x210
 */
void FEAnimModelManager::Update(float)
{
    if (!mTweak3dModels)
        return;

    CheckTweakModelSwitch();
    TweakModelPosition();
}
