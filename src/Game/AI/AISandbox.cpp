#include "Game/AI/AISandbox.h"

/**
 * Offset/Address/Size: 0x0 | 0x80006F6C | size: 0x10
 * DeleteEntry__105AVLTreeBase<Ul,12FuzzyVariant,...>
 * Template instantiation from nlAVLTreeBase.h via CommonScript.h include
 */

// Force weak symbol emission for AVLTreeBase<Ul, FuzzyVariant, ...>::DeleteEntry
typedef AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> > _FuzzyAVLTree;

/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler
 * lays out the related fields to match the original binary.
 */
void AISandbox_stub()
{
    void (_FuzzyAVLTree::* volatile forceDeleteEntry)(AVLTreeEntry<unsigned long, FuzzyVariant>*) = &_FuzzyAVLTree::DeleteEntry;
    (void)forceDeleteEntry;
}
