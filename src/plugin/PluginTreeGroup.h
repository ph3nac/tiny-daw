#pragma once
#include "plugin/PluginTreeBase.h"
#include "plugin/PluginTreeItem.h"
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;

class PluginTreeGroup : public PluginTreeBase
{
public:
    PluginTreeGroup (te::Edit&, juce::KnownPluginList::PluginTree&, te::Plugin::Type);
    PluginTreeGroup (const juce::String&);

    juce::String getUniqueName() const override { return name; }

    juce::String name;

private:
    void populateFrom (juce::KnownPluginList::PluginTree&);
    void createBuiltInItems (int& num, te::Plugin::Type);

    JUCE_LEAK_DETECTOR (PluginTreeGroup)
};

inline PluginTreeGroup::PluginTreeGroup (te::Edit& edit, juce::KnownPluginList::PluginTree& tree, te::Plugin::Type types)
    : name ("Plugins")
{
    {
        int num = 1;

        auto builtinFolder = new PluginTreeGroup (TRANS ("Builtin Plugins"));
        addSubItem (builtinFolder);
        builtinFolder->createBuiltInItems (num, types);
    }

    {
        auto racksFolder = new PluginTreeGroup (TRANS ("Plugin Racks"));
        addSubItem (racksFolder);

        racksFolder->addSubItem (new PluginTreeItem (juce::String (te::RackType::getRackPresetPrefix()) + "-1",
                                                     TRANS ("Create New Empty Rack"),
                                                     te::RackInstance::xmlTypeName,
                                                     false,
                                                     false));

        int i = 0;
        for (auto rf : edit.getRackList().getTypes())
            racksFolder->addSubItem (new PluginTreeItem ("RACK__" + juce::String (i++), rf->rackName, te::RackInstance::xmlTypeName, false, false));
    }

    populateFrom (tree);
}

inline PluginTreeGroup::PluginTreeGroup (const juce::String& s) : name (s)
{
    jassert (name.isNotEmpty());
}

inline void PluginTreeGroup::populateFrom (juce::KnownPluginList::PluginTree& tree)
{
    for (auto subTree : tree.subFolders)
    {
        if (subTree->plugins.size() > 0 || subTree->subFolders.size() > 0)
        {
            auto fs = new PluginTreeGroup (subTree->folder);
            addSubItem (fs);

            fs->populateFrom (*subTree);
        }
    }

    for (const auto& pd : tree.plugins)
        addSubItem (new PluginTreeItem (pd));
}

template <class FilterClass>
void addInternalPlugin (PluginTreeBase& item, int& num, bool synth = false)
{
    item.addSubItem (new PluginTreeItem (juce::String (num++) + "_trkbuiltin",
                                         TRANS (FilterClass::getPluginName()),
                                         FilterClass::xmlTypeName,
                                         synth,
                                         false));
}

inline void PluginTreeGroup::createBuiltInItems (int& num, te::Plugin::Type types)
{
    addInternalPlugin<te::VolumeAndPanPlugin> (*this, num);
    addInternalPlugin<te::LevelMeterPlugin> (*this, num);
    addInternalPlugin<te::EqualiserPlugin> (*this, num);
    addInternalPlugin<te::ReverbPlugin> (*this, num);
    addInternalPlugin<te::DelayPlugin> (*this, num);
    addInternalPlugin<te::ChorusPlugin> (*this, num);
    addInternalPlugin<te::PhaserPlugin> (*this, num);
    addInternalPlugin<te::CompressorPlugin> (*this, num);
    addInternalPlugin<te::PitchShiftPlugin> (*this, num);
    addInternalPlugin<te::LowPassPlugin> (*this, num);
    addInternalPlugin<te::MidiModifierPlugin> (*this, num);
    addInternalPlugin<te::MidiPatchBayPlugin> (*this, num);
    addInternalPlugin<te::PatchBayPlugin> (*this, num);
    addInternalPlugin<te::AuxSendPlugin> (*this, num);
    addInternalPlugin<te::AuxReturnPlugin> (*this, num);
    addInternalPlugin<te::TextPlugin> (*this, num);
    addInternalPlugin<te::FreezePointPlugin> (*this, num);

#if TRACKTION_ENABLE_REWIRE
    addInternalPlugin<te::ReWirePlugin> (*this, num, true);
#endif

    if (types == te::Plugin::Type::allPlugins)
    {
        addInternalPlugin<te::SamplerPlugin> (*this, num, true);
        addInternalPlugin<te::FourOscPlugin> (*this, num, true);
    }

    addInternalPlugin<te::InsertPlugin> (*this, num);

#if ENABLE_INTERNAL_PLUGINS
    for (auto& d : PluginTypeBase::getAllPluginDescriptions())
        if (isPluginAuthorised (d))
            addSubItem (new PluginTreeItem (d));
#endif
}
