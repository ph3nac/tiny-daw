#pragma once

#include "state/EditViewState.h"

class ClipComponent : public juce::Component
{
public:
    ClipComponent (EditViewState&, te::Clip::Ptr);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

    te::Clip& getClip() { return *clip; }

protected:
    EditViewState& editViewState;
    te::Clip::Ptr clip;
};

inline ClipComponent::ClipComponent (EditViewState& evs, te::Clip::Ptr c)
    : editViewState (evs), clip (c)
{
}

inline void ClipComponent::paint (juce::Graphics& g)
{
    g.fillAll (clip->getColour().withAlpha (0.5f));
    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds());

    if (editViewState.selectionManager.isSelected (clip.get()))
    {
        g.setColour (juce::Colours::red);
        g.drawRect (getLocalBounds(), 2);
    }
}

inline void ClipComponent::mouseDown (const juce::MouseEvent&)
{
    editViewState.selectionManager.selectOnly (clip.get());
}
