#pragma once

#include <tracktion_engine/tracktion_engine.h>

using namespace std::chrono_literals;
namespace te = tracktion;

struct Thumbnail : public juce::Component
{
    Thumbnail (te::TransportControl& tc)
        : transport (tc)
    {
        cursorUpdater.setCallback ([this]
                                   {
                                       updateCursorPosition();

                                       if (smartThumbnail.isGeneratingProxy() || smartThumbnail.isOutOfDate())
                                           repaint(); });
        cursor.setFill (findColour (juce::Label::textColourId));
        addAndMakeVisible (cursor);

        pendingCursorTo.setFill (juce::Colours::cyan);
        addChildComponent (pendingCursorTo);

        pendingCursorAt.setFill (juce::Colours::lightgreen);
        addChildComponent (pendingCursorAt);
    }

    void start()
    {
        cursorUpdater.startTimerHz (25);
    }

    void setFile (const te::AudioFile& file)
    {
        smartThumbnail.setNewFile (file);
        cursorUpdater.startTimerHz (25);
        repaint();
    }

    void setQuantisation (std::optional<int> numBars)
    {
        quantisationNumBars = numBars;
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds();
        const auto colour = findColour (juce::Label::textColourId);

        if (smartThumbnail.isGeneratingProxy())
        {
            g.setColour (colour.withMultipliedBrightness (0.9f));
            g.drawText ("Creating proxy: " + juce::String (juce::roundToInt (smartThumbnail.getProxyProgress() * 100.0f)) + "%",
                        r,
                        juce::Justification::centred);
        }
        else
        {
            const float brightness = smartThumbnail.isOutOfDate() ? 0.4f : 0.66f;
            g.setColour (colour.withMultipliedBrightness (brightness));
            smartThumbnail.drawChannels (g, r, { 0s, te::TimePosition::fromSeconds (smartThumbnail.getTotalLength()) }, 1.0f);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        positionToJumpAt = {};

        transport.setUserDragging (true);
        mouseDrag (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (! e.mouseWasDraggedSinceMouseDown())
            return;

        jassert (getWidth() > 0);
        const float proportion = e.position.x / getWidth();
        transport.setPosition (toPosition (transport.getLoopRange().getLength()) * proportion);
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        transport.setUserDragging (false);

        if (e.mouseWasDraggedSinceMouseDown())
            return;

        if (auto epc = transport.edit.getCurrentPlaybackContext())
        {
            auto& ts = transport.edit.tempoSequence;

            // Simple quantisation for demo purposes here
            //  1. Quantise the position to jump to
            //  2. Quantise the time to jump to it
            const float proportion = e.position.x / getWidth();
            auto positionToJumpTo = toPosition (transport.getLoopRange().getLength()) * proportion;

            if (quantisationNumBars)
            {
                positionToJumpTo = roundToNearest (toPosition (transport.getLoopRange().getLength()) * proportion, ts, *quantisationNumBars);
                positionToJumpAt = roundUp (epc->getPosition(), ts, *quantisationNumBars);
            }
            else
            {
                positionToJumpAt = {};
            }

            epc->postPosition (positionToJumpTo, positionToJumpAt);
        }
    }

private:
    te::TransportControl& transport;
    te::SmartThumbnail smartThumbnail { transport.engine, te::AudioFile (transport.engine), *this, nullptr };
    juce::DrawableRectangle cursor, pendingCursorTo, pendingCursorAt;
    te::LambdaTimer cursorUpdater;
    std::optional<int> quantisationNumBars;
    std::optional<te::TimePosition> positionToJumpAt;

    static te::TimePosition roundTo (te::TimePosition pos, const te::TempoSequence& ts, int quantisationNumBars, double adjustment)
    {
        const auto barsBeats = ts.toBarsAndBeats (pos);
        const auto nearestBar = static_cast<int> ((barsBeats.getTotalBars() / quantisationNumBars) + adjustment)
                                * quantisationNumBars;

        return ts.toTime (te::tempo::BarsAndBeats { nearestBar });
    }

    static te::TimePosition roundToNearest (te::TimePosition pos, const te::TempoSequence& ts, int quantisationNumBars)
    {
        return roundTo (pos, ts, quantisationNumBars, 0.5 - 1.0e-10);
    }

    static te::TimePosition roundUp (te::TimePosition pos, const te::TempoSequence& ts, int quantisationNumBars)
    {
        return roundTo (pos, ts, quantisationNumBars, 1.0 - 1.0e-10);
    }

    void updateCursorPosition()
    {
        const auto loopLength = transport.getLoopRange().getLength().inSeconds();
        const auto proportion = juce::exactlyEqual (loopLength, 0.0) ? 0.0 : transport.getPosition().inSeconds() / loopLength;

        auto r = getLocalBounds().toFloat();
        const float x = r.getWidth() * float (proportion);
        cursor.setRectangle (r.withWidth (2.0f).withX (x));

        // Pending cursor
        pendingCursorTo.setVisible (false);
        pendingCursorAt.setVisible (false);

        if (quantisationNumBars)
        {
            if (auto epc = transport.edit.getCurrentPlaybackContext())
            {
                if (auto pendingChange = epc->getPendingPositionChange())
                {
                    {
                        const auto pendingProportion = juce::exactlyEqual (loopLength, 0.0) ? 0.0 : pendingChange->inSeconds() / loopLength;
                        const float pendingX = r.getWidth() * float (pendingProportion);
                        pendingCursorTo.setRectangle (r.withWidth (2.0f).withX (pendingX));
                        pendingCursorTo.setVisible (true);
                    }

                    {
                        const auto pendingAtProportion = juce::exactlyEqual (loopLength, 0.0) ? 0.0 : positionToJumpAt->inSeconds() / loopLength;
                        const float pendingX = r.getWidth() * float (pendingAtProportion);
                        pendingCursorAt.setRectangle (r.withWidth (2.0f).withX (pendingX));
                        pendingCursorAt.setVisible (true);
                    }
                }
            }
        }
    }
};
