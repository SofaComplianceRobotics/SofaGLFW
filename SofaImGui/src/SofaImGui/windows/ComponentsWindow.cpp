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

#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/windows/ComponentsWindow.h>
#include <filesystem>
#include <sofa/simulation/graph/DAGNode.h>

#include <imgui.h>
#include <nfd.h>
#include <IconsFontAwesome5.h>
#include <fstream>


namespace sofaimgui::windows
{

ComponentsWindow::ComponentsWindow(const std::string& name, const bool& isWindowOpen)
{
    m_workbenches = Workbench::SCENE_EDITOR;

    m_name = name;
    m_isOpen = isWindowOpen;
}

std::string ComponentsWindow::getDescription()
{
    return "List and inspect the loaded components.";
}

void ComponentsWindow::showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags)
{
    SOFA_UNUSED(baseGUI);

    if (isOpen())
    {
        // Note for later
        // if not isEnabledInWorkbench()
        // Disable drag and drop

        if (ImGui::Begin(getLabel().c_str(), &m_isOpen, windowFlags))
        {
            static bool firstTime = true;
            if (firstTime)
            {
                sofa::helper::system::FileRepository ExamplesRepository("", {sofa::helper::Utils::getSofaPathTo("examples")});
                ExamplesRepository.findAllFilesInRepository("Component", m_examplesPaths, {".scn"});
                // TODO: loop over the plugins
                sofa::helper::system::FileRepository PluginsRepository("", {sofa::helper::Utils::getSofaPathTo("plugins")});
                PluginsRepository.findAllFilesInRepository("examples", m_examplesPaths, {".py", ".scn"});
                firstTime = false;
            }

            ImVec2 buttonSize(ImGui::GetFrameHeight(),ImGui::GetFrameHeight());
            static sofa::core::ClassEntry::SPtr selectedComponent;

            static std::vector<sofa::core::ClassEntry::SPtr> components;
            components.clear();
            sofa::core::ObjectFactory::getInstance()->getAllEntries(components);

            if (ImGui::BeginChild("#LoadedComponents", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), false))
            {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("List of Loaded Components:");
                ImGui::SetItemTooltip("Loaded %zu components and found %zu examples.",  components.size(), m_examplesPaths.size());
                ImGui::SameLine();

                showComponentsList(components, selectedComponent);
            }

            ImGui::EndChild();
            ImGui::SameLine();

            if (ImGui::BeginChild("##SelectedComponent", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::Text("Component Info:");

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
                if (ImGui::BeginChild("##SelectedComponentInfo", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AlwaysUseWindowPadding))
                {
                    if (selectedComponent)
                        showComponentInfo(selectedComponent);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();

            // if (ImGui::Button(ICON_FA_SAVE" "))
                // saveFile();
        }
        ImGui::End();
    }
}

void ComponentsWindow::showComponentsList(std::vector<sofa::core::ClassEntry::SPtr> components, sofa::core::ClassEntry::SPtr& selectedComponent)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    static ImGuiTextFilter filter;
    filter.Draw("##Filter", -1);
    ImGui::SetItemTooltip("Filter by name");
    ImGui::PopStyleVar();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
    // List of components
    if (ImGui::BeginChild("#LoadComponentsList", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_HorizontalScrollbar))
    {
        static std::map<std::string, bool> isSelected;
        for (const auto& component : components)
        {
            const auto& name = component->className;
            if (filter.PassFilter(name.c_str()))
            {
                if (ImGui::Selectable(name.c_str(), selectedComponent == component))
                {
                    selectedComponent = component;

                    m_selectedComponentExamples.clear();
                    for (const std::string& examplePath: m_examplesPaths)
                    {
                        if (examplePath.find(selectedComponent->className) != std::string::npos)
                            m_selectedComponentExamples.push_back(examplePath);
                    }
                }
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void ComponentsWindow::showComponentInfo(sofa::core::ClassEntry::SPtr selectedComponent)
{
    ImGui::TextDisabled("Name:");
    ImGui::SameLine();
    ImGui::TextWrapped("%s", selectedComponent->className.c_str());

    ImGui::TextDisabled("Description:");
    ImGui::Indent();
    ImGui::TextWrapped("%s", selectedComponent->description.c_str());
    ImGui::Unindent();

    if (!selectedComponent->creatorMap.empty() && !selectedComponent->creatorMap.contains(""))
    {
        ImGui::TextDisabled("Templates:");
        ImGui::Indent();
        for (const auto& [templateInstance, creator] : selectedComponent->creatorMap)
            ImGui::Text("%s", templateInstance.c_str());
        ImGui::Unindent();
    }

    if (!selectedComponent->documentationURL.empty())
    {
        ImGui::TextDisabled("Documentation:");
        ImGui::SameLine();
        ImGui::LocalTextLinkOpenURL("Documentation", selectedComponent->documentationURL.c_str());
    }

    if (!m_selectedComponentExamples.empty())
    {
        ImGui::TextDisabled("Examples:");
        ImGui::Indent();
        for (const auto& examplePath: m_selectedComponentExamples)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextLink));
            ImGui::TextWrapped(examplePath.filename().string().c_str());
            ImGui::PopStyleColor();

            if (ImGui::IsItemClicked())
                ImGui::SetClipboardText(examplePath.string().c_str());

            static float copiedMessageDuration = 0;
            if (ImGui::IsItemClicked() || copiedMessageDuration > 0)
                copiedMessageDuration += ImGui::GetIO().DeltaTime;
            if (copiedMessageDuration > 3)
                copiedMessageDuration = 0;

            ImGui::SetItemTooltip((copiedMessageDuration > 0)? "Copied!": "Click to copy the file path");
        }
        ImGui::Unindent();
    }

    showComponentData(selectedComponent);
}

void ComponentsWindow::showComponentData(sofa::core::ClassEntry::SPtr selectedComponent)
{
    struct DataInfo
    {
        sofa::type::vector<std::string> templateType;
        std::string description;
        std::string defaultValue;
        std::string type;
    };

    std::map<std::string, std::map<std::string, DataInfo>> allData;
    {
        const auto tmpNode = sofa::core::objectmodel::New<sofa::simulation::Node>("tmp");
        for (const auto& [templateInstance, creator] : selectedComponent->creatorMap)
        {
            sofa::core::objectmodel::BaseObjectDescription desc;
            const auto object = creator->createInstance(tmpNode.get(), &desc);
            if (object)
            {
                for (const auto& data : object->getDataFields())
                {
                    allData[data->getGroup()][data->getName()].templateType.push_back(templateInstance);
                    allData[data->getGroup()][data->getName()].description = data->getHelp();
                    allData[data->getGroup()][data->getName()].defaultValue = data->getDefaultValueString();
                    allData[data->getGroup()][data->getName()].type = data->getValueTypeString();
                }
            }
        }
    }

    if (!allData.empty())
    {
        ImGui::Spacing();
        ImGui::TextDisabled("Data:");
        ImGui::Indent();

        for (const auto& [group, templateData] : allData)
        {
            const auto groupName = group.empty() ? "Property" : group;
            if (ImGui::CollapsingHeader(groupName.c_str()))
            {
                ImGui::Indent();
                for (auto& data : templateData)
                {
                    if (ImGui::CollapsingHeader(data.first.c_str()))
                    {
                        ImGui::TextDisabled("Description:");
                        ImGui::SameLine();
                        ImGui::TextWrapped("%s", data.second.description.c_str());

                        ImGui::TextDisabled("Default Value:");
                        ImGui::SameLine();
                        ImGui::TextWrapped("%s", data.second.defaultValue.c_str());

                        ImGui::TextDisabled("Type:");
                        ImGui::SameLine();
                        ImGui::TextWrapped("%s", data.second.type.c_str());
                    }
                }
                ImGui::Unindent();
            }
        }
        ImGui::Unindent();
    }
}

void ComponentsWindow::saveFile()
{
    nfdchar_t *outPath;
    const nfdresult_t result = NFD_SaveDialog(&outPath, nullptr, 0, nullptr, "log.txt");
    if (result == NFD_OKAY)
    {
        static std::vector<sofa::core::ClassEntry::SPtr> entries;
        entries.clear();
        sofa::core::ObjectFactory::getInstance()->getAllEntries(entries);

        if (!entries.empty())
        {
            std::ofstream outputFile;
            outputFile.open(outPath, std::ios::out);

            if (outputFile.is_open())
            {
                for (const auto& entry : entries)
                {
                    struct EntryProperty
                    {
                        std::set<std::string> categories;
                        std::string target;
                        bool operator<(const EntryProperty& other) const { return target < other.target && categories < other.categories; }
                    };
                    std::set<EntryProperty> entryProperties;

                    for (const auto& [templateInstance, creator] : entry->creatorMap)
                    {
                        EntryProperty property;

                        std::vector<std::string> categories;
                        sofa::core::CategoryLibrary::getCategories(entry->creatorMap.begin()->second->getClass(), categories);
                        property.categories.insert(categories.begin(), categories.end());
                        property.target = creator->getTarget();

                        entryProperties.insert(property);
                    }

                    for (const auto& [categories, target] : entryProperties)
                    {
                        outputFile
                            << entry->className << ','
                            << sofa::helper::join(categories.begin(), categories.end(), ';') << ','
                            << target << ','
                            << '\n';
                    }
                }

                outputFile.close();
            }
        }
    }
}

} // namespace 
