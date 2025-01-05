#pragma once
#include "plugin/PluginTreeBase.h"

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

class PluginTreeItem : public PluginTreeBase
{
public:
    PluginTreeItem (const juce::PluginDescription&);
    PluginTreeItem (const juce::String& uniqueId, const juce::String& name, const juce::String& xmlType, bool isSynth, bool isPlugin);

    te::Plugin::Ptr create (te::Edit&);

    juce::String getUniqueName() const override
    {
        if (desc.fileOrIdentifier.startsWith (te::RackType::getRackPresetPrefix()))
            return desc.fileOrIdentifier;

        return desc.createIdentifierString();
    }

    juce::PluginDescription desc;
    juce::String xmlType;
    bool isPlugin = true;

    JUCE_LEAK_DETECTOR (PluginTreeItem)
};

inline PluginTreeItem::PluginTreeItem (const juce::PluginDescription& d)
    : desc (d), xmlType (te::ExternalPlugin::xmlTypeName), isPlugin (true)
{
    jassert (xmlType.isNotEmpty());
}

inline PluginTreeItem::PluginTreeItem (const juce::String& uniqueId, const juce::String& name, const juce::String& xmlType_, bool isSynth, bool isPlugin_)
    : xmlType (xmlType_), isPlugin (isPlugin_)
{
    jassert (xmlType.isNotEmpty());
    desc.name = name;
    desc.fileOrIdentifier = uniqueId;
    desc.pluginFormatName = (uniqueId.endsWith ("_trkbuiltin") || xmlType == te::RackInstance::xmlTypeName)
                                ? juce::String (te::PluginManager::builtInPluginFormatName)
                                : juce::String();
    desc.category = xmlType;
    desc.isInstrument = isSynth;
}

inline te::Plugin::Ptr PluginTreeItem::create (te::Edit& ed)
{
    return ed.getPluginCache().createNewPlugin (xmlType, desc);
}
