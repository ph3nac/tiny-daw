#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

using namespace std::chrono_literals;

namespace EngineHelpers
{
inline te::Project::Ptr createTempProject (te::Engine& engine)
{
    auto file = engine.getTemporaryFileManager().getTempDirectory().getChildFile ("temp_project").withFileExtension (te::projectFileSuffix);
    te::ProjectManager::TempProject tempProject (engine.getProjectManager(), file, true);
    return tempProject.project;
}

inline void showAudioDeviceSettings (te::Engine& engine)
{
    juce::DialogWindow::LaunchOptions o;
    o.dialogTitle = TRANS ("Audio Settings");
    o.dialogBackgroundColour = juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    o.content.setOwned (new juce::AudioDeviceSelectorComponent (engine.getDeviceManager().deviceManager,
                                                                0,
                                                                512,
                                                                1,
                                                                512,
                                                                false,
                                                                false,
                                                                true,
                                                                true));
    o.content->setSize (400, 600);
    o.launchAsync();
}

inline void browseForAudioFile (te::Engine& engine, std::function<void (const juce::File&)> fileChosenCallback)
{
    auto fc = std::make_shared<juce::FileChooser> ("Please select an audio file to load...",
                                                   engine.getPropertyStorage().getDefaultLoadSaveDirectory ("pitchAndTimeExample"),
                                                   engine.getAudioFileFormatManager().readFormatManager.getWildcardForAllFormats());

    fc->launchAsync (juce::FileBrowserComponent::openMode + juce::FileBrowserComponent::canSelectFiles,
                     [fc, &engine, callback = std::move (fileChosenCallback)] (const juce::FileChooser&)
                     {
                         const auto f = fc->getResult();

                         if (f.existsAsFile())
                             engine.getPropertyStorage().setDefaultLoadSaveDirectory ("pitchAndTimeExample", f.getParentDirectory());

                         callback (f);
                     });
}

inline void removeAllClips (te::AudioTrack& track)
{
    auto clips = track.getClips();

    for (int i = clips.size(); --i >= 0;)
        clips.getUnchecked (i)->removeFromParent();
}

inline te::AudioTrack* getOrInsertAudioTrackAt (te::Edit& edit, int index)
{
    edit.ensureNumberOfAudioTracks (index + 1);
    return te::getAudioTracks (edit)[index];
}

inline te::WaveAudioClip::Ptr loadAudioFileAsClip (te::Edit& edit, const juce::File& file)
{
    // Find the first track and delete all clips from it
    if (auto track = getOrInsertAudioTrackAt (edit, 0))
    {
        removeAllClips (*track);

        // Add a new clip to this track
        te::AudioFile audioFile (edit.engine, file);

        if (audioFile.isValid())
            if (auto newClip = track->insertWaveClip (file.getFileNameWithoutExtension(), file, { { {}, te::TimeDuration::fromSeconds (audioFile.getLength()) }, {} }, false))
                return newClip;
    }

    return {};
}

template <typename ClipType>
typename ClipType::Ptr loopAroundClip (ClipType& clip)
{
    auto& transport = clip.edit.getTransport();
    transport.setLoopRange (clip.getEditTimeRange());
    transport.looping = true;
    transport.setPosition (0s);
    transport.play (false);

    return clip;
}

enum class ReturnToStart
{
    no,
    yes
};

inline void togglePlay (te::Edit& edit, ReturnToStart rts = ReturnToStart::no)
{
    auto& transport = edit.getTransport();

    if (transport.isPlaying())
        transport.stop (false, false);
    else
    {
        if (rts == ReturnToStart::yes)
            transport.playFromStart (true);
        else
            transport.play (false);
    }
}

inline void toggleRecord (te::Edit& edit)
{
    auto& transport = edit.getTransport();

    if (transport.isRecording())
        transport.stop (true, false);
    else
        transport.record (false);
}

inline void armTrack (te::AudioTrack& t, bool arm, int position = 0)
{
    auto& edit = t.edit;
    for (auto instance : edit.getAllInputDevices())
        if (te::isOnTargetTrack (*instance, t, position))
            instance->setRecordingEnabled (t.itemID, arm);
}

inline bool isTrackArmed (te::AudioTrack& t, int position = 0)
{
    auto& edit = t.edit;
    for (auto instance : edit.getAllInputDevices())
        if (te::isOnTargetTrack (*instance, t, position))
            return instance->isRecordingEnabled (t.itemID);

    return false;
}

inline bool isInputMonitoringEnabled (te::AudioTrack& t, int position = 0)
{
    for (auto instance : t.edit.getAllInputDevices())
        if (te::isOnTargetTrack (*instance, t, position))
            return instance->isLivePlayEnabled (t);

    return false;
}

inline void enableInputMonitoring (te::AudioTrack& t, bool im, int position = 0)
{
    if (isInputMonitoringEnabled (t, position) != im)
    {
        for (auto instance : t.edit.getAllInputDevices())
        {
            if (te::isOnTargetTrack (*instance, t, position))
            {
                if (auto mode = instance->getInputDevice().getMonitorMode();
                    mode == te::InputDevice::MonitorMode::on || mode == te::InputDevice::MonitorMode::off)
                {
                    instance->getInputDevice().setMonitorMode (mode == te::InputDevice::MonitorMode::on
                                                                   ? te::InputDevice::MonitorMode::off
                                                                   : te::InputDevice::MonitorMode::on);
                }
            }
        }
    }
}

inline bool trackHasInput (te::AudioTrack& t, int position = 0)
{
    auto& edit = t.edit;
    for (auto instance : edit.getAllInputDevices())
        if (te::isOnTargetTrack (*instance, t, position))
            return true;

    return false;
}

inline std::unique_ptr<juce::KnownPluginList::PluginTree> createPluginTree (te::Engine& engine)
{
    auto& list = engine.getPluginManager().knownPluginList;

    if (auto tree = list.createTree (list.getTypes(), juce::KnownPluginList::sortByManufacturer))
        return tree;

    return {};
}

} // namespace EngineHelpers
