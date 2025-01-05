#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
namespace PlayHeadHelpers
{
// Quick-and-dirty function to format a timecode string
static inline juce::String timeToTimecodeString (double seconds)
{
    auto millisecs = juce::roundToInt (seconds * 1000.0);
    auto absMillisecs = std::abs (millisecs);

    return juce::String::formatted ("%02d:%02d:%02d.%03d",
                                    millisecs / 3600000,
                                    (absMillisecs / 60000) % 60,
                                    (absMillisecs / 1000) % 60,
                                    absMillisecs % 1000);
}

// Quick-and-dirty function to format a bars/beats string
static inline juce::String quarterNotePositionToBarsBeatsString (double quarterNotes, int numerator, int denominator)
{
    if (numerator == 0 || denominator == 0)
        return "1|1|000";

    auto quarterNotesPerBar = ((double) numerator * 4.0 / (double) denominator);
    auto beats = (fmod (quarterNotes, quarterNotesPerBar) / quarterNotesPerBar) * numerator;

    auto bar = ((int) quarterNotes) / quarterNotesPerBar + 1;
    auto beat = ((int) beats) + 1;
    auto ticks = ((int) (fmod (beats, 1.0) * 960.0 + 0.5));

    return juce::String::formatted ("%d|%d|%03d", bar, beat, ticks);
}

// Returns a textual description of a CurrentPositionInfo
static inline juce::String getTimecodeDisplay (const juce::AudioPlayHead::CurrentPositionInfo& pos)
{
    juce::MemoryOutputStream displayText;

    displayText << juce::String (pos.bpm, 2) << " bpm, "
                << pos.timeSigNumerator << '/' << pos.timeSigDenominator
                << "  -  " << timeToTimecodeString (pos.timeInSeconds)
                << "  -  " << quarterNotePositionToBarsBeatsString (pos.ppqPosition, pos.timeSigNumerator, pos.timeSigDenominator);

    if (pos.isRecording)
        displayText << "  (recording)";
    else if (pos.isPlaying)
        displayText << "  (playing)";
    else
        displayText << "  (stopped)";

    return displayText.toString();
}
} // namespace PlayHeadHelpers
