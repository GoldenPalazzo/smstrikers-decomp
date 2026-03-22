#include "Game/FE/feInGameMessengerManager.h"
#include "Game/FE/feIMessenger.h"
#include "Game/Game.h"
#include "Game/Ball.h"

/**
 * Offset/Address/Size: 0x0 | 0x800FF91C | size: 0x140
 * TODO: 97.31% match - MWCC strength-reduces TIMESTATE_TIMES loop:
 * pre-computes base+offset outside loop (r0) instead of keeping base in r6
 * and recomputing add inside loop body (5 diffs). Remaining 2 diffs:
 * loop-back target and 25.0f literal pool symbol.
 */
void FEInGameMessengerManager::Update(float fDeltaT)
{
    if (m_waitingToDisplay)
    {
        m_waitedToDisplay += fDeltaT;
    }

    float normTime = g_pGame->GetNormalizedGameTime();
    eTimeStates newState = m_curTimeState;
    int offset = (int)newState;
    float (*pTimes)[TS_NUMTIMESTATES] = (float (*)[TS_NUMTIMESTATES]) & TIMESTATE_TIMES;

    while (offset + 1 != (int)TS_NUMTIMESTATES)
    {
        if (normTime >= (*pTimes)[offset + 1])
        {
            offset++;
            newState = (eTimeStates)offset;
        }
        else
        {
            break;
        }
    }

    if (newState != m_curTimeState)
    {
        EnterNewTimeState(newState);
    }

    if (m_messageQueue.m_Head == NULL)
    {
        return;
    }

    if (m_messenger->IsMessengerOpen())
    {
        return;
    }

    if (!(m_waitedToDisplay > 25.f))
    {
        if (g_pBall->GetOwnerGoalie() == NULL)
        {
            return;
        }
    }

    ListEntry<eInGameMessages>* entry = nlListRemoveStart<ListEntry<eInGameMessages> >(&m_messageQueue.m_Head, &m_messageQueue.m_Tail);
    eInGameMessages msg;
    eInGameMessages* pMsg = &msg;
    if (pMsg != NULL)
    {
        msg = entry->data;
    }
    delete entry;

    m_messenger->SetDisplayMessage(m_messageList[(int)msg]);
    m_messenger->OpenMessenger();
    m_waitingToDisplay = false;
}

/**
 * Offset/Address/Size: 0x140 | 0x800FFA5C | size: 0x414
 */
void FEInGameMessengerManager::EnterNewTimeState(FEInGameMessengerManager::eTimeStates)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x554 | 0x800FFE70 | size: 0x104
 */
FEInGameMessengerManager::~FEInGameMessengerManager()
{
    Function<FnVoidVoid> cb;
    m_messenger->SetMessageFinishedCB(cb);
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800FFF74 | size: 0x24
//  */
// void ListContainerBase<FEInGameMessengerManager::eInGameMessages,
// NewAdapter<ListEntry<FEInGameMessengerManager::eInGameMessages>>>::DeleteEntry(ListEntry<FEInGameMessengerManager::eInGameMessages>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800FFF98 | size: 0x68
//  */
// void nlWalkList<ListEntry<FEInGameMessengerManager::eInGameMessages>, ListContainerBase<FEInGameMessengerManager::eInGameMessages,
// NewAdapter<ListEntry<FEInGameMessengerManager::eInGameMessages>>>>(ListEntry<FEInGameMessengerManager::eInGameMessages>*,
// ListContainerBase<FEInGameMessengerManager::eInGameMessages, NewAdapter<ListEntry<FEInGameMessengerManager::eInGameMessages>>>*, void
// (ListContainerBase<FEInGameMessengerManager::eInGameMessages,
// NewAdapter<ListEntry<FEInGameMessengerManager::eInGameMessages>>>::*)(ListEntry<FEInGameMessengerManager::eInGameMessages>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x80100000 | size: 0x44
//  */
// void nlListRemoveStart<ListEntry<FEInGameMessengerManager::eInGameMessages>>(ListEntry<FEInGameMessengerManager::eInGameMessages>**,
// ListEntry<FEInGameMessengerManager::eInGameMessages>**)
// {
// }

// /**
//  * Offset/Address/Size: 0xAC | 0x80100044 | size: 0x2C
//  */
// void nlListAddEnd<ListEntry<FEInGameMessengerManager::eInGameMessages>>(ListEntry<FEInGameMessengerManager::eInGameMessages>**,
// ListEntry<FEInGameMessengerManager::eInGameMessages>**, ListEntry<FEInGameMessengerManager::eInGameMessages>*)
// {
// }
