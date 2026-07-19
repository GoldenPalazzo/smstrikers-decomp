#ifndef _OVERLAYHANDLERSTSX2_H_
#define _OVERLAYHANDLERSTSX2_H_

#include "Game/FE/BaseOverlayHandler.h"
#include "Game/Sys/eventman.h"

class STSX2Overlay : public BaseOverlayHandler
{
public:
    STSX2Overlay();
    virtual ~STSX2Overlay();
    virtual void Update(float);
    virtual void SceneCreated();
    static void EventHandlerFunc(Event*, void*);

    EventHandler* m_EventHandler;
};

#endif // _OVERLAYHANDLERSTSX2_H_
