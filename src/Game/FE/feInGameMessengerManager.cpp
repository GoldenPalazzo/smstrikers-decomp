#include "Game/FE/feInGameMessengerManager.h"
#include "Game/FE/feIMessenger.h"
#include "Game/Game.h"
#include "Game/Ball.h"
#include "Game/GameInfo.h"

/**
 * Offset/Address/Size: 0x0 | 0x800FF91C | size: 0x140
 */
void FEInGameMessengerManager::Update(float fDeltaT)
{
    if (m_waitingToDisplay)
    {
        m_waitedToDisplay += fDeltaT;
    }

    float normTime = g_pGame->GetNormalizedGameTime();
    eTimeStates newState = m_curTimeState;

    while (newState + 1 != TS_NUMTIMESTATES)
    {
        if (normTime >= TIMESTATE_TIMES[newState + 1])
        {
            newState = (eTimeStates)(newState + 1);
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
 * TODO: 97.52% match - remaining diffs are this/timeState register swap
 * (r30/r31) and ListEntry constructor temporary register ordering.
 */
#pragma opt_common_subs off
void FEInGameMessengerManager::EnterNewTimeState(FEInGameMessengerManager::eTimeStates timeState)
{
    switch (timeState)
    {
    case TS_GAME_BEGINNING:
        break;

    case TS_GAME_EARLYMID:
    {
        ListEntry<eInGameMessages>** pTail = &m_messageQueue.m_Tail;
        ListEntry<eInGameMessages>** pHead = &m_messageQueue.m_Head;

        for (int i = 0; i < m_numWatchGames; i++)
        {
            eInGameMessages msg = (eInGameMessages)i;
            int hasMessage;

            if (m_messageList[(int)msg].m_data != NULL)
            {
                hasMessage = m_messageList[(int)msg].m_data->mSize - 1;
            }
            else
            {
                hasMessage = 0;
            }

            if (hasMessage != 0)
            {
                if (m_messageQueue.m_Head == NULL)
                {
                    m_waitingToDisplay = true;
                    m_waitedToDisplay = 0.0f;
                }

                volatile eInGameMessages stackMsg[2];
                stackMsg[1] = msg;

                ListEntry<eInGameMessages>* entry = new (nlMalloc(sizeof(ListEntry<eInGameMessages>), 8, false)) ListEntry<eInGameMessages>(msg);
                nlListAddEnd<ListEntry<eInGameMessages> >(pHead, pTail, entry);
            }
        }
        break;
    }

    case TS_GAME_MIDLATE:
    {
        ListEntry<eInGameMessages>** pTail = &m_messageQueue.m_Tail;
        ListEntry<eInGameMessages>** pHead = &m_messageQueue.m_Head;

        for (int i = 0; i < m_numWatchGames; i++)
        {
            eInGameMessages msg = (eInGameMessages)(i + 2);
            int hasMessage;

            if (m_messageList[(int)msg].m_data != NULL)
            {
                hasMessage = m_messageList[(int)msg].m_data->mSize - 1;
            }
            else
            {
                hasMessage = 0;
            }

            if (hasMessage != 0)
            {
                if (m_messageQueue.m_Head == NULL)
                {
                    m_waitingToDisplay = true;
                    m_waitedToDisplay = 0.0f;
                }

                volatile eInGameMessages stackMsg[2];
                stackMsg[0] = msg;

                ListEntry<eInGameMessages>* entry = new (nlMalloc(sizeof(ListEntry<eInGameMessages>), 8, false)) ListEntry<eInGameMessages>(stackMsg[0]);
                nlListAddEnd<ListEntry<eInGameMessages> >(pHead, pTail, entry);
            }
        }
        break;
    }

    case TS_GAME_LATE:
    {
        int sequence[4] = { 0, 1, 2, 3 };

        for (int i = 0; i < 4; i++)
        {
            int swapInd = i + nlRandom(4 - i, &nlDefaultSeed);
            int temp = sequence[i];
            sequence[i] = sequence[swapInd];
            sequence[swapInd] = temp;
        }

        ListEntry<eInGameMessages>** pTail = &m_messageQueue.m_Tail;
        ListEntry<eInGameMessages>** pHead = &m_messageQueue.m_Head;

        int numDisplayed = 0;
        for (int i = 0; i < 4; i++)
        {
            eInGameMessages msg;
            int hasMessage;

            if (numDisplayed >= 2)
            {
                break;
            }

            msg = (eInGameMessages)(i + 4);

            if (m_messageList[(int)msg].m_data != NULL)
            {
                hasMessage = m_messageList[(int)msg].m_data->mSize - 1;
            }
            else
            {
                hasMessage = 0;
            }

            if (hasMessage != 0)
            {
                if (m_messageList[(int)msg].m_data != NULL)
                {
                    hasMessage = m_messageList[(int)msg].m_data->mSize - 1;
                }
                else
                {
                    hasMessage = 0;
                }

                if (hasMessage != 0)
                {
                    if (m_messageQueue.m_Head == NULL)
                    {
                        m_waitingToDisplay = true;
                        m_waitedToDisplay = 0.0f;
                    }

                    volatile eInGameMessages stackMsg[2];
                    stackMsg[1] = msg;

                    ListEntry<eInGameMessages>* entry = new (nlMalloc(sizeof(ListEntry<eInGameMessages>), 8, false)) ListEntry<eInGameMessages>(msg);
                    nlListAddEnd<ListEntry<eInGameMessages> >(pHead, pTail, entry);
                }

                if (m_messageList[(int)msg].m_data != NULL)
                {
                    hasMessage = m_messageList[(int)msg].m_data->mSize - 1;
                }
                else
                {
                    hasMessage = 0;
                }

                if (hasMessage != 0)
                {
                    if (m_messageQueue.m_Head == NULL)
                    {
                        m_waitingToDisplay = true;
                        m_waitedToDisplay = 0.0f;
                    }

                    volatile eInGameMessages stackMsg[2];
                    stackMsg[1] = msg;

                    ListEntry<eInGameMessages>* entry = new (nlMalloc(sizeof(ListEntry<eInGameMessages>), 8, false)) ListEntry<eInGameMessages>(msg);
                    nlListAddEnd<ListEntry<eInGameMessages> >(pHead, pTail, entry);
                }

                numDisplayed++;
            }
        }

        if (nlSingleton<GameInfoManager>::s_pInstance->mCurrentMode == GameInfoManager::GM_TOURNAMENT)
        {
            eInGameMessages msg = MSG_CUSTOMTOURNNEXTMATCHUP;
            int hasMessage;

            if (m_messageList[(int)msg].m_data != NULL)
            {
                hasMessage = m_messageList[(int)msg].m_data->mSize - 1;
            }
            else
            {
                hasMessage = 0;
            }

            if (hasMessage != 0)
            {
                if (m_messageQueue.m_Head == NULL)
                {
                    m_waitingToDisplay = true;
                    m_waitedToDisplay = 0.0f;
                }

                volatile eInGameMessages stackMsg[2];
                stackMsg[0] = msg;

                ListEntry<eInGameMessages>* entry = new (nlMalloc(sizeof(ListEntry<eInGameMessages>), 8, false)) ListEntry<eInGameMessages>(stackMsg[0]);
                nlListAddEnd<ListEntry<eInGameMessages> >(pHead, pTail, entry);
            }
        }
        break;
    }

    default:
        break;
    }

    m_curTimeState = timeState;
}
#pragma opt_common_subs on

/**
 * Offset/Address/Size: 0x554 | 0x800FFE70 | size: 0x104
 */
FEInGameMessengerManager::~FEInGameMessengerManager()
{
    Function<FnVoidVoid> cb;
    cb.mTag = EMPTY;
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
