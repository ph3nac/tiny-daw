#pragma once

#include "juce_core/juce_core.h"
class PluginTreeBase
{
public:
    virtual ~PluginTreeBase() = default;
    virtual juce::String getUniqueName() const = 0;

    void addSubItem (PluginTreeBase* itm) { subitems.add (itm); }
    int getNumSubItems() { return subitems.size(); }
    PluginTreeBase* getSubItem (int idx) { return subitems[idx]; }

private:
    juce::OwnedArray<PluginTreeBase> subitems;
};
