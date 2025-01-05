#pragma once

#include "ui/ClipComponent.h"

class AudioClipComponent : public ClipComponent
{
public:
    AudioClipComponent (EditViewState&, te::Clip::Ptr);

    te::WaveAudioClip* getWaveAudioClip() { return dynamic_cast<te::WaveAudioClip*> (clip.get()); }

    void paint (juce::Graphics& g) override;

private:
    void updateThumbnail();
    void drawWaveform (juce::Graphics& g, te::AudioClipBase& c, te::SmartThumbnail& thumb, juce::Colour colour, int left, int right, int y, int h, int xOffset);
    void drawChannels (juce::Graphics& g, te::SmartThumbnail& thumb, juce::Rectangle<int> area, te::TimeRange time, bool useLeft, bool useRight, float leftGain, float rightGain);

    std::unique_ptr<te::SmartThumbnail> thumbnail;
};

// ===========================================================================

inline AudioClipComponent::AudioClipComponent (EditViewState& evs, te::Clip::Ptr c)
    : ClipComponent (evs, c)
{
    updateThumbnail();
}

inline void AudioClipComponent::paint (juce::Graphics& g)
{
    ClipComponent::paint (g);

    if (editViewState.drawWaveforms && thumbnail != nullptr)
        drawWaveform (g, *getWaveAudioClip(), *thumbnail, juce::Colours::black.withAlpha (0.5f), 0, getWidth(), 0, getHeight(), 0);
}

inline void AudioClipComponent::drawWaveform (juce::Graphics& g, te::AudioClipBase& c, te::SmartThumbnail& thumb, juce::Colour colour, int left, int right, int y, int h, int xOffset)
{
    auto getTimeRangeForDrawing = [this] (const int l, const int r) -> tracktion::TimeRange
    {
        if (auto p = getParentComponent())
        {
            auto t1 = editViewState.xToTime (l, p->getWidth());
            auto t2 = editViewState.xToTime (r, p->getWidth());

            return { t1, t2 };
        }

        return {};
    };

    jassert (left <= right);
    const auto gain = c.getGain();
    const auto pan = thumb.getNumChannels() == 1 ? 0.0f : c.getPan();

    const float pv = pan * gain;
    const float gainL = (gain - pv);
    const float gainR = (gain + pv);

    const bool usesTimeStretchedProxy = c.usesTimeStretchedProxy();

    const auto clipPos = c.getPosition();
    auto offset = clipPos.getOffset();
    auto speedRatio = c.getSpeedRatio();

    g.setColour (colour);

    if (usesTimeStretchedProxy)
    {
        const juce::Rectangle<int> area (left + xOffset, y, right - left, h);

        if (! thumb.isOutOfDate())
        {
            drawChannels (g, thumb, area, getTimeRangeForDrawing (left, right), c.isLeftChannelActive(), c.isRightChannelActive(), gainL, gainR);
        }
    }
    else if (c.getLoopLength() == 0s)
    {
        auto region = getTimeRangeForDrawing (left, right);

        auto t1 = (region.getStart() + offset) * speedRatio;
        auto t2 = (region.getEnd() + offset) * speedRatio;

        drawChannels (g, thumb, { left + xOffset, y, right - left, h }, { t1, t2 }, c.isLeftChannelActive(), c.isRightChannelActive(), gainL, gainR);
    }
}

inline void AudioClipComponent::drawChannels (juce::Graphics& g, te::SmartThumbnail& thumb, juce::Rectangle<int> area, te::TimeRange time, bool useLeft, bool useRight, float leftGain, float rightGain)
{
    if (useLeft && useRight && thumb.getNumChannels() > 1)
    {
        thumb.drawChannel (g, area.removeFromTop (area.getHeight() / 2), time, 0, leftGain);
        thumb.drawChannel (g, area, time, 1, rightGain);
    }
    else if (useLeft)
    {
        thumb.drawChannel (g, area, time, 0, leftGain);
    }
    else if (useRight)
    {
        thumb.drawChannel (g, area, time, 1, rightGain);
    }
}

inline void AudioClipComponent::updateThumbnail()
{
    if (auto* wac = getWaveAudioClip())
    {
        te::AudioFile af (wac->getAudioFile());

        if (af.getFile().existsAsFile() || (! wac->usesSourceFile()))
        {
            if (af.isValid())
            {
                const te::AudioFile proxy ((wac->hasAnyTakes() && wac->isShowingTakes()) ? wac->getAudioFile() : wac->getPlaybackFile());

                if (thumbnail == nullptr)
                    thumbnail = std::make_unique<te::SmartThumbnail> (wac->edit.engine, proxy, *this, &wac->edit);
                else
                    thumbnail->setNewFile (proxy);
            }
            else
            {
                thumbnail = nullptr;
            }
        }
    }
}
// ===========================================================================
