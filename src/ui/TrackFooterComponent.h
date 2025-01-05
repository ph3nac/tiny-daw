#pragma once

#include "plugin/pluginFunc.h"
#include "state/EditViewState.h"
#include "ui/PluginComponent.h"
#include "utils/FlaggedAsyncUpdater.h"

class TrackFooterComponent : public juce::Component,
                             private FlaggedAsyncUpdater,
                             private te::ValueTreeAllEventListener
{
public:
    TrackFooterComponent (EditViewState&, te::Track::Ptr);
    ~TrackFooterComponent() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void resized() override;

private:
    void valueTreeChanged() override {}
    void valueTreeChildAdded (juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged (juce::ValueTree&, int, int) override;

    void handleAsyncUpdate() override;

    void buildPlugins();

    EditViewState& editViewState;
    te::Track::Ptr track;

    juce::TextButton addButton { "+" };
    juce::OwnedArray<PluginComponent> plugins;

    bool updatePlugins = false;
};

inline TrackFooterComponent::TrackFooterComponent (EditViewState& evs, te::Track::Ptr t)
    : editViewState (evs), track (t)
{
    addAndMakeVisible (addButton);

    buildPlugins();

    track->state.addListener (this);

    addButton.onClick = [this]
    {
        if (auto plugin = showMenuAndCreatePlugin (track->edit))
            track->pluginList.insertPlugin (plugin, 0, &editViewState.selectionManager);
    };
}

inline TrackFooterComponent::~TrackFooterComponent()
{
    track->state.removeListener (this);
}

inline void TrackFooterComponent::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree& c)
{
    if (c.hasType (te::IDs::PLUGIN))
        markAndUpdate (updatePlugins);
}

inline void TrackFooterComponent::valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree& c, int)
{
    if (c.hasType (te::IDs::PLUGIN))
        markAndUpdate (updatePlugins);
}

inline void TrackFooterComponent::valueTreeChildOrderChanged (juce::ValueTree&, int, int)
{
    markAndUpdate (updatePlugins);
}

inline void TrackFooterComponent::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey);
    g.fillRect (getLocalBounds().withTrimmedLeft (2));

    if (editViewState.selectionManager.isSelected (track.get()))
    {
        g.setColour (juce::Colours::red);
        g.drawRect (getLocalBounds().withTrimmedLeft (-4), 2);
    }
}

inline void TrackFooterComponent::mouseDown (const juce::MouseEvent&)
{
    editViewState.selectionManager.selectOnly (track.get());
}

inline void TrackFooterComponent::resized()
{
    auto r = getLocalBounds().reduced (4);
    const int cx = 21;

    addButton.setBounds (r.removeFromLeft (cx).withSizeKeepingCentre (cx, cx));
    r.removeFromLeft (6);

    for (auto p : plugins)
    {
        p->setBounds (r.removeFromLeft (cx).withSizeKeepingCentre (cx, cx));
        r.removeFromLeft (2);
    }
}

inline void TrackFooterComponent::handleAsyncUpdate()
{
    if (compareAndReset (updatePlugins))
        buildPlugins();
}

inline void TrackFooterComponent::buildPlugins()
{
    plugins.clear();

    for (auto plugin : track->pluginList)
    {
        auto p = new PluginComponent (editViewState, plugin);
        addAndMakeVisible (p);
        plugins.add (p);
    }
    resized();
}
