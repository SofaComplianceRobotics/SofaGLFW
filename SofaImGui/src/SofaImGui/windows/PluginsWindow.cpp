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
#include <sofa/helper/system/FileRepository.h>
#include <nfd.h>
#include <SofaImGui/windows/PluginsWindow.h>
#include <sofa/helper/system/PluginManager.h>
#include <sofa/helper/system/FileSystem.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/gui/common/BaseGUI.h>

#include <fstream>

namespace sofaimgui::windows {

PluginsWindow::PluginsWindow(const std::string& name,
                             const bool& isWindowOpen)
{
    m_defaultIsOpen = false;
    m_name = name;
    m_isOpen = isWindowOpen;
}


void PluginsWindow::showWindow(const ImGuiWindowFlags &windowFlags)
{
    static sofa::type::vector<std::string> listDefaultPlugins;
    static std::string configPluginPath = "plugin_list.conf.default";
    const ImVec4 highlightColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

    static bool firstTime = true;
    if (firstTime)
    {
        firstTime = false;
        if (sofa::helper::system::PluginRepository.findFile(configPluginPath, "", nullptr))
            getPluginsFromIniFile(configPluginPath, listDefaultPlugins);
    }

    if (enabled() && isOpen())
    {
        ImGuiIO& io = ImGui::GetIO();
        const auto height = io.DisplaySize.y*0.66; // Main window size
        const ImVec2 defaultSize = ImVec2(height, height);
        ImGui::SetNextWindowSize(defaultSize, ImGuiCond_Once);

        if (ImGui::Begin(getName().c_str(), &m_isOpen, windowFlags | ImGuiWindowFlags_NoDocking))
        {
            if (ImGui::Button("Load Plugin"))
            {
                std::vector<nfdfilteritem_t> nfd_filters {{"SOFA plugin", sofa::helper::system::DynamicLibrary::extension.c_str()}};

                nfdchar_t *outPath;
                nfdresult_t result = NFD_OpenDialog(&outPath, nfd_filters.data(), nfd_filters.size(), NULL);
                if (result == NFD_OKAY)
                {
                    if (sofa::helper::system::FileSystem::exists(outPath))
                    {
                        sofa::helper::system::PluginManager::getInstance().loadPluginByPath(outPath);
                        sofa::helper::system::PluginManager::getInstance().writeToIniFile(sofa::gui::common::BaseGUI::getConfigDirectoryPath() + "/loadedPlugins.ini");
                    }
                }
            }

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
            ImGui::BeginChild("#LoadedPlugins", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);
            ImGui::TextDisabled("Loaded Plugins");

            const auto& pluginMap = sofa::helper::system::PluginManager::getInstance().getPluginMap();

            static std::map<std::string, bool> isSelected;
            static std::string selectedPlugin;
            static bool isSelectedPluginDefault;
            for (const auto& [path, plugin] : pluginMap)
            {
                const auto& pluginName = plugin.getModuleName();
                const auto& isDefault = (std::find(listDefaultPlugins.begin(), listDefaultPlugins.end(), pluginName) != listDefaultPlugins.end());
                if (isDefault)
                    ImGui::PushStyleColor(ImGuiCol_Text, highlightColor);

                if (ImGui::Selectable(pluginName, selectedPlugin == path))
                {
                    selectedPlugin = path;
                    isSelectedPluginDefault = isDefault;
                }

                if (isDefault)
                    ImGui::PopStyleColor();
            }

            ImGui::EndChild();
            ImGui::SameLine();

            if (!selectedPlugin.empty())
            {
                ImGui::BeginChild("selectedPlugin", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);
                ImGui::TextDisabled("Plugin Info:");

                const auto pluginIt = pluginMap.find(selectedPlugin);
                if (pluginIt != pluginMap.end())
                {
                    ImGui::Text("Name: %s", pluginIt->second.getModuleName());
                    ImGui::Text("Version: %s", pluginIt->second.getModuleVersion());
                    ImGui::Text("License: %s", pluginIt->second.getModuleLicense());
                    ImGui::Spacing();
                    ImGui::TextDisabled("Description:");
                    ImGui::TextWrapped("%s", pluginIt->second.getModuleDescription());
                    ImGui::Spacing();
                    ImGui::TextDisabled("Components:");
                    ImGui::TextWrapped("%s", sofa::core::ObjectFactory::getInstance()->listClassesFromTarget(pluginIt->second.getModuleName()).c_str());
                    ImGui::Spacing();
                    ImGui::TextDisabled("Path:");
                    ImGui::TextWrapped("%s", selectedPlugin.c_str());

                    if(isSelectedPluginDefault)
                        ImGui::BeginDisabled();

                    if (ImGui::Button((isSelectedPluginDefault)? "Loaded by Default": "Load by Default"))
                    {
                        if (sofa::helper::system::PluginRepository.findFile(configPluginPath, "", nullptr))
                        {
                            std::ofstream outfile(configPluginPath.c_str(), std::ios_base::app);
                            if (outfile.is_open())
                            {
                                outfile << "\n" << pluginIt->second.getModuleName() << " " << pluginIt->second.getModuleVersion();
                                outfile.close();

                                listDefaultPlugins.push_back(pluginIt->second.getModuleName());
                                isSelectedPluginDefault = true;
                                ImGui::BeginDisabled();
                            }
                        }
                    }

                    if(isSelectedPluginDefault)
                        ImGui::EndDisabled();
                }

                ImGui::EndChild();
            }
            ImGui::PopStyleColor();
        }
        ImGui::End();
    }
}

void PluginsWindow::getPluginsFromIniFile(const std::string& path, sofa::type::vector<std::string>& plugins)
{
    std::ifstream instream(path.c_str());
    std::string plugin, line;
    while(std::getline(instream, line))
    {
        if (line.empty()) continue;

        std::istringstream is(line);
        is >> plugin;
        plugins.push_back(plugin);
    }
    instream.close();
}

} // namespace


