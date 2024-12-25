#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace Utils
{

// オーディオファイルを読みこむ
inline void browseForAudioFile (
    tracktion::Engine& engine,
    std::function<void (const juce::File&)> fileChosenCallback)
{
    auto fileChooser = std::make_shared<juce::FileChooser> (
        "Choose an audio file",
        engine.getPropertyStorage().getDefaultLoadSaveDirectory ("TinyDAW"),
        engine.getAudioFileFormatManager()
            .readFormatManager.getWildcardForAllFormats());
    fileChooser->launchAsync (
        juce::FileBrowserComponent::openMode + juce::FileBrowserComponent::canSelectFiles,
        [fileChooser, &engine, callback = std::move (fileChosenCallback)] (const juce::FileChooser&)
        {
            const auto file = fileChooser->getResult();
            if (file.existsAsFile())
            {
                engine.getPropertyStorage().setDefaultLoadSaveDirectory (
                    "TinyDaw", file.getParentDirectory());
            }
            callback (file);
        });
}

// オーディオトラックを作成する
inline tracktion::AudioTrack* getOrInsertAudioTrackAt (tracktion::Edit& edit,
                                                       int index)
{
    edit.ensureNumberOfAudioTracks (index + 1);
    return tracktion::getAudioTracks (edit)[index];
}

inline void removeAllClips (tracktion::AudioTrack* track)
{
    auto clips = track->getClips();
    for (auto* clip : clips)
    {
        clip->removeFromParent();
    }
}

// オーディオファイルを読み込んでWaveAudioClipとして追加する
inline tracktion::WaveAudioClip::Ptr loadAudioFileAsClip (
    tracktion::Edit& edit,
    const juce::File& file)
{
    if (auto* track = tracktion::getOrInsertAudioTrackNearestIndex (edit, 0))
    {
        removeAllClips (track);

        tracktion::AudioFile audioFile { edit.engine, file };

        if (audioFile.isValid())
        {
            if (auto newClip = track->insertWaveClip (
                    file.getFileNameWithoutExtension(), file, { { {}, tracktion::TimeDuration::fromSeconds (audioFile.getLength()) }, {} }, false))
            {
                return newClip;
            }
        }
    }
    return {};
}

// editのtransportControlを使ってクリップをループ再生する
template <typename ClipType>
typename ClipType::Ptr loopAroundClip (ClipType& clip)
{
    auto& transport = clip.edit.getTransport();
    transport.setLoopRange (clip.getEditTimeRange());
    transport.looping = true;
    transport.setPosition (std::chrono::seconds (0));
    return clip;
}

} // namespace Utils
