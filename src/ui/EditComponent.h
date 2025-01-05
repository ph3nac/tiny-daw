#pragma once
#include "state/EditViewState.h"
#include "ui/PlayheadComponent.h"
#include "ui/TrackComponent.h"
#include "ui/TrackFooterComponent.h"
#include "ui/TrackHeaderComponent.h"
#include "utils/FlaggedAsyncUpdater.h"

class EditComponent : public juce::Component,
                      private te::ValueTreeAllEventListener,
                      private FlaggedAsyncUpdater,
                      private juce::ChangeListener
{
public:
    EditComponent (te::Edit&, te::SelectionManager&);
    ~EditComponent() override;

    EditViewState& getEditViewState() { return editViewState; }

private:
    void valueTreeChanged() override {}

    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override;
    void valueTreeChildAdded (juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged (juce::ValueTree&, int, int) override;

    void handleAsyncUpdate() override;
    void resized() override;

    void changeListenerCallback (juce::ChangeBroadcaster*) override { repaint(); }

    void buildTracks();

    te::Edit& edit;

    EditViewState editViewState;

    PlayheadComponent playhead { edit, editViewState };
    juce::OwnedArray<TrackComponent> tracks;
    juce::OwnedArray<TrackHeaderComponent> headers;
    juce::OwnedArray<TrackFooterComponent> footers;

    bool updateTracks = false, updateZoom = false;
};

inline EditComponent::EditComponent (te::Edit& e, te::SelectionManager& sm)
    : edit (e), editViewState (e, sm)
{
    edit.state.addListener (this);
    editViewState.selectionManager.addChangeListener (this);

    addAndMakeVisible (playhead);

    markAndUpdate (updateTracks);
}

inline EditComponent::~EditComponent()
{
    editViewState.selectionManager.removeChangeListener (this);
    edit.state.removeListener (this);
}

inline void EditComponent::valueTreePropertyChanged (juce::ValueTree& v, const juce::Identifier& i)
{
    if (v.hasType (IDs::EDITVIEWSTATE))
    {
        if (i == IDs::viewX1
            || i == IDs::viewX2
            || i == IDs::viewY)
        {
            markAndUpdate (updateZoom);
        }
        else if (i == IDs::showHeaders
                 || i == IDs::showFooters)
        {
            markAndUpdate (updateZoom);
        }
        else if (i == IDs::drawWaveforms)
        {
            repaint();
        }
    }
}

inline void EditComponent::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree& c)
{
    if (te::TrackList::isTrack (c))
        markAndUpdate (updateTracks);
}

inline void EditComponent::valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree& c, int)
{
    if (te::TrackList::isTrack (c))
        markAndUpdate (updateTracks);
}

inline void EditComponent::valueTreeChildOrderChanged (juce::ValueTree& v, int a, int b)
{
    if (te::TrackList::isTrack (v.getChild (a)))
        markAndUpdate (updateTracks);
    else if (te::TrackList::isTrack (v.getChild (b)))
        markAndUpdate (updateTracks);
}

inline void EditComponent::handleAsyncUpdate()
{
    if (compareAndReset (updateTracks))
        buildTracks();
    if (compareAndReset (updateZoom))
        resized();
}

inline void EditComponent::resized()
{
    jassert (headers.size() == tracks.size());

    const int trackHeight = 50, trackGap = 2;
    const int headerWidth = editViewState.showHeaders ? 150 : 0;
    const int footerWidth = editViewState.showFooters ? 150 : 0;

    playhead.setBounds (getLocalBounds().withTrimmedLeft (headerWidth).withTrimmedRight (footerWidth));

    int y = juce::roundToInt (editViewState.viewY.get());
    for (int i = 0; i < juce::jmin (headers.size(), tracks.size()); i++)
    {
        auto h = headers[i];
        auto t = tracks[i];
        auto f = footers[i];

        h->setBounds (0, y, headerWidth, trackHeight);
        t->setBounds (headerWidth, y, getWidth() - headerWidth - footerWidth, trackHeight);
        f->setBounds (getWidth() - footerWidth, y, footerWidth, trackHeight);

        y += trackHeight + trackGap;
    }

    for (auto t : tracks)
        t->resized();
}

inline void EditComponent::buildTracks()
{
    tracks.clear();
    headers.clear();
    footers.clear();

    for (auto t : getAllTracks (edit))
    {
        TrackComponent* c = nullptr;

        if (t->isMasterTrack())
        {
            if (editViewState.showMasterTrack)
                c = new TrackComponent (editViewState, t);
        }
        else if (t->isTempoTrack())
        {
            if (editViewState.showGlobalTrack)
                c = new TrackComponent (editViewState, t);
        }
        else if (t->isMarkerTrack())
        {
            if (editViewState.showMarkerTrack)
                c = new TrackComponent (editViewState, t);
        }
        else if (t->isChordTrack())
        {
            if (editViewState.showChordTrack)
                c = new TrackComponent (editViewState, t);
        }
        else if (t->isArrangerTrack())
        {
            if (editViewState.showArrangerTrack)
                c = new TrackComponent (editViewState, t);
        }
        else
        {
            c = new TrackComponent (editViewState, t);
        }

        if (c != nullptr)
        {
            tracks.add (c);
            addAndMakeVisible (c);

            auto h = new TrackHeaderComponent (editViewState, t);
            headers.add (h);
            addAndMakeVisible (h);

            auto f = new TrackFooterComponent (editViewState, t);
            footers.add (f);
            addAndMakeVisible (f);
        }
    }

    playhead.toFront (false);
    resized();
}
