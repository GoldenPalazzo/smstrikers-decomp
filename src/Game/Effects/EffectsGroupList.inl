template <>
WEAKFUNC void EffectsGroupUserSpecContainer::DeleteEntry(DLListEntry<UserEffectSpec*>* entry)
{
    m_Allocator.DeleteEntry(entry);
}
