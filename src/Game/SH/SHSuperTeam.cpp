#include "Game/SH/SHSuperTeam.h"

#include "Game/FE/feFinder.h"
#include "Game/FE/feInput.h"
#include "Game/GameSceneManager.h"
#include "Game/SH/SHCupHub.h"
#include "Game/Audio/WorldAudio.h"

// /**
//  * Offset/Address/Size: 0xBC | 0x800F6184 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800F6100 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F60C8 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800F6050 | size: 0x78
 */
SuperTeamScene::~SuperTeamScene()
{
}

/**
 * Offset/Address/Size: 0xC0 | 0x800F5FB4 | size: 0x9C
 */
void SuperTeamScene::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();

    TLSlide* slide;
    FEPresentation* pres;

    pres = m_pFEPresentation;
    slide = pres->m_currentSlide;

    if (pres->m_fadeDuration < slide->m_start + slide->m_duration)
    {
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        CupHubScene* scene = (CupHubScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_STANDINGS, SCREEN_NOTHING, true);
        scene->mDoAutoSave = true;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800F5EF4 | size: 0xC0
 */
void SuperTeamScene::SceneCreated()
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA, h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    unsigned long hash1 = nlStringLowerHash("buttons");
    h8.m_Hash = hash1;
    h9.m_Hash = hash1;

    unsigned long hash2 = nlStringLowerHash("Layer");
    hB.m_Hash = hash2;
    hA.m_Hash = hash2;

    mButtons.mButtonInstance = findComp.byRef(
        m_pFEPresentation->m_currentSlide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    mButtons.SetState(ButtonComponent::BS_A_ONLY);

    Audio::gWorldSFX.Play((Audio::eWorldSFX)27, 100.0f, -1.0f, true, 100.0f);
}
