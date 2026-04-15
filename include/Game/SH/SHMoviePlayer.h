#ifndef _SHMOVIEPLAYER_H_
#define _SHMOVIEPLAYER_H_

#include "Game/BaseGameSceneManager.h"
#include "Game/BaseSceneHandler.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feRender.h"
#include "Game/GameInfo.h"

class MoviePlayerScene : public BaseSceneHandler
{
public:
    MoviePlayerScene();
    virtual ~MoviePlayerScene();
    virtual void SceneCreated();
    void SetMovieDetails(const char*, bool, bool);
    virtual void Update(float);
    virtual bool CheckMoviePlayerAbort();
    virtual void PlayScreenForwardSFX();
    virtual void PlayScreenBackSFX();
    virtual void OverrideMovieDimensions();

    bool mSwappedTexture;                    // offset 0x1C, size 0x1
    bool mMovieStarted;                      // offset 0x1D, size 0x1
    TLImageInstance* mMovieInstance;         // offset 0x20, size 0x4
    SceneList mNextScene;                    // offset 0x24, size 0x4
    char mMovieFilename[128];                // offset 0x28, size 0x80
    bool mWithSound;                         // offset 0xA8, size 0x1
    bool mLoopMovie;                         // offset 0xA9, size 0x1
    bool mPushWithPop;                       // offset 0xAA, size 0x1
    BaseGameSceneManager* mGameSceneManager; // offset 0xAC, size 0x4
};

class IntroMovieScene : public MoviePlayerScene
{
public:
    IntroMovieScene()
    {
        SetMovieDetails("intromovie.thp", true, false);
        mNextScene = SCENE_TITLE;
    }
    virtual ~IntroMovieScene() { };
    virtual void PlayScreenBackSFX();
    virtual void PlayScreenForwardSFX() { };
    virtual void OverrideMovieDimensions();
};

class NLGLogoMovieScene : public MoviePlayerScene
{
public:
    NLGLogoMovieScene()
    {
        if (nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mVisualOptions.mIsWidescreen)
        {
            SetMovieDetails("movies/nlgintrowide.thp", true, false);
        }
        else
        {
            SetMovieDetails("movies/nlgintrofull.thp", true, false);
        }
        mNextScene = SCENE_CREDITS;
    }
    virtual ~NLGLogoMovieScene() { };
    virtual void PlayScreenBackSFX() { };
    virtual void PlayScreenForwardSFX() { };
    virtual void OverrideMovieDimensions();
};

class LessonMoviePlayerScene : public MoviePlayerScene
{
public:
    virtual ~LessonMoviePlayerScene() { };
    virtual void SceneCreated();
    virtual bool CheckMoviePlayerAbort();
    virtual void Update(float);

    ButtonComponent mButtonComponent;
};

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLImageInstance, 2>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

#endif // _SHMOVIEPLAYER_H_
