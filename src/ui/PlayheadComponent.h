#pragma once

#include "state/EditViewState.h"

class PlayheadComponent : public juce::Component,
                          private juce::Timer
{
public:
    PlayheadComponent (te::Edit&, EditViewState&);

    void paint (juce::Graphics& g) override;
    bool hitTest (int x, int y) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void timerCallback() override;

    te::Edit& edit;
    EditViewState& editViewState;

    int xPosition = 0;
    bool firstTimer = true;
};

inline PlayheadComponent::PlayheadComponent (te::Edit& e, EditViewState& evs)
    : edit (e), editViewState (evs)
{
    startTimerHz (30);
}

inline void PlayheadComponent::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::yellow);
    g.drawRect (xPosition, 0, 2, getHeight());
}

inline bool PlayheadComponent::hitTest (int x, int)
{
    if (std::abs (x - xPosition) <= 3)
        return true;

    return false;
}

inline void PlayheadComponent::mouseDown (const juce::MouseEvent&)
{
    edit.getTransport().setUserDragging (true);
}

inline void PlayheadComponent::mouseUp (const juce::MouseEvent&)
{
    edit.getTransport().setUserDragging (false);
}

inline void PlayheadComponent::mouseDrag (const juce::MouseEvent& e)
{
    auto t = editViewState.xToTime (e.x, getWidth());
    edit.getTransport().setPosition (t);
    timerCallback();
}

inline void PlayheadComponent::timerCallback()
{
    if (firstTimer)
    {
        // On Linux, don't set the mouse cursor until after the Component has appeared
        firstTimer = false;
        setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
    }

    int newX = editViewState.timeToX (edit.getTransport().getPosition(), getWidth());
    if (newX != xPosition)
    {
        repaint (juce::jmin (newX, xPosition) - 1, 0, juce::jmax (newX, xPosition) - juce::jmin (newX, xPosition) + 3, getHeight());
        xPosition = newX;
    }
}
