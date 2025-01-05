#pragma once

#include "state/EditViewState.h"

class RecordingClipComponent : public juce::Component,
                               private juce::Timer
{
public:
    RecordingClipComponent (te::Track::Ptr t, EditViewState&);

    void paint (juce::Graphics& g) override;

private:
    void timerCallback() override;
    void updatePosition();
    void initialiseThumbnailAndPunchTime();
    void drawThumbnail (juce::Graphics& g, juce::Colour waveformColour) const;
    bool getBoundsAndTime (juce::Rectangle<int>& bounds, tracktion::TimeRange& times) const;

    te::Track::Ptr track;
    EditViewState& editViewState;

    te::RecordingThumbnailManager::Thumbnail::Ptr thumbnail;
    te::TimePosition punchInTime = -1.0s;
};

inline RecordingClipComponent::RecordingClipComponent (te::Track::Ptr t, EditViewState& evs)
    : track (t), editViewState (evs)
{
    startTimerHz (10);
    initialiseThumbnailAndPunchTime();
}

inline void RecordingClipComponent::initialiseThumbnailAndPunchTime()
{
    if (auto at = dynamic_cast<te::AudioTrack*> (track.get()))
    {
        for (auto idi : at->edit.getEditInputDevices().getDevicesForTargetTrack (*at))
        {
            punchInTime = idi->getPunchInTime (at->itemID);

            if (idi->getRecordingFile (at->itemID).exists())
                thumbnail = at->edit.engine.getRecordingThumbnailManager().getThumbnailFor (idi->getRecordingFile (at->itemID));
        }
    }
}

inline void RecordingClipComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::red.withAlpha (0.5f));
    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds());

    if (editViewState.drawWaveforms)
        drawThumbnail (g, juce::Colours::black.withAlpha (0.5f));
}

inline void RecordingClipComponent::drawThumbnail (juce::Graphics& g, juce::Colour waveformColour) const
{
    if (thumbnail == nullptr)
        return;

    juce::Rectangle<int> bounds;
    tracktion::TimeRange times;
    getBoundsAndTime (bounds, times);
    auto w = bounds.getWidth();

    if (w > 0 && w < 10000)
    {
        g.setColour (waveformColour);
        thumbnail->thumb->drawChannels (g, bounds, times.getStart().inSeconds(), times.getEnd().inSeconds(), 1.0f);
    }
}

inline bool RecordingClipComponent::getBoundsAndTime (juce::Rectangle<int>& bounds, tracktion::TimeRange& times) const
{
    auto editTimeToX = [this] (te::TimePosition t)
    {
        if (auto p = getParentComponent())
            return editViewState.timeToX (t, p->getWidth()) - getX();

        return 0;
    };

    auto xToEditTime = [this] (int x)
    {
        if (auto p = getParentComponent())
            return editViewState.xToTime (x + getX(), p->getWidth());

        return te::TimePosition();
    };

    bool hasLooped = false;
    auto& edit = track->edit;

    if (auto epc = edit.getTransport().getCurrentPlaybackContext())
    {
        auto localBounds = getLocalBounds();

        auto timeStarted = thumbnail->punchInTime;
        auto unloopedPos = timeStarted + te::TimeDuration::fromSeconds (thumbnail->thumb->getTotalLength());

        auto t1 = timeStarted;
        auto t2 = unloopedPos;

        if (epc->isLooping() && t2 >= epc->getLoopTimes().getEnd())
        {
            hasLooped = true;

            t1 = juce::jmin (t1, epc->getLoopTimes().getStart());
            t2 = epc->getPosition();

            t1 = juce::jmax (editViewState.viewX1.get(), t1);
            t2 = juce::jmin (editViewState.viewX2.get(), t2);
        }
        else if (edit.recordingPunchInOut)
        {
            const auto in = thumbnail->punchInTime;
            const auto out = edit.getTransport().getLoopRange().getEnd();

            t1 = juce::jlimit (in, out, t1);
            t2 = juce::jlimit (in, out, t2);
        }

        bounds = localBounds.withX (juce::jmax (localBounds.getX(), editTimeToX (t1)))
                     .withRight (juce::jmin (localBounds.getRight(), editTimeToX (t2)));

        auto loopRange = epc->getLoopTimes();
        const auto recordedTime = unloopedPos - toDuration (epc->getLoopTimes().getStart());
        const int numLoops = (int) (recordedTime / loopRange.getLength());

        const tracktion::TimeRange editTimes (xToEditTime (bounds.getX()),
                                              xToEditTime (bounds.getRight()));

        times = (editTimes + (loopRange.getLength() * numLoops)) - toDuration (timeStarted);
    }

    return hasLooped;
}

inline void RecordingClipComponent::timerCallback()
{
    updatePosition();
}

inline void RecordingClipComponent::updatePosition()
{
    auto& edit = track->edit;

    if (auto epc = edit.getTransport().getCurrentPlaybackContext())
    {
        auto t1 = punchInTime >= 0s ? punchInTime : edit.getTransport().getTimeWhenStarted();
        auto t2 = juce::jmax (t1, epc->getUnloopedPosition());

        if (epc->isLooping())
        {
            auto loopTimes = epc->getLoopTimes();

            if (t2 >= loopTimes.getEnd())
            {
                t1 = juce::jmin (t1, loopTimes.getStart());
                t2 = loopTimes.getEnd();
            }
        }
        else if (edit.recordingPunchInOut)
        {
            auto mr = edit.getTransport().getLoopRange();
            auto in = mr.getStart();
            auto out = mr.getEnd();

            t1 = juce::jlimit (in, out, t1);
            t2 = juce::jlimit (in, out, t2);
        }

        t1 = juce::jmax (t1, editViewState.viewX1.get());
        t2 = juce::jmin (t2, editViewState.viewX2.get());

        if (auto p = getParentComponent())
        {
            int x1 = editViewState.timeToX (t1, p->getWidth());
            int x2 = editViewState.timeToX (t2, p->getWidth());

            setBounds (x1, 0, x2 - x1, p->getHeight());
            return;
        }
    }

    setBounds ({});
}
