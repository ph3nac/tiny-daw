#pragma once

#include "plugin/PluginTreeGroup.h"

class PluginMenu : public juce::PopupMenu
{
public:
    PluginMenu() = default;

    PluginMenu (PluginTreeGroup& node)
    {
        for (int i = 0; i < node.getNumSubItems(); ++i)
            if (auto subNode = dynamic_cast<PluginTreeGroup*> (node.getSubItem (i)))
                addSubMenu (subNode->name, PluginMenu (*subNode), true);

        for (int i = 0; i < node.getNumSubItems(); ++i)
            if (auto subType = dynamic_cast<PluginTreeItem*> (node.getSubItem (i)))
                addItem (subType->getUniqueName().hashCode(), subType->desc.name, true, false);
    }

    static PluginTreeItem* findType (PluginTreeGroup& node, int hash)
    {
        for (int i = 0; i < node.getNumSubItems(); ++i)
            if (auto subNode = dynamic_cast<PluginTreeGroup*> (node.getSubItem (i)))
                if (auto* t = findType (*subNode, hash))
                    return t;

        for (int i = 0; i < node.getNumSubItems(); ++i)
            if (auto t = dynamic_cast<PluginTreeItem*> (node.getSubItem (i)))
                if (t->getUniqueName().hashCode() == hash)
                    return t;

        return nullptr;
    }

    PluginTreeItem* runMenu (PluginTreeGroup& node)
    {
        int res = show();

        if (res == 0)
            return nullptr;

        return findType (node, res);
    }
};
