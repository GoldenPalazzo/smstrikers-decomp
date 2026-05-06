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

/**
 * Offset/Address/Size: 0x0 | 0x800947AC | size: 0x210
 */
void FEAnimModelManager::Update(float)
{
    if (!mTweak3dModels)
        return;

    cGlobalPad* pad = cPadManager::GetPad(1);
    if (pad != NULL)
    {
        if (pad->JustPressed(8, true))
        {
            if (pad->GetPressure(0x15, true) > 0.0f)
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

    FEBasic3dModel* model = mCurrentTweakModel;
    if (model == NULL)
        return;

    pad = cPadManager::GetPad(1);
    if (pad == NULL)
        return;

    model->mPosition.f.x += 0.01f * pad->AnalogLeftX();
    model->mPosition.f.z += 0.005f * pad->AnalogLeftY();
    model->mPosition.f.y += 0.005f * pad->AnalogRightY();
    model->mRotation.f.z -= pad->GetPressure(1, true);
    model->mRotation.f.z += pad->GetPressure(0, true);

    nlColour rectanglecolour;
    nlColour debugtextcolour = { 0x80, 0x80, 0x80, 0x80 };
    DrawTextRectangle(GLV_Debug, 0.0f, 0.0f, 0.5f, 0.25f, 0.0f, debugtextcolour, true);

    glStateBundle state;
    glStateSave(state);
    glFontBegin(false);

    char tempbuffer[128];
    nlSNPrintf(tempbuffer, 128, "x=%.2f y=%.2f z=%.2f rot=%.2f", model->mPosition.f.x, model->mPosition.f.y, model->mPosition.f.z, model->mRotation.f.z);

    static const nlColour kRectColour = { 0xFF, 0xFF, 0x00, 0xFF };
    rectanglecolour = kRectColour;
    glFontPrint(GLV_Debug, 0, 0, rectanglecolour, tempbuffer);
    glFontEnd();
    glStateRestore(state);
}

/**
 * Offset/Address/Size: 0x210 | 0x800949BC | size: 0x4
 */
void FEAnimModelManager::Initialize()
{
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
