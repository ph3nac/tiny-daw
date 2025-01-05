#pragma once

#include "state/EditViewState.h"
#include "ui/AudioClipComponent.h"
#include "ui/ClipComponent.h"
#include "ui/MidiClipComponent.h"
#include "ui/RecordingClipComponent.h"
#include "utils/FlaggedAsyncUpdater.h"

class TrackComponent : public juce::Component,
                       private te::ValueTreeAllEventListener,
                       private FlaggedAsyncUpdater,
                       private juce::ChangeListener
{
public:
    TrackComponent (EditViewState&, te::Track::Ptr);
    ~TrackComponent() override;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void resized() override;

private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    void valueTreeChanged() override {}

    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override;
    void valueTreeChildAdded (juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged (juce::ValueTree&, int, int) override;

    void handleAsyncUpdate() override;

    void buildClips();
    void buildRecordClips();

    EditViewState& editViewState;
    te::Track::Ptr track;

    juce::OwnedArray<ClipComponent> clips;
    std::unique_ptr<RecordingClipComponent> recordingClip;

    bool updateClips = false, updatePositions = false, updateRecordClips = false;
};

inline TrackComponent::TrackComponent (EditViewState& evs, te::Track::Ptr t)
    : editViewState (evs), track (t)
{
    track->state.addListener (this);
    track->edit.getTransport().addChangeListener (this);

    markAndUpdate (updateClips);
}

inline TrackComponent::~TrackComponent()
{
    track->state.removeListener (this);
    track->edit.getTransport().removeChangeListener (this);
}

inline void TrackComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::grey);

    if (editViewState.selectionManager.isSelected (track.get()))
    {
        g.setColour (juce::Colours::red);

        auto rc = getLocalBounds();
        if (editViewState.showHeaders)
            rc = rc.withTrimmedLeft (-4);
        if (editViewState.showFooters)
            rc = rc.withTrimmedRight (-4);

        g.drawRect (rc, 2);
    }
}

inline void TrackComponent::mouseDown (const juce::MouseEvent&)
{
    editViewState.selectionManager.selectOnly (track.get());
}

inline void TrackComponent::changeListenerCallback (juce::ChangeBroadcaster*)
{
    markAndUpdate (updateRecordClips);
}

inline void TrackComponent::valueTreePropertyChanged (juce::ValueTree& v, const juce::Identifier& i)
{
    if (te::Clip::isClipState (v))
    {
        if (i == te::IDs::start
            || i == te::IDs::length)
        {
            markAndUpdate (updatePositions);
        }
    }
}

inline void TrackComponent::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree& c)
{
    if (te::Clip::isClipState (c))
        markAndUpdate (updateClips);
}

inline void TrackComponent::valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree& c, int)
{
    if (te::Clip::isClipState (c))
        markAndUpdate (updateClips);
}

inline void TrackComponent::valueTreeChildOrderChanged (juce::ValueTree& v, int a, int b)
{
    if (te::Clip::isClipState (v.getChild (a)))
        markAndUpdate (updatePositions);
    else if (te::Clip::isClipState (v.getChild (b)))
        markAndUpdate (updatePositions);
}

inline void TrackComponent::handleAsyncUpdate()
{
    if (compareAndReset (updateClips))
        buildClips();
    if (compareAndReset (updatePositions))
        resized();
    if (compareAndReset (updateRecordClips))
        buildRecordClips();
}

inline void TrackComponent::resized()
{
    for (auto cc : clips)
    {
        auto& c = cc->getClip();
        auto pos = c.getPosition();
        int x1 = editViewState.timeToX (pos.getStart(), getWidth());
        int x2 = editViewState.timeToX (pos.getEnd(), getWidth());

        cc->setBounds (x1, 0, x2 - x1, getHeight());
    }
}

inline void TrackComponent::buildClips()
{
    clips.clear();

    if (auto ct = dynamic_cast<te::ClipTrack*> (track.get()))
    {
        for (auto c : ct->getClips())
        {
            ClipComponent* cc = nullptr;

            if (dynamic_cast<te::WaveAudioClip*> (c))
                cc = new AudioClipComponent (editViewState, c);
            else if (dynamic_cast<te::MidiClip*> (c))
                cc = new MidiClipComponent (editViewState, c);
            else
                cc = new ClipComponent (editViewState, c);

            clips.add (cc);
            addAndMakeVisible (cc);
        }
    }

    resized();
}

inline void TrackComponent::buildRecordClips()
{
    bool needed = false;

    if (track->edit.getTransport().isRecording())
    {
        for (auto in : track->edit.getAllInputDevices())
        {
            if (in->isRecordingActive() && track->itemID == in->getTargets().getFirst())
            {
                needed = true;
                break;
            }
        }
    }

    if (needed)
    {
        recordingClip = std::make_unique<RecordingClipComponent> (track, editViewState);
        addAndMakeVisible (*recordingClip);
    }
    else
    {
        recordingClip = nullptr;
    }
}
