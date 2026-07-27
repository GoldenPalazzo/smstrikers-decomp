#include "Game/FE/FEAudio.h"

#include "Game/Audio/AudioLoader.h"
#include "Game/BasicStadium.h"
#include "Game/Game.h"
#include "Game/Sys/eventman.h"
#include "NL/nlList.h"
#include "NL/nlAlgorithm.h"
#include "NL/nlString.h"

AnimAudioEventLookup* gp_AnimAudioEventTable;
unsigned long gNumAnimAudioEvents;

static bool mIsEnabled = true;
static void* gpLastSoundFromPlayer;

struct FrontEndAnimAudioData : EventData
{
    /* 0x04 */ unsigned long audioIdentifier;
}; // total size: 0x8

extern "C" void qsort(void*, unsigned long, unsigned long, int (*)(const void*, const void*));

typedef nlListSlotPoolHigh<AnimAudioEventLookup> FELookupPool;
typedef ListContainerBase<AnimAudioEventLookup, BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup> > > FELookupBase;

struct LookupBlock
{
    unsigned long w[14];
};

struct LookupEntryBlock
{
    unsigned long next;
    LookupBlock data;
};

/**
 * Offset/Address/Size: 0x0 | 0x8009EDAC | size: 0x8
 */
void FEAudio::EnableSounds(bool enable)
{
    mIsEnabled = enable;
}

/**
 * Offset/Address/Size: 0x8 | 0x8009EDB4 | size: 0x27C
 */
// #pragma inline_depth(0)
void FEAudioEventHandler(Event* pEvent, void*)
{
    if (!AudioLoader::IsInited())
    {
        return;
    }

    switch (pEvent->m_uEventID)
    {
    case 0x48:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_PLACEHOLDER, 100.0f, -1.0f, false, 100.0f);
        break;
    case 0x49:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_PLACEHOLDER, 100.0f, -1.0f, false, 100.0f);
        break;
    case 0x4A:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_PLACEHOLDER, 100.0f, -1.0f, false, 100.0f);
        break;
    case 0x4B:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_PLACEHOLDER, 100.0f, -1.0f, false, 100.0f);
        break;
    case 0x4C:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_BUTTON_GEN_SELECT_ACCEPT, 100.0f, -1.0f, false, 100.0f);
        break;
    case 0x4D:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_BUTTON_GEN_SELECT_BACK, 100.0f, -1.0f, false, 100.0f);
        break;
    case 0x4E:
        AudioLoader::PlayPauseMenuMusic();
        break;
    case 0x4F:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_SCREEN_GEN_BEGIN, 100.0f, -1.0f, false, 100.0f);
        break;
    case 0x50:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_SCREEN_GEN_END, 100.0f, -1.0f, false, 100.0f);
        break;
    case 0x51:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_DENY, 100.0f, -1.0f, false, 100.0f);
        break;
    case 0x52:
    {
        FrontEndAnimAudioData* data;
        if ((s32)pEvent->m_data.GetID() == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = NULL;
        }
        else if ((s32)pEvent->m_data.GetID() != 0x16D)
        {
            nlPrintf("Error: GetData() failed! Data types do not match!\n");
            data = NULL;
        }
        else
        {
            data = (FrontEndAnimAudioData*)&pEvent->m_data;
        }

        unsigned long stackHash = data->audioIdentifier;
        AnimAudioEventLookup* result = nlBSearch<AnimAudioEventLookup, unsigned long>(stackHash, gp_AnimAudioEventTable, gNumAnimAudioEvents);
        AnimAudioEventLookup* event;
        if (result)
        {
            event = result;
        }
        else
        {
            event = NULL;
        }

        if (AudioLoader::IsInited() && mIsEnabled)
        {
            Audio::PlayWorldSFXbyStr(event->szSFXType, 1.0f, 0.0f, true, true, NULL, NULL, NULL);
        }

        break;
    }
    case 0x53:
    case 0x54:
    default:
        break;
    }
}
// #pragma inline_depth()

/**
 * Offset/Address/Size: 0x284 | 0x8009F030 | size: 0xC
 */
void FEAudio::ResetRandomVoiceToggleSFX()
{
    gpLastSoundFromPlayer = nullptr;
}

/**
 * Offset/Address/Size: 0x290 | 0x8009F03C | size: 0x360
 */
void FEAudio::PlayRandomVoiceToggleSFX()
{
    if (g_pGame != NULL)
    {
        Audio::eCharSFX charInGameDialogueTypes[] = {
            Audio::CHARSFX_EFFORTS_ATTACK_01, Audio::CHARSFX_EFFORTS_ATTACK_02, Audio::CHARSFX_EFFORTS_ATTACK_03, Audio::CHARSFX_EFFORTS_HIT_01, Audio::CHARSFX_EFFORTS_HIT_02, Audio::CHARSFX_EFFORTS_HIT_03, Audio::CHARSFX_EFFORTS_GET_HIT_01, Audio::CHARSFX_EFFORTS_GET_HIT_02, Audio::CHARSFX_EFFORTS_GET_HIT_03, Audio::CHARSFX_EFFORTS_PAIN_01, Audio::CHARSFX_EFFORTS_PAIN_02, Audio::CHARSFX_EFFORTS_PAIN_03, Audio::CHARSFX_EFFORTS_PAIN_04, Audio::CHARSFX_EFFORTS_PAIN_05, Audio::CHARSFX_EFFORTS_ELECTROCUTE_01, Audio::CHARSFX_EFFORTS_ELECTROCUTE_02, Audio::CHARSFX_EFFORTS_ELECTROCUTE_03, Audio::CHARSFX_EFFORTS_EXERT_01, Audio::CHARSFX_EFFORTS_EXERT_02, Audio::CHARSFX_EFFORTS_EXERT_03, Audio::CHARSFX_EFFORTS_KICK_01, Audio::CHARSFX_EFFORTS_KICK_02, Audio::CHARSFX_EFFORTS_KICK_03, Audio::CHARSFX_BOWSER_ENTER, Audio::CHARSFX_BOWSER_ACTIVATE, Audio::CHARSFX_BOWSER_HOWL_01
        };

        static Audio::eCharSFX lastSoundPlayedType;
        static signed char init;
        if (!init)
        {
            lastSoundPlayedType = Audio::CHARSFX_NONE;
            init = 1;
        }

        GameInfoManager& gameInfo = *GameInfoManager::GetInstance();
        gameInfo.GetSidekick(0);
        gameInfo.GetSidekick(0);

        Audio::eCharSFX newSound;
        unsigned int randomElementIndex = nlRandom(26, &nlDefaultSeed);

        if (lastSoundPlayedType != Audio::CHARSFX_NONE)
        {
            if (lastSoundPlayedType == charInGameDialogueTypes[randomElementIndex])
            {
                randomElementIndex = (randomElementIndex + 1) % 26;
            }

            if (lastSoundPlayedType >= Audio::CHARSFX_BOWSER_ENTER && lastSoundPlayedType <= Audio::CHARSFX_BOWSER_HOWL_03)
            {
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->Stop(lastSoundPlayedType, cGameSFX::SFX_STOP_FIRST);
            }
            else
            {
                if (gpLastSoundFromPlayer != NULL)
                {
                    ((cCharacter*)gpLastSoundFromPlayer)->StopSFX(lastSoundPlayedType);
                }
            }

            lastSoundPlayedType = Audio::CHARSFX_NONE;
            gpLastSoundFromPlayer = NULL;
        }

        newSound = charInGameDialogueTypes[randomElementIndex];
        lastSoundPlayedType = newSound;

        if (newSound < Audio::CHARSFX_BOWSER_ENTER && newSound != Audio::CHARSFX_EFFORTS_KICK_03)
        {
            if ((newSound >= Audio::CHARSFX_EFFORTS_DAZED && newSound <= Audio::CHARSFX_EFFORTS_STS_FLOAT_01) || (newSound >= Audio::CHARSFX_EFFORTS_PAIN_04 && newSound <= Audio::CHARSFX_EFFORTS_ELECTROCUTE_01))
            {
                gpLastSoundFromPlayer = g_pTeams[0]->GetFielder(1);
            }
            else
            {
                gpLastSoundFromPlayer = g_pTeams[0]->GetGoalie();
            }

            Audio::SoundAttributes sndAtr;
            sndAtr.Init();
            sndAtr.SetSoundType(charInGameDialogueTypes[randomElementIndex], false);
            ((cCharacter*)gpLastSoundFromPlayer)->m_pCharacterSFX->Play(sndAtr);
        }
        else
        {
            if (newSound >= Audio::CHARSFX_BOWSER_ENTER && newSound <= Audio::CHARSFX_BOWSER_HOWL_03)
            {
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->PlaySFX(newSound, NONE, 0.0f, false);
            }
            else
            {
                gpLastSoundFromPlayer = g_pTeams[0]->GetGoalie();
                Audio::SoundAttributes sndAtr;
                sndAtr.Init();
                sndAtr.SetSoundType(charInGameDialogueTypes[randomElementIndex], false);
                ((cCharacter*)gpLastSoundFromPlayer)->m_pCharacterSFX->Play(sndAtr);
            }
        }
    }
    else
    {
        const char* szEvent = "fe_toggle_voice";

        if (AudioLoader::IsInited())
        {
            unsigned long hash = nlStringLowerHash(szEvent);
            if (AudioLoader::IsInited())
            {
                unsigned long stackHash = hash;
                AnimAudioEventLookup* result = nlBSearch<AnimAudioEventLookup, unsigned long>(stackHash, gp_AnimAudioEventTable, gNumAnimAudioEvents);
                AnimAudioEventLookup* event = result ? result : NULL;

                if (nlStrCmp<char>(event->szSFXType, "") != 0)
                {
                    Audio::StopWorldSFXbyStr(event->szSFXType);
                }
            }
        }

        unsigned long hash = nlStringLowerHash(szEvent);
        if (AudioLoader::IsInited())
        {
            if (mIsEnabled)
            {
                unsigned long stackHash = hash;
                AnimAudioEventLookup* result = nlBSearch<AnimAudioEventLookup, unsigned long>(stackHash, gp_AnimAudioEventTable, gNumAnimAudioEvents);
                AnimAudioEventLookup* event = result ? result : NULL;

                if (nlStrICmp<char>(event->szSFXType, "") != 0)
                {
                    Audio::PlayWorldSFXbyStr(event->szSFXType, 1.0f, 0.0f, false, true, NULL, NULL, NULL);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x5F0 | 0x8009F39C | size: 0xB8
 */
long FEAudio::PlayAnimAudioEvent(unsigned long uHash, bool)
{
    if (!AudioLoader::IsInited())
    {
        return -1;
    }

    if (!mIsEnabled)
    {
        return -1;
    }

    unsigned long stackHash = uHash;
    AnimAudioEventLookup* result = nlBSearch<AnimAudioEventLookup, unsigned long>(stackHash, gp_AnimAudioEventTable, gNumAnimAudioEvents);
    AnimAudioEventLookup* event;
    if (result)
    {
        event = result;
    }
    else
    {
        event = NULL;
    }

    if (nlStrICmp<char>(event->szSFXType, "") == 0)
    {
        return -1;
    }

    return Audio::PlayWorldSFXbyStr(event->szSFXType, 1.0f, 0.0f, false, true, NULL, NULL, NULL);
}

/**
 * Offset/Address/Size: 0x6A8 | 0x8009F454 | size: 0x90
 */
void FEAudio::StopAnimAudioEvent(const char* eventName)
{
    if (!AudioLoader::IsInited())
    {
        return;
    }

    unsigned long hash = nlStringLowerHash(eventName);

    if (!AudioLoader::IsInited())
    {
        return;
    }

    unsigned long stackHash = hash;
    AnimAudioEventLookup* result = nlBSearch<AnimAudioEventLookup, unsigned long>(stackHash, gp_AnimAudioEventTable, gNumAnimAudioEvents);
    AnimAudioEventLookup* event;
    if (result)
    {
        event = result;
    }
    else
    {
        event = NULL;
    }

    if (nlStrCmp<char>(event->szSFXType, "") != 0)
    {
        Audio::StopWorldSFXbyStr(event->szSFXType);
    }
}

/**
 * Offset/Address/Size: 0x738 | 0x8009F4E4 | size: 0xBC
 */
long FEAudio::PlayAnimAudioEvent(const char* eventName, bool)
{
    unsigned long hash = nlStringLowerHash(eventName);

    if (!AudioLoader::IsInited())
    {
        return -1;
    }

    if (!mIsEnabled)
    {
        return -1;
    }

    unsigned long stackHash = hash;
    AnimAudioEventLookup* result = nlBSearch<AnimAudioEventLookup, unsigned long>(stackHash, gp_AnimAudioEventTable, gNumAnimAudioEvents);
    AnimAudioEventLookup* event;
    if (result)
    {
        event = result;
    }
    else
    {
        event = NULL;
    }

    if (nlStrICmp<char>(event->szSFXType, "") == 0)
    {
        return -1;
    }

    return Audio::PlayWorldSFXbyStr(event->szSFXType, 1.0f, 0.0f, false, true, NULL, NULL, NULL);
}

/**
 * Offset/Address/Size: 0x7F4 | 0x8009F5A0 | size: 0x408
 * TODO: 99.13% match - pEntry/list head-tail and output pointer/counter use different registers
 */
void FEAudio::BuildAnimAudioEventLookup()
{
    unsigned long fileSize;
    ListEntry<AnimAudioEventLookup>* pEntry;
    char* pFileData = (char*)nlLoadEntireFile("audio/FEAnimAudio.txt", &fileSize, 0x20, (eAllocType)1);

    static AnimAudioEventLookup blankEntry;
    FELookupPool LookupList(0x10, 0x10);

    AnimAudioEventLookup temp;
    LookupEntryBlock entry;

    SimpleParser parser;
    parser.StartParsing(pFileData, fileSize, false);

    char* pToken;
    char* pSFXTypeStr;

    do
    {
        pToken = parser.NextToken(false);
        while (pToken != NULL)
        {
            char* equalPos = strchr(pToken, '=');
            if (equalPos != NULL)
            {
                equalPos[-1] = (char)(pEntry = NULL);
                pSFXTypeStr = equalPos + 2;
                *(LookupBlock*)&temp = *(LookupBlock*)&blankEntry;
                entry.data = *(LookupBlock*)&temp;

                if (LookupList.m_Allocator.m_FreeList == NULL)
                {
                    SlotPoolBase::BaseAddNewBlock(&LookupList.m_Allocator, sizeof(ListEntry<AnimAudioEventLookup>));
                }
                if (LookupList.m_Allocator.m_FreeList != NULL)
                {
                    pEntry = (ListEntry<AnimAudioEventLookup>*)LookupList.m_Allocator.m_FreeList;
                    LookupList.m_Allocator.m_FreeList = LookupList.m_Allocator.m_FreeList->next;
                }

                if (pEntry != NULL)
                {
                    pEntry->next = NULL;
                    *(LookupBlock*)&pEntry->entry = entry.data;
                }

                nlListAddEnd(&LookupList.m_Head, &LookupList.m_Tail, pEntry);
                pEntry->entry.eventNameHash = nlStringHash(pToken);
                nlStrNCpy(pEntry->entry.szSFXType, pSFXTypeStr, 0x32);
                gNumAnimAudioEvents++;
            }

            pToken = parser.NextToken(false);
        }
    } while (parser.AdvanceLine());

    gp_AnimAudioEventTable = (AnimAudioEventLookup*)nlMalloc(gNumAnimAudioEvents * sizeof(AnimAudioEventLookup), 8, false);

    int i = 0;
    ListEntry<AnimAudioEventLookup>* node = LookupList.m_Head;
    while (node != NULL)
    {
        AnimAudioEventLookup& out = gp_AnimAudioEventTable[i];
        out = node->entry;
        i++;
        node = node->next;
    }

    nlQSort<AnimAudioEventLookup>(gp_AnimAudioEventTable, (int)gNumAnimAudioEvents, &nlDefaultQSortComparer<AnimAudioEventLookup>);

    nlFree(pFileData);

    nlWalkList(LookupList.m_Head, (FELookupBase*)&LookupList, &FELookupBase::DeleteEntry);
    LookupList.m_Head = NULL;
    LookupList.m_Tail = NULL;
    SlotPoolBase::BaseFreeBlocks(&LookupList.m_Allocator, sizeof(ListEntry<AnimAudioEventLookup>));
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8009F9A8 | size: 0x10
//  */
// void ListContainerBase<AnimAudioEventLookup, BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>>::DeleteEntry(
//     ListEntry<AnimAudioEventLookup>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8009F9B8 | size: 0x20
//  */
// void BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>::freeFN(void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x20 | 0x8009F9D8 | size: 0x28
//  */
// void BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>::allocFN(unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8009FA00 | size: 0x28
//  */
// void nlQSort<AnimAudioEventLookup>(AnimAudioEventLookup*, int, int (*)(const AnimAudioEventLookup*, const AnimAudioEventLookup*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8009FAE0 | size: 0x68
//  */
// void nlWalkList<ListEntry<AnimAudioEventLookup>,
//                 ListContainerBase<AnimAudioEventLookup, BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>>>(
//     ListEntry<AnimAudioEventLookup>*, ListContainerBase<AnimAudioEventLookup, BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>>*,
//     void (ListContainerBase<AnimAudioEventLookup,
//     BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>>::*)(ListEntry<AnimAudioEventLookup>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x8009FB48 | size: 0x2C
//  */
// void nlListAddEnd<ListEntry<AnimAudioEventLookup>>(ListEntry<AnimAudioEventLookup>**, ListEntry<AnimAudioEventLookup>**,
//                                                    ListEntry<AnimAudioEventLookup>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x84 | 0x8009FBF8 | size: 0x40
//  */
// void nlStrCmp<char>(const char*, const char*)
// {
// }
