

#include "juce_gui_basics/juce_gui_basics.h"
namespace Helpers
{
static inline void addAndMakeVisible (juce::Component& parent, const juce::Array<juce::Component*>& children)
{
    for (auto c : children)
        parent.addAndMakeVisible (c);
}

static inline juce::String getStringOrDefault (const juce::String& stringToTest, const juce::String& stringToReturnIfEmpty)
{
    return stringToTest.isEmpty() ? stringToReturnIfEmpty : stringToTest;
}

static inline juce::File findRecentEdit (const juce::File& dir)
{
    auto files = dir.findChildFiles (juce::File::findFiles, false, "*.tracktionedit");

    if (files.size() > 0)
    {
        files.sort();
        return files.getLast();
    }

    return {};
}
} // namespace Helpers
