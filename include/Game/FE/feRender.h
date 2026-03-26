#ifndef _FERENDER_H_
#define _FERENDER_H_

#include "NL/gl/glMatrixStack.h"
#include "NL/nlColour.h"

void ConvertColour(nlColour&, const nlFloatColour&);

class TLImageInstance;
class TLTextInstance;
class FEScene;
class FEPresentation;
class TLComponentInstance;
class TLSlide;
class TLInstance;

class FERender
{
public:
    static unsigned char RenderImageInstance(const TLImageInstance*);
    static void RenderTextInstance(TLTextInstance*);
    static void RenderScene(FEScene*);
    static void RenderPresentation(const FEPresentation*);
    static void RenderComponentInstance(TLComponentInstance*);
    static void RenderSlide(const TLSlide*);
    static void RenderTimeLineAsset(TLInstance*, float);
    static void PopTransformMatrix();
    static void PushTransformMatrix(const TLInstance*);
    static void Initialize();
    static void Cleanup();
    static void CalculateCurrentAssetColour(const TLInstance*);

    static GLMatrixStack* m_pMatrixStack;
    static FEScene* m_pRenderScene;
};

// class nlFloatColour
// {
// public:
//     void operator=(const nlFloatColour&);
// };

// class TLInstance
// {
// public:
//     void GetType() const;
//     void IsVisible() const;
//     void GetLibRefObject() const;
// };

// class TLComponent
// {
// public:
//     void GetActiveSlide();
// };

// class FEScene
// {
// public:
//     void GetRenderView() const;
//     void GetCameraMatrix() const;
// };

// class TLTextInstance
// {
// public:
//     void SetMatrix(nlMatrix4*);
// };

// class nlMatrix4
// {
// public:
//     void SetTranslation(const nlVector3&);
// };

// class feVector3
// {
// public:
//     void GetNLVector3(nlVector3&) const;
// };

// class TLSlide
// {
// public:
//     void GetCurrentTime() const;
// };

#endif // _FERENDER_H_
