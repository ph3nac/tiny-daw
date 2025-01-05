#pragma once

#include "state/EditViewState.h"
#include "utils/EngineHelpers.h"

class TrackHeaderComponent : public juce::Component,
                             private te::ValueTreeAllEventListener
{
public:
    TrackHeaderComponent (EditViewState&, te::Track::Ptr);
    ~TrackHeaderComponent() override;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void resized() override;

private:
    void valueTreeChanged() override {}
    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override;

    EditViewState& editViewState;
    te::Track::Ptr track;

    juce::ValueTree inputsState;
    juce::Label trackName;
    juce::TextButton armButton { "A" }, muteButton { "M" }, soloButton { "S" }, inputButton { "I" };
};

inline TrackHeaderComponent::TrackHeaderComponent (EditViewState& evs, te::Track::Ptr t)
    : editViewState (evs), track (t)
{
    addAndMakeVisible (trackName);
    addAndMakeVisible (armButton);
    addAndMakeVisible (muteButton);
    addAndMakeVisible (soloButton);
    addAndMakeVisible (inputButton);

    armButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::red);
    muteButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::red);
    soloButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::green);

    trackName.setText (t->getName(), juce::dontSendNotification);

    if (auto at = dynamic_cast<te::AudioTrack*> (track.get()))
    {
        inputButton.onClick = [this, at]
        {
            juce::PopupMenu m;

            if (EngineHelpers::trackHasInput (*at))
            {
                bool ticked = EngineHelpers::isInputMonitoringEnabled (*at);
                m.addItem (1000, "Input Monitoring", true, ticked);
                m.addSeparator();
            }

            if (editViewState.showWaveDevices)
            {
                int id = 1;
                for (auto instance : at->edit.getAllInputDevices())
                {
                    if (instance->getInputDevice().getDeviceType() == te::InputDevice::waveDevice)
                    {
                        bool ticked = instance->getTargets().getFirst() == at->itemID;
                        m.addItem (id++, instance->getInputDevice().getName(), true, ticked);
                    }
                }
            }

            if (editViewState.showMidiDevices)
            {
                m.addSeparator();

                int id = 100;
                for (auto instance : at->edit.getAllInputDevices())
                {
                    if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice)
                    {
                        bool ticked = instance->getTargets().getFirst() == at->itemID;
                        m.addItem (id++, instance->getInputDevice().getName(), true, ticked);
                    }
                }
            }

            int res = m.show();

            if (res == 1000)
            {
                EngineHelpers::enableInputMonitoring (*at, ! EngineHelpers::isInputMonitoringEnabled (*at));
            }
            else if (res >= 100)
            {
                int id = 100;
                for (auto instance : at->edit.getAllInputDevices())
                {
                    if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice)
                    {
                        if (id == res)
                            [[maybe_unused]]
                            auto result = instance->setTarget (at->itemID, true, &at->edit.getUndoManager(), 0);

                        id++;
                    }
                }
            }
            else if (res >= 1)
            {
                int id = 1;
                for (auto instance : at->edit.getAllInputDevices())
                {
                    if (instance->getInputDevice().getDeviceType() == te::InputDevice::waveDevice)
                    {
                        if (id == res)
                            [[maybe_unused]]
                            auto result = instance->setTarget (at->itemID, true, &at->edit.getUndoManager(), 0);

                        id++;
                    }
                }
            }
        };
        armButton.onClick = [this, at]
        {
            EngineHelpers::armTrack (*at, ! EngineHelpers::isTrackArmed (*at));
            armButton.setToggleState (EngineHelpers::isTrackArmed (*at), juce::dontSendNotification);
        };
        muteButton.onClick = [at]
        { at->setMute (! at->isMuted (false)); };
        soloButton.onClick = [at]
        { at->setSolo (! at->isSolo (false)); };

        armButton.setToggleState (EngineHelpers::isTrackArmed (*at), juce::dontSendNotification);
    }
    else
    {
        armButton.setVisible (false);
        muteButton.setVisible (false);
        soloButton.setVisible (false);
    }

    track->state.addListener (this);
    inputsState = track->edit.state.getChildWithName (te::IDs::INPUTDEVICES);
    inputsState.addListener (this);

    valueTreePropertyChanged (track->state, te::IDs::mute);
    valueTreePropertyChanged (track->state, te::IDs::solo);
    valueTreePropertyChanged (inputsState, te::IDs::targetIndex);
}

inline TrackHeaderComponent::~TrackHeaderComponent()
{
    track->state.removeListener (this);
}

inline void TrackHeaderComponent::valueTreePropertyChanged (juce::ValueTree& v, const juce::Identifier& i)
{
    if (te::TrackList::isTrack (v))
    {
        if (i == te::IDs::mute)
            muteButton.setToggleState ((bool) v[i], juce::dontSendNotification);
        else if (i == te::IDs::solo)
            soloButton.setToggleState ((bool) v[i], juce::dontSendNotification);
    }
    else if (v.hasType (te::IDs::INPUTDEVICES)
             || v.hasType (te::IDs::INPUTDEVICE)
             || v.hasType (te::IDs::INPUTDEVICEDESTINATION))
    {
        if (auto at = dynamic_cast<te::AudioTrack*> (track.get()))
        {
            armButton.setEnabled (EngineHelpers::trackHasInput (*at));
            armButton.setToggleState (EngineHelpers::isTrackArmed (*at), juce::dontSendNotification);
        }
    }
}

inline void TrackHeaderComponent::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey);
    g.fillRect (getLocalBounds().withTrimmedRight (2));

    if (editViewState.selectionManager.isSelected (track.get()))
    {
        g.setColour (juce::Colours::red);
        g.drawRect (getLocalBounds().withTrimmedRight (-4), 2);
    }
}

inline void TrackHeaderComponent::mouseDown (const juce::MouseEvent&)
{
    editViewState.selectionManager.selectOnly (track.get());
}

inline void TrackHeaderComponent::resized()
{
    auto r = getLocalBounds().reduced (4);
    trackName.setBounds (r.removeFromTop (r.getHeight() / 2));

    int w = r.getHeight();
    inputButton.setBounds (r.removeFromLeft (w));
    r.removeFromLeft (2);
    armButton.setBounds (r.removeFromLeft (w));
    r.removeFromLeft (2);
    muteButton.setBounds (r.removeFromLeft (w));
    r.removeFromLeft (2);
    soloButton.setBounds (r.removeFromLeft (w));
    r.removeFromLeft (2);
}
