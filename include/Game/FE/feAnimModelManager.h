#ifndef _FEANIMMODELMANAGER_H_
#define _FEANIMMODELMANAGER_H_

#include "types.h"
#include "NL/nlSingleton.h"
#include "Game/Inventory.h"

class FEAnimModel;
class FEBasic3dModel;
class cSAnim;

class FEAnimModelManager : public nlSingleton<FEAnimModelManager>
{
public:
    virtual ~FEAnimModelManager();
    FEAnimModelManager();

    void Update(float);
    void Initialize();

    /* 0x04 */ FEAnimModel* mLoadedModels[22];
    /* 0x5C */ void* mLightData;
    /* 0x60 */ void* mSpecularData;
    /* 0x64 */ FEBasic3dModel* mFEAnimModelTweakList;
    /* 0x68 */ FEBasic3dModel* mCurrentTweakModel;
    /* 0x6C */ cInventory<cSAnim> mAnimInventory;
    /* 0x88 */ u8 mTweak3dModels;
};

#endif // _FEANIMMODELMANAGER_H_
