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

#include <SofaImGui/windows/PluginsWindow.h>

#include <sofa/helper/system/FileSystem.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/gui/common/BaseGUI.h>
#include <SofaImGui/FooterStatusBar.h>
#include <sofa/helper/system/FileRepository.h>

#include <fstream>
#include <nfd.h>

namespace sofaimgui::windows {

PluginsWindow::PluginsWindow(const std::string& name)
    : BaseWindow(name)
{
    m_defaultWorkbenches = 0;
}

std::string PluginsWindow::getDescription()
{
    return "List loaded plugins.";
}

void PluginsWindow::beforeShowWindow()
{
    if (isOpen())
    {
        ImGuiIO& io = ImGui::GetIO();
        const ImVec2 defaultSize = ImVec2(io.DisplaySize.x * 0.5, io.DisplaySize.y * 0.66);
        ImGui::SetNextWindowSize(defaultSize, ImGuiCond_Once);
    }
}

void PluginsWindow::internalShowWindow()
{
    static bool firstTime = true;
    if (firstTime)
    {
        // These filenames are defined in a cmake file and a post install script. TODO (Sofa): store in a variable
        m_pluginsConfigPath = "plugin_list.conf"; // Used in binaries
        std::string configPluginPathDefault = "plugin_list.conf.default"; // Used when compiling from source

        m_configExists = (sofa::helper::system::PluginRepository.findFile(m_pluginsConfigPath, "", nullptr));
        if (!m_configExists)
        {
            m_pluginsConfigPath = configPluginPathDefault;
            m_configExists = (sofa::helper::system::PluginRepository.findFile(m_pluginsConfigPath, "", nullptr));
        }

        m_defaultPlugins = (m_configExists)? getPluginsFromIniFile(m_pluginsConfigPath): sofa::type::vector<std::string>();
        firstTime = false;
    }

    showLeftPanel();

    ImGui::SameLine();

    showRightPanel();
}

void PluginsWindow::showLeftPanel()
{
    if (ImGui::BeginChild("#LoadedPlugins", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None))
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));

        showInstructionsHelp();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("List of Loaded Plugins:");

        showLoadPluginButton();

        showListOfLoadedPlugins();

        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}

void PluginsWindow::showRightPanel()
{
    if (ImGui::BeginChild("#SelectedPlugin", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::Text("Plugin Info:");

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
        if (ImGui::BeginChild("#SelectedPluginInfo", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (!m_selectedPlugin.empty())
            {
                const sofa::helper::system::PluginManager::PluginMap& pluginMap = sofa::helper::system::PluginManager::getInstance().getPluginMap();
                const auto pluginIt = pluginMap.find(m_selectedPlugin);
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
                    ImGui::TextWrapped("%s", m_selectedPlugin.c_str());

                    if(m_isSelectedPluginDefault)
                        ImGui::BeginDisabled();

                    if (ImGui::Button((m_isSelectedPluginDefault)? "Loaded by Default": "Load by Default"))
                    {
                        if (m_configExists)
                        {
                            std::ofstream outfile(m_pluginsConfigPath.c_str(), std::ios_base::app);
                            if (outfile.is_open())
                            {
                                FooterStatusBar::getInstance().setTempMessage("Saving default plugins file:", FooterStatusBar::MINFO, m_pluginsConfigPath);
                                outfile << "\n" << pluginIt->first << " " << pluginIt->second.getModuleVersion();
                                outfile.close();

                                m_defaultPlugins.push_back(pluginIt->second.getModuleName());
                                m_isSelectedPluginDefault = true;
                                ImGui::BeginDisabled();
                            }
                        }
                        else {
                            FooterStatusBar::getInstance().setTempMessage("Could not find default plugins file: " + m_pluginsConfigPath, FooterStatusBar::MERROR);
                        }
                    }

                    if(m_isSelectedPluginDefault)
                        ImGui::EndDisabled();
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
}

void PluginsWindow::showInstructionsHelp()
{
    if (ImGui::BeginChild("#LoadPluginsInstructions", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x * 0.32), ImGuiChildFlags_AlwaysUseWindowPadding))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));

        ImGui::TextWrapped("A plugin can be loaded by either:");
        ImGui::TextWrapped("\t 1. adding the component RequiredPlugin in the scene");
        ImGui::TextWrapped("\t 2. using the - l option of the runSofa command");
        ImGui::TextWrapped("\t 3. adding the plugin from the Plugins Manager");

        ImGui::TextWrapped("\t 4. adding the plugin to the ");
        ImGui::SameLine(0, 0);
        ImGui::TextLink("default plugins config file");
        ImGui::SetItemTooltip("%s", m_pluginsConfigPath.c_str());
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            sofa::helper::system::FileSystem::openFileWithDefaultApplication(m_pluginsConfigPath);

        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
}

void PluginsWindow::showLoadPluginButton()
{
    // Align rigth
    std::string buttonText = "Load Plugin";
    float rightPosition = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - ImGui::CalcTextSize(buttonText.c_str()).x - ImGui::GetStyle().FramePadding.x * 2;
    ImGui::SameLine();
    ImGui::SetCursorPosX(rightPosition);

    if (ImGui::Button(buttonText.c_str()))
    {
        std::vector<nfdfilteritem_t> nfd_filters{ {"SOFA plugin", sofa::helper::system::DynamicLibrary::extension.c_str()} };

        nfdchar_t* outPath;
        auto defaultPath = sofa::helper::system::PluginRepository.getFirstPath();

        nfdresult_t result = NFD_OpenDialog(&outPath, nfd_filters.data(), nfd_filters.size(), defaultPath.c_str());
        if (result == NFD_OKAY)
        {
            // Check if file exists
            if (sofa::helper::system::FileSystem::exists(outPath))
            {
                // Load plugin
                if (sofa::helper::system::PluginManager::getInstance().loadPluginByPath(outPath) == sofa::helper::system::PluginManager::PluginLoadStatus::SUCCESS)
                {
                    auto plugin = sofa::helper::system::PluginManager::getInstance().getPluginMap()[outPath];

                    // Register components
                    sofa::core::ObjectFactory* objectFactory = sofa::core::ObjectFactory::getInstance();
                    objectFactory->registerObjectsFromPlugin(plugin.getModuleName());
                }
            }
        }
    }
}

void PluginsWindow::showListOfLoadedPlugins()
{
    if (ImGui::BeginChild("#LoadPluginsList", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const sofa::helper::system::PluginManager::PluginMap& pluginMap = sofa::helper::system::PluginManager::getInstance().getPluginMap();
        const ImVec4 highlightColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

        static std::map<std::string, bool> isSelected;
        for (const auto& [path, plugin] : pluginMap)
        {
            const auto& pluginName = plugin.getModuleName();
            bool isDefault = (std::find(m_defaultPlugins.begin(), m_defaultPlugins.end(), pluginName) != m_defaultPlugins.end());
            isDefault = isDefault || (std::find(m_defaultPlugins.begin(), m_defaultPlugins.end(), path) != m_defaultPlugins.end());
            if (isDefault)
                ImGui::PushStyleColor(ImGuiCol_Text, highlightColor);

            if (ImGui::Selectable(pluginName, m_selectedPlugin == path))
            {
                m_selectedPlugin = path;
                m_isSelectedPluginDefault = isDefault;
            }

            if (isDefault)
                ImGui::PopStyleColor();
        }
    }
    ImGui::EndChild();
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


