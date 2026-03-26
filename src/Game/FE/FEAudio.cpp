#include "Game/FE/FEAudio.h"

#include "Game/Audio/AudioLoader.h"
#include "Game/BasicStadium.h"
#include "Game/Game.h"
#include "Game/Sys/eventman.h"
#include "NL/nlList.h"
#include "NL/nlQSort.h"
#include "NL/nlString.h"

static bool mIsEnabled = true;
static void* gpLastSoundFromPlayer;

struct FrontEndAnimAudioData : EventData
{
    /* 0x04 */ unsigned long audioIdentifier;
}; // total size: 0x8

extern "C" void qsort(void*, unsigned long, unsigned long, int (*)(const void*, const void*));

template <typename T>
class BasicSlotPoolHigh : public SlotPoolBase
{
public:
    static void* allocFN(unsigned long size) { return nlMalloc(size, 8, true); }
    static void freeFN(void* ptr) { nlFree(ptr); }
};

template <>
class ListContainerBase<AnimAudioEventLookup, BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup> > >
    : public BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup> >
{
public:
    ListEntry<AnimAudioEventLookup>* m_Head;
    ListEntry<AnimAudioEventLookup>* m_Tail;

    ListContainerBase()
    {
        m_AllocFn = BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup> >::allocFN;
        m_FreeFn = BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup> >::freeFN;
        m_Head = NULL;
        m_Tail = NULL;
        m_Initial = 0x10;
        SlotPoolBase::BaseAddNewBlock(this, sizeof(ListEntry<AnimAudioEventLookup>));
        m_Delta = 0x10;
    }

    void DeleteEntry(ListEntry<AnimAudioEventLookup>* entry)
    {
        entry->next = (ListEntry<AnimAudioEventLookup>*)m_FreeList;
        m_FreeList = (SlotPoolEntry*)entry;
    }
};

typedef ListContainerBase<AnimAudioEventLookup, BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup> > > FELookupPool;

struct LookupBlock
{
    unsigned long w[14];
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
        Audio::gWorldSFX.Play(Audio::WORLDSFX_PLACEHOLDER, 1.0f, 0.0f, false, 1.0f);
        break;
    case 0x49:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_PLACEHOLDER, 1.0f, 0.0f, false, 1.0f);
        break;
    case 0x4A:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_PLACEHOLDER, 1.0f, 0.0f, false, 1.0f);
        break;
    case 0x4B:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_PLACEHOLDER, 1.0f, 0.0f, false, 1.0f);
        break;
    case 0x4C:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_BUTTON_GEN_SELECT_ACCEPT, 1.0f, 0.0f, false, 1.0f);
        break;
    case 0x4D:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_BUTTON_GEN_SELECT_BACK, 1.0f, 0.0f, false, 1.0f);
        break;
    case 0x4E:
        AudioLoader::PlayPauseMenuMusic();
        break;
    case 0x4F:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_SCREEN_GEN_BEGIN, 1.0f, 0.0f, false, 1.0f);
        break;
    case 0x50:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_SCREEN_GEN_END, 1.0f, 0.0f, false, 1.0f);
        break;
    case 0x51:
        Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_DENY, 1.0f, 0.0f, false, 1.0f);
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
 * TODO: 66.2% match - r30/r31 register swap for pGIM/randomElementIndex
 * and static variable label numbering differences
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

        GameInfoManager* pGIM = nlSingleton<GameInfoManager>::s_pInstance;
        pGIM->GetSidekick(0);
        pGIM->GetSidekick(0);

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

        Audio::eCharSFX newSound = charInGameDialogueTypes[randomElementIndex];
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
                AnimAudioEventLookup* event;
                if (result)
                    event = result;
                else
                    event = NULL;

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
                AnimAudioEventLookup* event;
                if (result)
                    event = result;
                else
                    event = NULL;

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
 * TODO: 98.38% match - stack frame 0x5F0 vs 0x600 (16-byte alignment boundary),
 * pEntry r29 vs r31, r22-relative vs sp-relative init stores
 */
void FEAudio::BuildAnimAudioEventLookup()
{
    unsigned long fileSize;
    ListEntry<AnimAudioEventLookup>* pEntry;
    char* pFileData = (char*)nlLoadEntireFile("audio/FEAnimAudio.txt", &fileSize, 0x20, (eAllocType)1);

    static AnimAudioEventLookup blankEntry;
    FELookupPool LookupList;

    AnimAudioEventLookup entry;
    AnimAudioEventLookup temp;

    SimpleParser parser;
    parser.StartParsing(pFileData, fileSize, false);

    char* pToken;

    do
    {
        pToken = parser.NextToken(false);
        while (pToken != NULL)
        {
            char* equalPos = strchr(pToken, '=');
            if (equalPos != NULL)
            {
                pEntry = NULL;
                equalPos[-1] = '\0';
                char* pSFXTypeStr = equalPos + 2;
                *(LookupBlock*)&entry = *(LookupBlock*)&blankEntry;
                *(LookupBlock*)&temp = *(LookupBlock*)&entry;

                if (LookupList.m_FreeList == NULL)
                {
                    SlotPoolBase::BaseAddNewBlock(&LookupList, sizeof(ListEntry<AnimAudioEventLookup>));
                }
                if (LookupList.m_FreeList != NULL)
                {
                    pEntry = (ListEntry<AnimAudioEventLookup>*)LookupList.m_FreeList;
                    LookupList.m_FreeList = LookupList.m_FreeList->m_next;
                }

                if (pEntry != NULL)
                {
                    pEntry->next = NULL;
                    *(LookupBlock*)&pEntry->data = *(LookupBlock*)&temp;
                }

                nlListAddEnd(&LookupList.m_Head, &LookupList.m_Tail, pEntry);
                pEntry->data.hash = nlStringHash(pToken);
                nlStrNCpy(pEntry->data.szSFXType, pSFXTypeStr, 0x32);
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
        gp_AnimAudioEventTable[i] = node->data;
        i++;
        node = node->next;
    }

    nlQSort<AnimAudioEventLookup>(gp_AnimAudioEventTable, (int)gNumAnimAudioEvents, &nlDefaultQSortComparer<AnimAudioEventLookup>);

    nlFree(pFileData);

    nlWalkList(LookupList.m_Head, (FELookupPool*)&LookupList, &FELookupPool::DeleteEntry);
    LookupList.m_Head = NULL;
    LookupList.m_Tail = NULL;
    SlotPoolBase::BaseFreeBlocks(&LookupList, sizeof(ListEntry<AnimAudioEventLookup>));

    nlWalkList(LookupList.m_Head, (FELookupPool*)&LookupList, &FELookupPool::DeleteEntry);
    LookupList.m_Head = NULL;
    LookupList.m_Tail = NULL;
    SlotPoolBase::BaseFreeBlocks(&LookupList, sizeof(ListEntry<AnimAudioEventLookup>));
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

/**
 * Offset/Address/Size: 0x28 | 0x8009FA28 | size: 0x2C
 */
template <>
int nlDefaultQSortComparer<AnimAudioEventLookup>(const AnimAudioEventLookup* a, const AnimAudioEventLookup* b)
{
    if (a->hash > b->hash)
        return 1;
    if (a->hash == b->hash)
        return 0;
    return -1;
}

/**
 * Offset/Address/Size: 0x54 | 0x8009FA54 | size: 0x8C
 */
template <>
AnimAudioEventLookup* nlBSearch(const unsigned long& key, AnimAudioEventLookup* base, int count)
{
    unsigned long keyVal = (unsigned long)key;
    int high = count - 1;
    int low = -1;

    while ((high - low) > 1)
    {
        int mid = (high + low) / 2;
        if (base[mid].hash > keyVal)
            high = mid;
        else
            low = mid;
    }

    unsigned long cmpKey = *(const volatile unsigned long*)&key;

    unsigned long highHash = base[high].hash;
    if (highHash == cmpKey)
        return &base[high];

    if (low == -1)
        return 0;

    unsigned long lowHash = base[low].hash;
    if (lowHash == cmpKey)
        return &base[low];

    return 0;
}

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

/**
 * Offset/Address/Size: 0x0 | 0x8009FB74 | size: 0x84
 * TODO: 87.7% match - MWCC emits extsb r3,r0 before each bl nlToUpper<c>__Fc
 * (sign-extending char arg), but target loads directly into r3 with lbz.
 * Also minor scheduling diff with -O4,p (lbz for 2nd str moved before mr r31).
 * Compiler char sign-extension ABI quirk - not fixable with code changes.
 */
// TEMP removed in scratch-root for decomp iteration
template <>
int nlStrICmp(const char* a, const char* b);

// /**
//  * Offset/Address/Size: 0x84 | 0x8009FBF8 | size: 0x40
//  */
// void nlStrCmp<char>(const char*, const char*)
// {
// }

// /**
//  * Offset/Address/Size: 0xC4 | 0x8009FC38 | size: 0x20
//  */
// void nlToUpper<char>(char)
// {
// }
