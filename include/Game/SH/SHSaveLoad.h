#ifndef _SHSAVELOAD_H_
#define _SHSAVELOAD_H_

#include "Game/BaseGameSceneManager.h"
#include "Game/FE/feButtonComponent.h"

bool PushNoCardMessage();
void CheckResults();
void CreateFileAndSaveCB();
void OverwriteFileAndContinueCB();
void ManageMemCardCB();
void FormatCB();
void FormatConfirmCB();
void DeleteFileCB();
void RetryCB();
void ContinueLoadingCB();
void ContinueWithoutLoadingCB();
void ContinueWithoutSavingCB();
void SaveLoadCallback(long);
void ResetEnableSaveLoadFlag();
bool DidContinueWithoutOperation();

class SaveLoadScene : public BaseSceneHandler
{
public:
    enum eSaveLoadMode
    {
        SLM_INVALID = -1,
        SLM_AT_BOOT = 0,
        SLM_SAVING = 1,
        SLM_ASK_BEFORE_SAVING = 2,
        SLM_LOADING = 3,
        SLM_ASK_BEFORE_LOADING = 4,
    };

    SaveLoadScene(SaveLoadScene::eSaveLoadMode);
    ~SaveLoadScene();
    void SceneCreated();
    void Update(float);
    static bool IsIOEnabled();
    void SetupForAboutAutoSave();
    void UpdateForAboutToSaveSlide();
    void HandleSaveLoadFinishedResult();

    static void StartSaveNow();
    static void UpdateCardRemovedFlag();

    /* 0x1C */ TLTextInstance* m_displayText;
    /* 0x20 */ SceneList mNextScene;
    /* 0x24 */ bool mIsAutoSaving;
    /* 0x25 */ bool mIsFirstTimeCreateFile;
    /* 0x28 */ TLSlide* mAboutAutoSaveSlide;
    /* 0x2C */ ButtonComponent* mButtonComponent;
    /* 0x30 */ eSaveLoadMode mSaveLoadMode;

    static SaveLoadScene* mInstance;
    static bool mLastSaveLoadSuccess;
    static bool mIsFirstTimeAboutIPL;
}; // total size: 0x34

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLSlide, 0>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//     InlineHasher);
// };

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//     InlineHasher);
// };

#endif // _SHSAVELOAD_H_
