#pragma once

#include <juce_core/juce_core.h>

namespace IDs
{
#define DECLARE_ID(name) const juce::Identifier name (#name);
DECLARE_ID (EDITVIEWSTATE)
DECLARE_ID (showMasterTrack)
DECLARE_ID (showGlobalTrack)
DECLARE_ID (showMarkerTrack)
DECLARE_ID (showChordTrack)
DECLARE_ID (showMidiDevices)
DECLARE_ID (showWaveDevices)
DECLARE_ID (viewX1)
DECLARE_ID (viewX2)
DECLARE_ID (viewY)
DECLARE_ID (drawWaveforms)
DECLARE_ID (showHeaders)
DECLARE_ID (showFooters)
DECLARE_ID (showArranger)
#undef DECLARE_ID
} // namespace IDs
