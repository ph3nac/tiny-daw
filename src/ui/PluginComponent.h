#pragma once

#include "state/EditViewState.h"

class PluginComponent : public juce::TextButton
{
public:
    PluginComponent (EditViewState&, te::Plugin::Ptr);
    ~PluginComponent() override;

    using TextButton::clicked;
    void clicked (const juce::ModifierKeys& modifiers) override;

private:
    EditViewState& editViewState;
    te::Plugin::Ptr plugin;
};

inline PluginComponent::PluginComponent (EditViewState& evs, te::Plugin::Ptr p)
    : editViewState (evs), plugin (p)
{
    setButtonText (plugin->getName().substring (0, 1));
}

inline PluginComponent::~PluginComponent()
{
}

inline void PluginComponent::clicked (const juce::ModifierKeys& modifiers)
{
    editViewState.selectionManager.selectOnly (plugin.get());
    if (modifiers.isPopupMenu())
    {
        juce::PopupMenu m;
        m.addItem ("Delete", [this]
                   { plugin->deleteFromParent(); });
        m.showAt (this);
    }
    else
    {
        plugin->showWindowExplicitly();
    }
}
