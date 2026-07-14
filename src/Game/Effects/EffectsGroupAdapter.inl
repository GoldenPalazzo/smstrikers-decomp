typedef NewAdapter<EffectsGroupTreeEntry> EffectsGroupTreeAdapter;
typedef NewAdapter<EffectsTerrainTreeEntry> EffectsTerrainTreeAdapter;

template <>
WEAKFUNC void EffectsGroupTreeAdapter::Delete(EffectsGroupTreeEntry* ptr)
{
    delete ptr;
}

template <>
WEAKFUNC void EffectsTerrainTreeAdapter::Delete(EffectsTerrainTreeEntry* ptr)
{
    delete ptr;
}
