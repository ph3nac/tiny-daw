#pragma once

#include "Thumbnail.h"
#include "Utils.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

class AudioTrackComponent : public juce::Component
{
public:
    AudioTrackComponent (te::engine::TransportControl& tc) : transport (tc), thumbnail (transport)
    {
        addAndMakeVisible (thumbnail);
    }

    void resized() override
    {
        thumbnail.setBounds (getLocalBounds().reduced (2));
    }

    void setFile (const juce::File& file)
    {
        // オーディオファイルを読み込み、クリップを作成して波形表示を更新する
        if (auto clip = Utils::loadAudioFileAsClip (transport.edit, file))
        {
            // テンポ情報付きのwavを読み込むとtimestretcher.initialize()のエラーが発生するため，自動的にピッチとテンポを変更するのを無効にする
            clip->setAutoPitch (false);
            clip->setAutoTempo (false);
            clip->setTimeStretchMode (te::TimeStretcher::defaultMode);
            thumbnail.setFile (Utils::loopAroundClip (*clip)->getPlaybackFile());
        }
        else
        {
            // クリップをループ再生させるよう設定し、波形サムネイルに反映
            thumbnail.setFile ({ transport.engine }); // clear the thumbnail エラーにならないが気持ち悪いな
        }
    }

private:
    te::engine::TransportControl& transport;
    Thumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioTrackComponent)
};
