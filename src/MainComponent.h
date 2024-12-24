#pragma once

#include <tracktion_engine/tracktion_engine.h>

#include "Thumbnail.h"
#include "Utils.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace te = tracktion;
class MainComponent : public juce::Component, private juce::ChangeListener
{
private:
    te::Engine engine { "TinyDAW" };
    te::Edit edit { engine, te::Edit::EditRole::forEditing };
    te::engine::TransportControl& transport { edit.getTransport() };

    juce::FileChooser audioFileChooser { "select an audio file",
                                         engine.getPropertyStorage().getDefaultLoadSaveDirectory ("TinyDAW"),
                                         engine.getAudioFileFormatManager().readFormatManager.getWildcardForAllFormats() };
    std::unique_ptr<juce::TemporaryFile> defaultTempProject;

    juce::TextButton playPauseButton { "Play" };
    juce::TextButton loadFileButton { "Load File" };
    Thumbnail thumbnail { transport };

    te::engine::AudioTrack* audioTrack = nullptr;
    void togglePlay()
    {
        if (transport.isPlaying())
        {
            transport.stop (false, false);
        }
        else
        {
            transport.playFromStart (true);
            // transport.play (false);
        }
    }

public:
    MainComponent()
    {
        setSize (600, 400);
        transport.addChangeListener (this);
        updatePlayButtonText();

        addAndMakeVisible (playPauseButton);
        addAndMakeVisible (loadFileButton);
        addAndMakeVisible (thumbnail);

        playPauseButton.onClick = [this]
        { togglePlay(); };
        loadFileButton.onClick = [this]
        { Utils::browseForAudioFile (engine, [this] (const juce::File& f)
                                     { setFile (f); }); };
    }
    ~MainComponent() override
    {
        edit.getTempDirectory (false).deleteRecursively();
    }

    void paint (juce::Graphics& g) override
    {
        const auto c = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
        g.fillAll (c);

        g.setColour (c.contrasting (0.2f));
        g.fillRect (getLocalBounds().reduced (2).withHeight (2));
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto topR = r.removeFromTop (30);
        playPauseButton.setBounds (topR.removeFromLeft (topR.getWidth() / 2).reduced (2));
        loadFileButton.setBounds (topR.reduced (2));

        thumbnail.setBounds (r.reduced (2));
    }

private:
    te::WaveAudioClip::Ptr getClip()
    {
        if (auto track = Utils::getOrInsertAudioTrackAt (edit, 0))
        {
            // track->getClips()はClipの配列を返すため，dynamic_castでWaveAudioClipに変換す必要がある
            if (auto clip = dynamic_cast<te::WaveAudioClip*> (track->getClips()[0]))
            {
                return clip;
            }
        }
        return {};
    }

    juce::File getSourceFile()
    {
        if (auto clip = getClip())
        {
            return clip->getAudioFile().getFile();
        }
        return {};
    }
    void setFile (const juce::File& f)
    {
        if (auto clip = Utils::loadAudioFileAsClip (edit, f))
        {
            thumbnail.setFile (Utils::loopAroundClip (*clip)->getPlaybackFile());
        }
        else
        {
            thumbnail.setFile ({ engine }); // clear the thumbnail エラーにならないが気持ち悪いな
        }
    }
    void updatePlayButtonText()
    {
        playPauseButton.setButtonText (transport.isPlaying() ? "Pause" : "Play");
    }

    void changeListenerCallback (juce::ChangeBroadcaster*) override
    {
        updatePlayButtonText();
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
