#pragma once

#include "ui/ClipComponent.h"

class MidiClipComponent : public ClipComponent
{
public:
    MidiClipComponent (EditViewState&, te::Clip::Ptr);

    te::MidiClip* getMidiClip() { return dynamic_cast<te::MidiClip*> (clip.get()); }

    void paint (juce::Graphics& g) override;
};
