#pragma once

#include <tracktion_engine/tracktion_engine.h>

#include <chrono>

#include "juce_gui_basics/juce_gui_basics.h"
namespace te = tracktion;

struct Thumbnail : public juce::Component
{
    explicit Thumbnail (te::TransportControl& tc) : transport (tc)
    {
        cursorUpdater.setCallback ([this]
                                   {
      updateCursorPosition();

      if (smartThumbnail.isGeneratingProxy() || smartThumbnail.isOutOfDate()) {
        repaint();
      } });
        cursor.setFill (findColour (juce::Label::textColourId));
        addAndMakeVisible (cursor);

        pendingCursorTo.setFill (juce::Colours::cyan);
        addChildComponent (pendingCursorTo);

        pendingCursorAt.setFill (juce::Colours::lightgreen);
        addChildComponent (pendingCursorAt);
    }

    void start() { cursorUpdater.startTimerHz (25); }

    void setFile (const te::AudioFile& file)
    {
        smartThumbnail.setNewFile (file);
        cursorUpdater.startTimerHz (25);
        repaint();
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
            smartThumbnail.drawChannels (
                g, r, { std::chrono::seconds (0), te::TimePosition::fromSeconds (smartThumbnail.getTotalLength()) }, 1.0F);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
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
    }

private:
    te::TransportControl& transport;
    te::SmartThumbnail smartThumbnail {
        transport.engine,
        te::AudioFile (transport.engine),
        *this,
        nullptr
    };
    juce::DrawableRectangle cursor, pendingCursorTo, pendingCursorAt;
    te::LambdaTimer cursorUpdater;

    static te::TimePosition roundTo (te::TimePosition pos,
                                     const te::TempoSequence& ts,
                                     int quantisationNumBars,
                                     double adjustment)
    {
        const auto barsBeats = ts.toBarsAndBeats (pos);
        const auto nearestBar =
            static_cast<int> ((barsBeats.getTotalBars() / quantisationNumBars) + adjustment) * quantisationNumBars;

        return ts.toTime (te::tempo::BarsAndBeats { nearestBar });
    }

    static te::TimePosition roundToNearest (te::TimePosition pos,
                                            const te::TempoSequence& ts,
                                            int quantisationNumBars)
    {
        return roundTo (pos, ts, quantisationNumBars, 0.5 - 1.0e-10);
    }

    static te::TimePosition roundUp (te::TimePosition pos,
                                     const te::TempoSequence& ts,
                                     int quantisationNumBars)
    {
        return roundTo (pos, ts, quantisationNumBars, 1.0 - 1.0e-10);
    }

    void updateCursorPosition()
    {
        const auto loopLength = transport.getLoopRange().getLength().inSeconds();
        const auto proportion =
            juce::exactlyEqual (loopLength, 0.0)
                ? 0.0
                : transport.getPosition().inSeconds() / loopLength;

        auto r = getLocalBounds().toFloat();
        const float x = r.getWidth() * float (proportion);
        cursor.setRectangle (r.withWidth (2.0f).withX (x));

        // Pending cursor
        pendingCursorTo.setVisible (false);
        pendingCursorAt.setVisible (false);
    }
};
