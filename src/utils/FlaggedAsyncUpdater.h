#pragma once

#include "juce_events/juce_events.h"

class FlaggedAsyncUpdater : public juce::AsyncUpdater
{
public:
    //==============================================================================
    void markAndUpdate (bool& flag)
    {
        flag = true;
        triggerAsyncUpdate();
    }

    bool compareAndReset (bool& flag) noexcept
    {
        if (! flag)
            return false;

        flag = false;
        return true;
    }
};
