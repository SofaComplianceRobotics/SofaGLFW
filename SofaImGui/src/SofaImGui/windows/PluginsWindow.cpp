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
#include <SofaImGui/FooterStatusBar.h>

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
    static std::string configPluginPath = "plugin_list.conf.default";

    static bool configExists = (sofa::helper::system::PluginRepository.findFile(configPluginPath, "", nullptr));
    static sofa::type::vector<std::string> listDefaultPlugins = (configExists)? getPluginsFromIniFile(configPluginPath): sofa::type::vector<std::string>();
    const ImVec4 highlightColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
    static bool isSelectedPluginDefault;

    if (enabled() && isOpen())
    {
        ImGuiIO& io = ImGui::GetIO();
        const ImVec2 defaultSize = ImVec2(io.DisplaySize.x * 0.5, io.DisplaySize.y * 0.66);
        ImGui::SetNextWindowSize(defaultSize, ImGuiCond_Once);
        static std::string selectedPlugin;
        const auto& pluginMap = sofa::helper::system::PluginManager::getInstance().getPluginMap();

        if (ImGui::Begin(getName().c_str(), &m_isOpen, windowFlags | ImGuiWindowFlags_NoDocking))
        {
            if (ImGui::BeginChild("#LoadedPlugins", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), false))
            {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));

                std::string buttonText = "Load Plugin";
                float rightPosition = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - ImGui::CalcTextSize(buttonText.c_str()).x - ImGui::GetStyle().FramePadding.x * 2;

                // Instructions / help
                if (ImGui::BeginChild("#LoadPluginsInstructions", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x * 0.32), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
                    ImGui::TextWrapped("A plugin can be loaded by either:");
                    ImGui::TextWrapped("\t 1. adding the component RequiredPlugin in the scene");
                    ImGui::TextWrapped("\t 2. using the - l option of the runSofa command");
                    ImGui::TextWrapped("\t 3. adding the plugin to the default plugins config file");
                    ImGui::TextWrapped("\t 4. adding the plugin from the Plugins Manager");
                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();

                ImGui::AlignTextToFramePadding();
                ImGui::Text("List of Loaded Plugins:");

                ImGui::SameLine();
                ImGui::SetCursorPosX(rightPosition); // Set the position to the right of the area

                if (ImGui::Button(buttonText.c_str()))
                {
                    std::vector<nfdfilteritem_t> nfd_filters{ {"SOFA plugin", sofa::helper::system::DynamicLibrary::extension.c_str()} };

                    nfdchar_t* outPath;
                    auto defaultPath = sofa::helper::system::PluginRepository.getFirstPath();

                    nfdresult_t result = NFD_OpenDialog(&outPath, nfd_filters.data(), nfd_filters.size(), defaultPath.c_str());
                    if (result == NFD_OKAY)
                    {
                        if (sofa::helper::system::FileSystem::exists(outPath))
                        {
                            sofa::helper::system::PluginManager::getInstance().loadPluginByPath(outPath);
                            sofa::helper::system::PluginManager::getInstance().writeToIniFile(sofa::gui::common::BaseGUI::getConfigDirectoryPath() + "/loadedPlugins.ini");
                        }
                    }
                }
                
                // List of plugins
                if (ImGui::BeginChild("#LoadPluginsList", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding))
                {
                    static std::map<std::string, bool> isSelected;
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
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
            
            ImGui::EndChild();
            ImGui::SameLine();

            if (ImGui::BeginChild("selectedPlugin", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding))
            {
                ImGui::Text("Plugin Info:");

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
                if (ImGui::BeginChild("selectedPlugin", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding))
                {
                    if (!selectedPlugin.empty())
                    {
                        const auto pluginIt = pluginMap.find(selectedPlugin);
                        if (pluginIt != pluginMap.end())
                        {
                            ImGui::TextDisabled("Name:"); ImGui::SameLine(); ImGui::Text("%s", pluginIt->second.getModuleName());
                            ImGui::TextDisabled("Version:"); ImGui::SameLine(); ImGui::Text("%s", pluginIt->second.getModuleVersion());
                            ImGui::TextDisabled("License:"); ImGui::SameLine(); ImGui::Text("%s", pluginIt->second.getModuleLicense());
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
                                if (configExists)
                                {
                                    std::ofstream outfile(configPluginPath.c_str(), std::ios_base::app);
                                    if (outfile.is_open())
                                    {
                                        FooterStatusBar::getInstance().setTempMessage("Saving default plugins file: " + configPluginPath);
                                        outfile << "\n" << pluginIt->second.getModuleName() << " " << pluginIt->second.getModuleVersion();
                                        outfile.close();

                                        listDefaultPlugins.push_back(pluginIt->second.getModuleName());
                                        isSelectedPluginDefault = true;
                                        ImGui::BeginDisabled();
                                    }
                                }
                                else {
                                    FooterStatusBar::getInstance().setTempMessage("Could not find default plugins file: " + configPluginPath, FooterStatusBar::MERROR);
                                }
                            }

                            if(isSelectedPluginDefault)
                                ImGui::EndDisabled();
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}

sofa::type::vector<std::string> PluginsWindow::getPluginsFromIniFile(const std::string& path)
{
    sofa::type::vector<std::string> plugins;
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
    return plugins;
}

} // namespace


