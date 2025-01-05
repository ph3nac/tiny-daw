#pragma once

#include "plugin/PluginMenu.h"
#include "plugin/PluginTreeGroup.h"
#include "utils/EngineHelpers.h"
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

inline te::Plugin::Ptr showMenuAndCreatePlugin (te::Edit& edit)
{
    if (auto tree = EngineHelpers::createPluginTree (edit.engine))
    {
        PluginTreeGroup root (edit, *tree, te::Plugin::Type::allPlugins);
        PluginMenu m (root);

        if (auto type = m.runMenu (root))
            return type->create (edit);
    }

    return {};
}
