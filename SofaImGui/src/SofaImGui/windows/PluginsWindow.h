/******************************************************************************
 *                 SOFA, Simulation Open-Framework Architecture                *
 *                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the Free  *
 * Software Foundation; either version 2 of the License, or (at your option)   *
 * any later version.                                                          *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but WITHOUT *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
 * more details.                                                               *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with this program. If not, see <http://www.gnu.org/licenses/>.              *
 *******************************************************************************
 * Authors: The SOFA Team and external contributors (see Authors.txt)          *
 *                                                                             *
 * Contact information: contact@sofa-framework.org                             *
 ******************************************************************************/
#pragma once

#include <SofaImGui/windows/BaseWindow.h>
#include <sofa/helper/system/PluginManager.h>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

namespace sofaimgui::windows {

class SOFAIMGUI_API PluginsWindow : public BaseWindow
{
public:

    PluginsWindow(const std::string& name);
    ~PluginsWindow() = default;

    std::string getDescription() override;

protected:
    void beforeShowWindow() override;
    void internalShowWindow() override;

    void showLeftPanel();
    void showRightPanel();

    void showInstructionsHelp();
    void showLoadPluginButton();
    void showListOfPlugins();

    sofa::type::vector<std::string> getPluginsFromIniFile(const std::string& path);

    std::string m_pluginsConfigPath; /// Plugins config file path
    bool m_configExists; /// Did we find a config file
    sofa::type::vector<std::string> m_defaultPlugins; /// List of default plugins
    std::string m_selectedPlugin; /// Plugin selected by the user (clicked on the GUI)
    bool m_isSelectedPluginDefault; /// Is selected plugin loaded by default
};

}


