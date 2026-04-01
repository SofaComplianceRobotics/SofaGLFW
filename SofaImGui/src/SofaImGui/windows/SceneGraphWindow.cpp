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

#include "Style.h"
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <sofa/gl/component/rendering3d/OglModel.h>
#include <SofaImGui/windows/SceneGraphWindow.h>
#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/FooterStatusBar.h>
#include <IconsFontAwesome6.h>
#include <SofaImGui/ObjectColor.h>
#include <SofaImGui/widgets/ImGuiDataWidget.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/helper/system/FileSystem.h>
#include <SofaGLFW/SofaGLFWBaseGUI.h>

namespace sofaimgui::windows {

SceneGraphWindow::SceneGraphWindow(const std::string& name, const bool& isWindowOpen)
{
    m_workbenches = Workbench::SCENE_EDITOR | Workbench::SIMULATION_MODE;

    m_defaultIsOpen = false;
    m_name = name;
    m_isOpen = isWindowOpen;
}

std::string SceneGraphWindow::getDescription()
{
    return "Scene graph of the simulation nodes and components.";
}

void SceneGraphWindow::clearWindow()
{
    m_openedComponents.clear();
    m_openedNodes.clear();
    m_openedComponentPopups.clear();
    m_openedNodePopups.clear();
    m_selection.clear();
}

void SceneGraphWindow::showWindow(sofaglfw::SofaGLFWBaseGUI* baseGUI, const ImGuiWindowFlags& windowFlags)
{
    std::set<sofa::core::objectmodel::BaseObject*> componentToOpen;
    std::set<sofa::simulation::Node*> nodeToOpen;
    std::set<std::pair<sofa::core::objectmodel::BaseObject*, bool>> componentToOpenContextMenu;
    std::set<std::pair<sofa::simulation::Node*, bool>> nodeToOpenContextMenu;

    auto c = baseGUI->m_selectionColor;
    m_highlightMaterial.setColor(c.r(), c.g(), c.b(), c.a());
    float s = 0.7;
    m_highlightMaterial.emissive.set(c.r()*s, c.g()*s, c.b()*s, c.a());
    m_highlightMaterial.ambient.set(c.r()*s, c.g()*s, c.b()*s, c.a());
    m_highlightMaterial.diffuse.set(c.r()*s, c.g()*s, c.b()*s, c.a());
    m_highlightMaterial.useEmissive = true;
    m_highlightMaterial.useAmbient = true;
    m_highlightMaterial.useDiffuse = true;

    if (isOpen())
    {
        showGraph(baseGUI, windowFlags, componentToOpen, nodeToOpen, componentToOpenContextMenu, nodeToOpenContextMenu);
    }

    ImGuiIO& io = ImGui::GetIO();
    const auto height = io.DisplaySize.y*0.66; // Main window size
    const ImVec2 defaultSize = ImVec2(height*0.66, height);

    { // Node context menu
        m_openedNodePopups.insert(nodeToOpenContextMenu.begin(), nodeToOpenContextMenu.end());

        sofa::type::vector<std::pair<sofa::simulation::Node*, bool>> toRemove;
        sofa::type::vector<std::pair<sofa::simulation::Node*, bool>> toUpdate;

        for (const auto& popup : m_openedNodePopups)
        {
            if (popup.second)
            {
                ImGui::OpenPopup("##NodeContextMenu");
                toUpdate.push_back(popup);
            }

            if (ImGui::BeginPopup("##NodeContextMenu"))
            {
                addNodeContextMenu(popup.first);
                ImGui::EndPopup();
            }
            else {
                toRemove.push_back(popup);
            }
        }

        while (!toRemove.empty())
        {
            auto it = m_openedNodePopups.find(toRemove.back());
            if (it != m_openedNodePopups.end())
            {
                m_openedNodePopups.erase(it);
            }
            toRemove.pop_back();
        }

        while (!toUpdate.empty())
        {
            auto it = m_openedNodePopups.find(toUpdate.back());
            if (it != m_openedNodePopups.end())
            {
                std::pair<sofa::simulation::Node*, bool> newKey(it->first, false);
                m_openedNodePopups.erase(it);
                m_openedNodePopups.insert(newKey);
            }
            toUpdate.pop_back();
        }
    }

    { // Component context menu
        m_openedComponentPopups.insert(componentToOpenContextMenu.begin(), componentToOpenContextMenu.end());

        sofa::type::vector<std::pair<sofa::core::objectmodel::BaseObject*, bool>> toRemove;
        sofa::type::vector<std::pair<sofa::core::objectmodel::BaseObject*, bool>> toUpdate;

        for (const auto& popup : m_openedComponentPopups)
        {
            if (popup.second)
            {
                ImGui::OpenPopup("##ComponentContextMenu");
                toUpdate.push_back(popup);
            }

            if (ImGui::BeginPopup("##ComponentContextMenu"))
            {
                addComponentContextMenu(popup.first);
                ImGui::EndPopup();
            }
            else {
                toRemove.push_back(popup);
            }
        }

        while (!toRemove.empty())
        {
            auto it = m_openedComponentPopups.find(toRemove.back());
            if (it != m_openedComponentPopups.end())
            {
                m_openedComponentPopups.erase(it);
            }
            toRemove.pop_back();
        }

        while (!toUpdate.empty())
        {
            auto it = m_openedComponentPopups.find(toUpdate.back());
            if (it != m_openedComponentPopups.end())
            {
                std::pair<sofa::core::objectmodel::BaseObject*, bool> newKey(it->first, false);
                m_openedComponentPopups.erase(it);
                m_openedComponentPopups.insert(newKey);
            }
            toUpdate.pop_back();
        }
    }

    { // Nodes window
        m_openedNodes.insert(nodeToOpen.begin(), nodeToOpen.end());

        sofa::type::vector<sofa::simulation::Node*> toRemove;

        for (auto* node : m_openedNodes)
        {
            ImGuiWindowFlags nodeWindowFlags = ImGuiWindowFlags_NoDocking;
            ImGui::SetNextWindowSize(defaultSize, ImGuiCond_Once);

            if (!showNodeWindow(node, nodeWindowFlags))
            {
                toRemove.push_back(node);
            }
        }

        while (!toRemove.empty())
        {
            auto it = m_openedNodes.find(toRemove.back());
            if (it != m_openedNodes.end())
            {
                m_openedNodes.erase(it);
            }
            toRemove.pop_back();
        }
    }

    { // Components windows
        m_openedComponents.insert(componentToOpen.begin(), componentToOpen.end());

        sofa::type::vector<sofa::core::objectmodel::BaseObject*> toRemove;

        for (auto* component : m_openedComponents)
        {
            ImGuiWindowFlags componentWindowFlags = ImGuiWindowFlags_NoDocking;
            ImGui::SetNextWindowSize(defaultSize, ImGuiCond_Once);

            if (!showComponentWindow(component, componentWindowFlags))
            {
                toRemove.push_back(component);
            }
        }

        while (!toRemove.empty())
        {
            auto it = m_openedComponents.find(toRemove.back());
            if (it != m_openedComponents.end())
            {
                m_openedComponents.erase(it);
            }
            toRemove.pop_back();
        }
    }
}

void SceneGraphWindow::getComponentIconAlert(sofa::core::objectmodel::BaseObject* object, ImVec4& objectColor, std::string& icon)
{
    // Different color for component with a message
    objectColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    if (object->countLoggedMessages({sofa::helper::logging::Message::Error,
                                        sofa::helper::logging::Message::Fatal})!=0)
    {
        icon = ICON_FA_CIRCLE_EXCLAMATION;
        objectColor = ImVec4(1.f, 0.f, 0.f, 1.f); //red
    }
    else if (object->countLoggedMessages({sofa::helper::logging::Message::Warning})!=0)
    {
        icon = ICON_FA_TRIANGLE_EXCLAMATION;
        objectColor = ImVec4(1.f, 0.5f, 0.f, 1.f); //orange
    }
    else if (object->countLoggedMessages({sofa::helper::logging::Message::Info,
                                            sofa::helper::logging::Message::Deprecated,
                                            sofa::helper::logging::Message::Advice})!=0)
    {
        icon = ICON_FA_CIRCLE_INFO;
    }
    else
    {
        objectColor = ImVec4(0.5f, 0.5f, 0.5f, 1.f); //grey //getObjectColor(object);
    }
}

void SceneGraphWindow::showGraph(sofaglfw::SofaGLFWBaseGUI* baseGUI, const ImGuiWindowFlags& windowFlags,
                                 std::set<sofa::core::objectmodel::BaseObject*>& componentToOpen,
                                 std::set<sofa::simulation::Node*>& nodeToOpen,
                                 std::set<std::pair<sofa::core::objectmodel::BaseObject*, bool>>& componentToOpenContextMenu,
                                 std::set<std::pair<sofa::simulation::Node*, bool>>& nodeToOpenContextMenu)
{
    if (ImGui::Begin(getLabel().c_str(), &m_isOpen, windowFlags))
    {
        if (!isEnabledInWorkbench())
            showInfoMessage("Modifying the scene graph is disabled in the active workbench.");

        // Top option buttons
        ImVec2 buttonSize(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());

        const bool expandAll = ImGui::Button(ICON_FA_EXPAND, buttonSize);
        ImGui::SetItemTooltip("Expand all");
        ImGui::SameLine();

        const bool collapseAll = ImGui::Button(ICON_FA_COMPRESS, buttonSize);
        ImGui::SetItemTooltip("Collapse all");
        ImGui::SameLine();

        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        static bool showSearch = false;
        static bool showFiltered = false;

        if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS, buttonSize))
        {
            showSearch = !showSearch;
            showFiltered = false;
        }
        ImGui::SetItemTooltip("Search by name");
        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_FILTER, buttonSize))
        {
            showFiltered = !showFiltered;
            showSearch = false;
        }
        ImGui::SetItemTooltip("Filter by name");

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
        static ImGuiTextFilter filter;
        ImGui::PushItemWidth(buttonSize.x * 5);
        if (showSearch)
        {
            ImGui::SameLine();
            filter.Draw("Search");
        }
        if (showFiltered)
        {
            ImGui::SameLine();
            filter.Draw("Filter");
        }
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        // Table
        unsigned int treeDepth {};

        std::function<void(sofa::simulation::Node*, sofa::simulation::Node*, const bool&, const bool&)> showNode;
        showNode = [&showNode, &treeDepth, expandAll, collapseAll, &componentToOpen, &nodeToOpen, &componentToOpenContextMenu, &nodeToOpenContextMenu, this, baseGUI](sofa::simulation::Node* parent, sofa::simulation::Node* node, const bool& showSearch, const bool& showFiltered)
        {
            const auto o = baseGUI->m_selectionColor;
            const ImVec4 selectedColor(o.r(), o.g(), o.b(), o.a());
            const ImVec4 selectedColorBg(o.r(), o.g(), o.b(), o.a()*0.2); // todo: style sheet
            const ImVec4 filteredColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

            // Node
            if (node == nullptr) return;
            ImGui::TableNextRow();
            bool highlightRow = ImGui::TableGetHoveredRow() == ImGui::TableGetRowIndex();

            if (workbench == Workbench::SCENE_EDITOR && highlightRow)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(selectedColorBg));

            ImGui::TableNextColumn();

            if (workbench == Workbench::SCENE_EDITOR) // Show add button
            {
                if (highlightRow && !node->hasTag(baseGUI->getGUITag()))
                {
                    bool newNode = showAddNodeButton(node);
                    ImGui::SameLine(0, 0);
                    if (newNode)
                        ImGui::SetNextItemOpen(true);
                }
                else
                {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX()+ImGui::GetFrameHeight());
                }
            }

            if (treeDepth == 0)
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (expandAll)
                ImGui::SetNextItemOpen(true);
            if (collapseAll)
                ImGui::SetNextItemOpen(false);

            const auto& nodeName = node->getName();
            const bool& isDeactivated = !node->is_activated.getValue();
            const bool isNodeSelected = m_selection.contains(node);
            const bool isNodeHighlighted = !filter.Filters.empty() && filter.PassFilter(nodeName.c_str()) && showSearch;

            if (isDeactivated)
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            if (!m_renaming)
            {
                if (isNodeSelected)
                    ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
                if (isNodeHighlighted)
                    ImGui::PushStyleColor(ImGuiCol_Text, filteredColor);
            }

            std::string nodeIcon = ICON_FA_SITEMAP " ";

            const bool open = showName(baseGUI, node, nodeIcon, node->getName());

            if (workbench == Workbench::SCENE_EDITOR) // Drop component from Component Window
            {
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_COMPONENT"))
                    {
                        std::string *sp = static_cast<std::string*>(payload->Data);
                        std::string componentClassName = *sp;
                        if (node)
                        {
                            sofa::core::ObjectFactory::ClassEntry entry = sofa::core::ObjectFactory::getInstance()->getEntry(componentClassName);
                            if (! entry.creatorMap.empty())
                            {
                                auto creator = entry.creatorMap.begin()->second;
                                sofa::core::objectmodel::BaseObjectDescription desc;
                                desc.setName(componentClassName);
                                const auto object = creator->createInstance(node, &desc);
                                ImGui::TreeNodeSetOpen(ImGui::GetItemID(), true);
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            auto selectedTag = sofa::core::objectmodel::Tag("GUISelected")    ;
            if (isNodeSelected && !node->hasTag(selectedTag))
            {
                node->addTag(selectedTag);
                highlightOglModels(node);
            }
            else if (!isNodeSelected && node->hasTag(selectedTag))
            {
                node->removeTag(selectedTag);
                resetOglModels(node);
            }

            if (!m_renaming)
            { // Click on node
                // Double click on the node, open the window
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        nodeToOpen.insert(node);

                    if (!ImGui::IsItemToggledOpen())
                        updateSelection(node);
                }

                // Right click, open a context menu
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                {
                    nodeToOpenContextMenu.insert(std::pair<sofa::simulation::Node*, bool>(node, true));
                }
            }

            if (!m_renaming)
                ImGui::PopStyleColor(isNodeHighlighted + isNodeSelected);

            ImGui::TableNextColumn();
            ImGui::TextDisabled("Node"); // Class Name

            bool removed = false;
            if (workbench == Workbench::SCENE_EDITOR) // Add remove button
            {
                ImGui::TableNextColumn();
                if (parent)
                {
                    removed = showRemoveNodeButton(baseGUI, parent, node);
                    ImGui::SameLine();
                }
            }

            // Components in the node
            if (!removed && open)
            {
                ImGui::Indent();
                ImGui::Indent();

                showNodeComponents(baseGUI, node,
                                   filter, showSearch, showFiltered,
                                   expandAll, collapseAll,
                                   componentToOpen, componentToOpenContextMenu);

                ++treeDepth;
                // Child nodes
                for (const auto child : node->getChildren())
                {
                    ImGui::PushID(child->getName().c_str());
                    showNode(node, dynamic_cast<sofa::simulation::Node*>(child), showSearch, showFiltered);
                    ImGui::PopID();
                }
                --treeDepth;
                ImGui::Unindent();
                ImGui::Unindent();
            }

            if (isDeactivated)
                ImGui::PopStyleColor();
        };

        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
                                       ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody;

        if (ImGui::BeginTable("SceneGraphTable", (workbench == Workbench::SCENE_EDITOR)? 3: 2, flags))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(workbench == Workbench::SCENE_EDITOR? "Type  /  Template":"Type", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
            if (workbench == Workbench::SCENE_EDITOR) // Delete buttons
                ImGui::TableSetupColumn("  " ICON_FA_TRASH_CAN, ImGuiTableColumnFlags_NoResize, ImGui::GetFrameHeightWithSpacing());
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
            ImGui::TableHeadersRow();

            sofa::simulation::Node *groot = baseGUI->getRootNode().get();

            const auto o = baseGUI->m_selectionColor;
            const ImVec4 selectedColorBg(o.r(), o.g(), o.b(), o.a()*0.2); // todo: style sheet
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, workbench == Workbench::SCENE_EDITOR? ImVec4(0., 0., 0., 0): selectedColorBg);

            showNode(nullptr, groot, showSearch, showFiltered);

            ImGui::PopStyleColor();

            baseGUI->setCurrentSelection(m_selection);

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void SceneGraphWindow::showNodeComponents(sofaglfw::SofaGLFWBaseGUI* baseGUI,
                                          sofa::simulation::Node* node,
                                          const ImGuiTextFilter& filter,
                                          const bool& showSearch, const bool& showFiltered,
                                          const bool& expandAll, const bool&collapseAll,
                                          std::set<sofa::core::objectmodel::BaseObject*>& componentToOpen,
                                          std::set<std::pair<sofa::core::objectmodel::BaseObject*, bool>>& componentToOpenContextMenu)
{
    const auto o = baseGUI->m_selectionColor;
    const ImVec4 selectedColor(o.r(), o.g(), o.b(), o.a());
    const ImVec4 selectedColorBg(o.r(), o.g(), o.b(), o.a()*0.2); // todo: style sheet
    const ImVec4 filteredColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

    int i = 0;
    for (const auto object : node->getNodeObjects())
    {
        const auto& objectName = object->getName();
        const auto objectClassName = object->getClassName();
        const bool isObjectSelected = m_selection.contains(object);
        const bool isObjectFiltered = (filter.PassFilter(objectName.c_str()) || filter.PassFilter(objectClassName.c_str()));
        const bool isObjectHighlighted = !filter.Filters.empty() && isObjectFiltered && (showSearch || showFiltered);
        const bool isObjectHidden = !filter.Filters.empty() && !isObjectFiltered && showFiltered;

        if (!isObjectHidden)
        {
            ImGui::PushID(object);
            ImGui::TableNextRow();
            bool highlightRow = ImGui::TableGetHoveredRow() == ImGui::TableGetRowIndex() || m_modifyingRow == ImGui::TableGetRowIndex();

            if (workbench == Workbench::SCENE_EDITOR && highlightRow)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(selectedColorBg));

            ImGui::TableNextColumn();

            ImGuiTreeNodeFlags objectFlags = ImGuiTreeNodeFlags_None;
            const auto& slaves = object->getSlaves();
            if (slaves.empty())
            {
                objectFlags = ImGuiTreeNodeFlags_Leaf;
            }
            else
            {
                if (expandAll)
                    ImGui::SetNextItemOpen(true);
                if (collapseAll)
                    ImGui::SetNextItemOpen(false);
            }

            ImVec4 objectColor;
            std::string icon = ICON_FA_STOP;
            getComponentIconAlert(object, objectColor, icon);

            ImGui::PushID(i++);

            if (!(m_renaming && object == m_renamingObject))
                ImGui::PushStyleColor(ImGuiCol_Text, isObjectSelected? selectedColor: objectColor);
            if (workbench == Workbench::SCENE_EDITOR && !object->hasTag(baseGUI->getGUITag()) && highlightRow)
                ImGui::AlignTextToFramePadding();
            const bool objectOpen = showName(baseGUI, object, std::string(icon + " "), "", objectFlags);
            if (!(m_renaming && object == m_renamingObject))
                ImGui::PopStyleColor();

            if (workbench != Workbench::SCENE_EDITOR) // Templates are displayed elsewhere
            {
                const auto& templateName = object->getTemplateName();
                if (!templateName.empty())
                    ImGui::SetItemTooltip("%s", (std::string("template: ")+templateName).c_str());
            }

            ImGui::PopID();

            if (!m_renaming)
            { // Double click on the component, open the window
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        componentToOpen.insert(object);

                    if (!ImGui::IsItemToggledOpen())
                        updateSelection(object);
                }

                // Right click, open a context menu
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                {
                    componentToOpenContextMenu.insert(std::pair<sofa::core::objectmodel::BaseObject*, bool>(object, true));
                }
            }

            if (!(object == m_renamingObject && m_renaming)) // Name
            {
                ImGui::SameLine(0.f, 0.f);
                if (isObjectSelected)
                    ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
                if (isObjectHighlighted)
                    ImGui::PushStyleColor(ImGuiCol_Text, filteredColor);
                ImGui::Text("%s", object->getName().c_str());
                ImGui::PopStyleColor(isObjectSelected + isObjectHighlighted);
            }

            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", objectClassName.c_str()); // Class Name

            sofa::core::ObjectFactory::ClassEntry entry = sofa::core::ObjectFactory::getInstance()->getEntry(objectClassName);
            if (! entry.creatorMap.empty())
            {
                const auto& description = entry.description;
                if (!description.empty())
                    ImGui::SetItemTooltip("%s", (description).c_str());
            }

            bool removed = false;
            if (workbench == Workbench::SCENE_EDITOR) // Show template as option
            {
                removed = showTemplateCombo(object, node);
            }

            if (workbench == Workbench::SCENE_EDITOR) // Show delete buttons
            {
                ImGui::TableNextColumn();
                if (!removed)
                    removed = showRemoveComponentButton(baseGUI, node, object);
            }

            ImGui::PopID();

            // Components created by the component
            if (!removed && objectOpen && !slaves.empty())
            {
                for (const auto &slave : slaves)
                {
                    const auto& slaveName = slave->getName();
                    const auto slaveClassName = slave->getClassName();
                    const bool isSlaveSelected = m_selection.contains(slave.get());
                    const bool isSlaveFiltered = !filter.Filters.empty() && (filter.PassFilter(slaveName.c_str()) || filter.PassFilter(slaveClassName.c_str()));
                    const bool isSlaveHighlighted = isSlaveFiltered && (showSearch || showFiltered);
                    const bool isSlaveHidden = !isSlaveFiltered && showFiltered;

                    if (!isSlaveHidden)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::PushID(slave.get());

                        ImVec4 objectColor;
                        std::string icon = ICON_FA_STOP;
                        getComponentIconAlert(object, objectColor, icon);

                        ImGui::PushStyleColor(ImGuiCol_Text, isObjectSelected? selectedColor: objectColor);
                        ImGui::TreeNodeEx(std::string(icon + " ").c_str(), // Name
                                          ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
                        ImGui::PopStyleColor();

                        if (workbench != Workbench::SCENE_EDITOR)
                        {
                            const auto& templateName = object->getTemplateName();
                            if (!templateName.empty())
                                ImGui::SetItemTooltip("%s", (std::string("template: ")+templateName).c_str());
                        }
                        { // Double click on the component, open the window
                            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                            {
                                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                                    componentToOpen.insert(slave.get());

                                updateSelection(slave.get());
                            }
                        }

                        ImGui::SameLine(0.f, 0.f);
                        if (isSlaveSelected)
                            ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
                        if (isSlaveHighlighted)
                            ImGui::PushStyleColor(ImGuiCol_Text, filteredColor);
                        ImGui::Text("%s", slave->getName().c_str());
                        ImGui::PopStyleColor(isSlaveHighlighted + isSlaveSelected);

                        ImGui::TableNextColumn();
                        ImGui::TextDisabled("%s", slave->getClassName().c_str()); // Class Name

                        if (workbench == Workbench::SCENE_EDITOR) // We cannot delete those components
                            ImGui::TableNextColumn();

                        ImGui::PopID();
                    }
                }
            }
        }
    }
}

void SceneGraphWindow::updateSelection(sofa::core::objectmodel::Base::SPtr object)
{
    if (!m_renaming)
    {
        if (!m_selection.contains(object))
        {
            m_selection.clear();
            m_selection.insert(object);
        }
        else if (object)
        {
            m_selection.erase(object);
        }
    }
}

bool SceneGraphWindow::showComponentWindow(sofa::core::objectmodel::BaseObject* component,
                                           const ImGuiWindowFlags& windowsFlags)
{
    bool isOpen = true;

    ImVec4 objectColor;
    std::string icon;
    getComponentIconAlert(component, objectColor, icon);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
    if (ImGui::Begin((icon + " " + component->getName() + "##" + component->getPathName()).c_str(), &isOpen, windowsFlags))
    {
        std::map<std::string, std::vector<sofa::core::BaseData*> > groupMap;
        for (auto* data : component->getDataFields())
        {
            groupMap[data->getGroup()].push_back(data);
        }
        if (ImGui::BeginTabBar(("##tabs"+component->getName()).c_str(), ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
        {
            addGroupTab(groupMap);
            addLinksTab(component->getLinks());
            { // addInfosTab
                if (ImGui::BeginTabItem("Infos"))
                {
                    ImGui::TextDisabled("Type:");
                    ImGui::TextWrapped("%s", component->getClassName().c_str());
                    ImGui::Spacing();
                    ImGui::TextDisabled("Documentation:");
                    addComponentDocTextLinkOpenURL(component);
                    ImGui::Spacing();
                    ImGui::TextDisabled("Linkpath:");
                    ImGui::TextWrapped("@%s", component->getPathName().c_str());
                    ImGui::Spacing();
                    if (!component->getClass()->templateName.empty())
                    {
                        ImGui::TextDisabled("Template:");
                        ImGui::TextWrapped("%s", component->getClass()->templateName.c_str());
                    }
                    ImGui::Spacing();
                    ImGui::TextDisabled("Namespace:");
                    ImGui::TextWrapped("%s", component->getClass()->namespaceName.c_str());

                    sofa::core::ObjectFactory::ClassEntry entry = sofa::core::ObjectFactory::getInstance()->getEntry(component->getClassName());
                    if (!entry.creatorMap.empty())
                    {
                        ImGui::Spacing();
                        ImGui::TextDisabled("Description:");
                        ImGui::TextWrapped("%s", entry.description.c_str());
                    }

                    const std::string instantiationSourceFilename = component->getInstanciationSourceFileName();
                    if (!instantiationSourceFilename.empty())
                    {
                        ImGui::Spacing();
                        ImGui::TextDisabled("Instantiation:");
                        ImGui::TextWrapped("%s", component->getInstanciationSourceFileName().c_str());
                    }

                    const std::string implementationSourceFilename = component->getDefinitionSourceFileName();
                    if (!implementationSourceFilename.empty())
                    {
                        ImGui::Spacing();
                        ImGui::TextDisabled("Implementation:");
                        ImGui::TextWrapped("%s", component->getDefinitionSourceFileName().c_str());
                    }

                    ImGui::EndTabItem();
                }
            }
            addMessagesTab(component->getLoggedMessages(), icon + " " + component->getName(), icon);

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    return isOpen;
}

bool SceneGraphWindow::showNodeWindow(sofa::simulation::Node* node, const ImGuiWindowFlags& windowsFlags)
{
    bool isOpen = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
    if (ImGui::Begin((ICON_FA_SITEMAP "  " + node->getName() + "##" + node->getPathName()).c_str(), &isOpen, windowsFlags))
    {
        std::map<std::string, std::vector<sofa::core::BaseData*> > groupMap;
        for (auto* data : node->getDataFields())
        {
            groupMap[data->getGroup()].push_back(data);
        }
        if (ImGui::BeginTabBar(("##tabs"+node->getName()).c_str(), ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
        {
            addGroupTab(groupMap);
            addLinksTab(node->getLinks());
            addInfosTab(node);
            addMessagesTab(node->getLoggedMessages(), node->getName(), "");

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    return isOpen;
}

void SceneGraphWindow::addComponentDocTextLinkOpenURL(sofa::core::objectmodel::BaseObject *component)
{
    sofa::core::ObjectFactory::ClassEntry entry = sofa::core::ObjectFactory::getInstance()->getEntry(component->getClassName());

    if (!entry.creatorMap.empty() &&  !entry.documentationURL.empty() && entry.documentationURL.starts_with("http"))
        ImGui::LocalTextLinkOpenURL("Documentation", entry.documentationURL.c_str());
    else
    {
        ImGui::BeginDisabled();
        ImGui::LocalTextLinkOpenURL("Documentation", ""); // No documentation
        ImGui::EndDisabled();
    }
}

void SceneGraphWindow::addGroupTab(const std::map<std::string, std::vector<sofa::core::BaseData*> >& groupMap)
{
    for (auto& [group, datas] : groupMap)
    {
        const auto groupName = group.empty() ? "Property" : group;
        if (ImGui::BeginTabItem(groupName.c_str()))
        {
            for (auto& data : datas)
            {
                if (ImGui::CollapsingHeader(data->m_name.c_str()))
                {
                    ImGui::Indent();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::BeginDisabled();
                    ImGui::TextWrapped("%s", data->getHelp().c_str());
                    ImGui::EndDisabled();

                    if (data->getParent())
                    {
                        const auto linkPath = data->getLinkPath();
                        if (!linkPath.empty())
                        {
                            ImGui::TextWrapped("%s", linkPath.c_str());

                            if (ImGui::IsItemHovered())
                            {
                                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                            }
                        }
                    }

                    ImGui::PopStyleColor();

                    if (!isEnabledInWorkbench())
                        ImGui::BeginDisabled();
                    showWidget(*data);
                    if (!isEnabledInWorkbench())
                        ImGui::EndDisabled();

                    ImGui::Unindent();
                }
            }
            ImGui::EndTabItem();
        }
    }
}

void SceneGraphWindow::addLinksTab(const sofa::core::objectmodel::Base::VecLink& links)
{
    if (ImGui::BeginTabItem("Links"))
    {
        for (const auto* link : links)
        {
            const auto linkValue = link->getValueString();
            const auto linkTitle = link->getName();
            if (ImGui::CollapsingHeader(linkTitle.c_str()))
            {
                ImGui::Indent();
                ImGui::BeginDisabled();
                ImGui::TextWrapped("%s", link->getHelp().c_str());
                ImGui::EndDisabled();
                ImGui::TextWrapped("%s", linkValue.c_str());
                ImGui::Unindent();
            }
        }
        ImGui::EndTabItem();
    }
}

void SceneGraphWindow::addMessagesTab(const std::deque<sofa::helper::logging::Message>& messages, const std::string& name, const std::string& icon)
{
    if (ImGui::BeginTabItem((icon + std::string(" Messages")).c_str()))
    {
        if (ImGui::BeginTable(std::string("logTableComponent"+name).c_str(), 2, ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("message type", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("message", ImGuiTableColumnFlags_WidthStretch);
            for (const auto& message : messages)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                constexpr auto writeMessageType = [](const sofa::helper::logging::Message::Type t)
                {
                    switch (t)
                    {
                    case sofa::helper::logging::Message::Advice     : return ImGui::TextColored(ImVec4(0.f, 0.5686f, 0.9176f, 1.f), "[SUGGESTION]");
                    case sofa::helper::logging::Message::Deprecated : return ImGui::TextColored(ImVec4(0.5529f, 0.4314f, 0.3882f, 1.f), "[DEPRECATED]");
                    case sofa::helper::logging::Message::Warning    : return ImGui::TextColored(ImVec4(1.f, 0.4275f, 0.f, 1.f), "[WARNING]");
                    case sofa::helper::logging::Message::Info       : return ImGui::Text("[INFO]");
                    case sofa::helper::logging::Message::Error      : return ImGui::TextColored(ImVec4(0.8667f, 0.1725f, 0.f, 1.f), "[ERROR]");
                    case sofa::helper::logging::Message::Fatal      : return ImGui::TextColored(ImVec4(0.8353, 0.f, 0.f, 1.f), "[FATAL]");
                    case sofa::helper::logging::Message::TEmpty     : return ImGui::Text("[EMPTY]");
                    default: return;
                    }
                };
                writeMessageType(message.type());

                ImGui::TableNextColumn();
                ImGui::TextWrapped("%s", message.message().str().c_str());
            }
            ImGui::EndTable();
        }

        ImGui::EndTabItem();
    }
}

void SceneGraphWindow::addInfosTab(sofa::simulation::Node *node)
{
    if (ImGui::BeginTabItem("Infos"))
    {
        ImGui::TextDisabled("Type:");
        ImGui::TextWrapped("%s", node->getClassName().c_str());
        ImGui::Spacing();
        ImGui::TextDisabled("Linkpath:");
        ImGui::TextWrapped("@%s", node->getPathName().c_str());
        ImGui::Spacing();
        ImGui::TextDisabled("Namespace:");
        ImGui::TextWrapped("%s", node->getClass()->namespaceName.c_str());
        ImGui::EndTabItem();
    }
}

void SceneGraphWindow::addNodeContextMenu(sofa::simulation::Node* node)
{
    if (node)
    {
        { // Deactivate
            const bool& activated = node->is_activated.getValue();
            if (!isEnabledInWorkbench())
                ImGui::BeginDisabled();

            if(ImGui::MenuItem(activated? "Deactivate": "Activate"))
                node->setActive(!activated);

            if (!isEnabledInWorkbench())
                ImGui::EndDisabled();
        }

        ImGui::Separator();

        addBaseContextMenu(node);

        ImGui::Separator();

        ImGui::BeginDisabled();
        ImGui::LocalTextLinkOpenURL("Documentation", ""); // No documentation for node
        ImGui::EndDisabled();
    }
}

void SceneGraphWindow::addComponentContextMenu(sofa::core::objectmodel::BaseObject *component)
{
    if (component)
    {
        { // Deactivate
            ImGui::BeginDisabled();
            ImGui::MenuItem("Deactivate"); // Not possible for component
            ImGui::EndDisabled();
        }

        ImGui::Separator();

        addBaseContextMenu(component);

        ImGui::Separator();

        addComponentDocTextLinkOpenURL(component);
    }
}

void SceneGraphWindow::addBaseContextMenu(sofa::core::objectmodel::Base *object)
{
    if (object)
    {
        const std::string instantiationFilename = object->getInstanciationSourceFileName();
        const std::string implementationFilename = object->getDefinitionSourceFileName();

        if(ImGui::MenuItem("Copy Linkpath"))
            ImGui::SetClipboardText(object->getPathName().c_str());

        ImGui::Separator();

        { // Istantiation File
            if (instantiationFilename.empty())
                ImGui::BeginDisabled();

            if(ImGui::MenuItem("Open Instantiation File"))
            {
                if (sofa::helper::system::FileSystem::openFileWithDefaultApplication(instantiationFilename))
                    FooterStatusBar::getInstance().setTempMessage("Opening file : " + instantiationFilename);
                else
                    FooterStatusBar::getInstance().setTempMessage("Could not open file : " + instantiationFilename, FooterStatusBar::MERROR);
            }

            if (instantiationFilename.empty())
                ImGui::EndDisabled();
        }

        { // Implementation File
            if (implementationFilename.empty())
                ImGui::BeginDisabled();

            if(ImGui::MenuItem("Open Implementation File"))
            {
                if(sofa::helper::system::FileSystem::openFileWithDefaultApplication(implementationFilename))
                    FooterStatusBar::getInstance().setTempMessage("Opening file : " + implementationFilename);
                else
                    FooterStatusBar::getInstance().setTempMessage("Could not open file : " + implementationFilename, FooterStatusBar::MERROR);
            }

            if (implementationFilename.empty())
                ImGui::EndDisabled();
        }
    }
}

void SceneGraphWindow::highlightOglModels(sofa::simulation::Node *node)
{
    if (node)
    {
        for (const auto object : node->getNodeObjects())
        {
            auto oglModel = dynamic_cast<sofa::gl::component::rendering3d::OglModel*>(object);
            if (oglModel)
                oglModel->d_material.setValue(m_highlightMaterial);
        }

        for (const auto child : node->getChildren())
        {
            highlightOglModels(dynamic_cast<sofa::simulation::Node*>(child));
        }
    }
}

void SceneGraphWindow::resetOglModels(sofa::simulation::Node *node)
{
    if (node)
    {
        for (const auto object : node->getNodeObjects())
        {
            auto oglModel = dynamic_cast<sofa::gl::component::rendering3d::OglModel*>(object);
            if (oglModel)
                oglModel->d_material.setValue(oglModel->m_resetMaterial);
        }

        for (const auto child : node->getChildren())
        {
            resetOglModels(dynamic_cast<sofa::simulation::Node*>(child));
        }
    }
}

bool SceneGraphWindow::showTemplateCombo(sofa::core::objectmodel::BaseObject *object, sofa::simulation::Node *node)
{
    bool removed = false;

    sofa::core::ObjectFactory::ClassEntry entry = sofa::core::ObjectFactory::getInstance()->getEntry(object->getClassName());
    const size_t nbTemplates = object->getTemplateName().empty()? 0: entry.creatorMap.size();
    int rowIndex = ImGui::TableGetRowIndex();
    bool rowHovered = ImGui::TableGetHoveredRow() == rowIndex || m_modifyingRow == rowIndex;

    if (nbTemplates > 0)
    {
        // Has at least one template
        ImGui::SameLine();
        ImGui::Text("/");
        ImGui::SameLine();

        static int componentTemplateIndex = 0;
        static std::vector<std::string> componentTemplatesList;

        if (rowHovered) // Update list and selection index on hover
        {
            componentTemplatesList.clear();
            componentTemplatesList.reserve(nbTemplates);
            int templateIndex=0;
            for (const auto& [templateInstance, creator] : entry.creatorMap)
            {
                componentTemplatesList.push_back(templateInstance);
                if (templateInstance == object->getTemplateName())
                    componentTemplateIndex = templateIndex;
                templateIndex++;
            }
        }

        if (rowHovered)
        {
            if (componentTemplatesList.size() == nbTemplates)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(ImGui::CalcTextSize(object->getTemplateName().c_str()).x + ImGui::GetFrameHeight() * 2);

                std::string currentTemplate = componentTemplatesList[componentTemplateIndex];
                if (ImGui::BeginCombo("##Template", currentTemplate.c_str(), ImGuiComboFlags_None))
                {
                    m_modifyingRow = rowIndex;

                    for (size_t n = 0; n < nbTemplates; n++)
                    {
                        if (ImGui::Selectable(componentTemplatesList[n].c_str(), currentTemplate == componentTemplatesList[n]))
                        {
                            auto componentClassName = object->getName();
                            node->removeObject(object);
                            removed = true;

                            auto creator = entry.creatorMap.find(componentTemplatesList[n])->second;
                            sofa::core::objectmodel::BaseObjectDescription desc;
                            desc.setName(componentClassName);
                            creator->createInstance(node, &desc);
                            m_modifyingRow = -1;
                        }
                    }

                    ImGui::EndCombo();
                }
                else
                    m_modifyingRow = -1;
                ImGui::PopItemWidth();
            }
        }
        else
            ImGui::Text("%s", object->getTemplateName().c_str());
    }

    return removed;
}

bool SceneGraphWindow::showName(sofaglfw::SofaGLFWBaseGUI* baseGUI,
                                sofa::core::objectmodel::Base *object,
                                const std::string icon,
                                const std::string name,
                                ImGuiTreeNodeFlags objectFlags)
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().FramePadding.x); // Add padding

    bool open = false;
    if (m_renamingObject == object && !object->hasTag(baseGUI->getGUITag()) && (ImGui::IsKeyPressed(ImGuiKey_F2) || m_renaming)) // InputText to rename the object
    {
        std::string newName = object->getName();
        ImGui::InputText("##RenamingNode", &newName, ImGuiInputTextFlags_AutoSelectAll);
        if (!m_renaming)
        {
            ImGui::SetFocusID(ImGui::GetItemID(), ImGui::GetCurrentWindow());
        }
        m_renaming = true;
        open = m_renamingTreeOpen;

        if (ImGui::IsKeyPressed(ImGuiKey_Enter)) // Validate renaming
        {
            m_renaming = false;
            m_renamingObject = nullptr;
            object->setName(newName);
        }
    }
    else // Default tree
    {
        objectFlags |= ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
        open = ImGui::TreeNodeEx(std::string(icon + name).c_str(), objectFlags); // Name

        if (m_renamingObject == object)
            m_renamingTreeOpen = open;
    }

    if (m_renamingObject == object &&
        (!ImGui::IsItemFocused() || ImGui::IsMouseClicked(ImGuiMouseButton_Right) || (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemClicked(ImGuiMouseButton_Left)))
       ) // Loose focus
    {
        m_renaming = false;
        m_renamingObject = nullptr;
    }

    if (m_renamingObject != object && ImGui::IsItemFocused()) // New focus
    {
        m_renaming = false;
        m_renamingObject = object;
    }

    return open;
}

bool SceneGraphWindow::showAddNodeButton(sofa::simulation::Node *node)
{
    bool clicked = false;
    if (node)
    {
        if(ImGui::Button("\xe2\x9c\x9a", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
        {
            node->createChild("New Node");
            clicked = true;
        }
        ImGui::SetItemTooltip("Add New Node");
    }
    return clicked;
}

bool SceneGraphWindow::showRemoveNodeButton(sofaglfw::SofaGLFWBaseGUI* baseGUI, sofa::simulation::Node *parent, sofa::simulation::Node *node)
{
    bool clicked = false;
    if (node && parent && !node->hasTag(baseGUI->getGUITag()))
    {
        if (ImGui::TableGetHoveredRow() == ImGui::TableGetRowIndex() || m_modifyingRow == ImGui::TableGetRowIndex())
        {
            ImVec4 red = ImVec4(1., 0.3, 0.3, 1.); // todo: style sheet
            ImGui::PushStyleColor(ImGuiCol_Button, red);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, sofaimgui::blendColor(red, ImVec4(0.5,0.,0.,1.), 0.1)); // todo: style sheet
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, sofaimgui::blendColor(red, ImVec4(0.5,0.,0.,1.), 0.3)); // todo: style sheet

            if(ImGui::Button(ICON_FA_TRASH_CAN, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
            {
                parent->removeChild(node);
                clicked = true;
            }
            ImGui::SetItemTooltip("Delete Node");

            ImGui::PopStyleColor(3);
        }
    }
    return clicked;
}

bool SceneGraphWindow::showRemoveComponentButton(sofaglfw::SofaGLFWBaseGUI* baseGUI, sofa::simulation::Node *parent, sofa::core::objectmodel::BaseObject *component)
{
    bool clicked = false;
    if (component && parent && !component->hasTag(baseGUI->getGUITag()))
    {
        if (ImGui::TableGetHoveredRow() == ImGui::TableGetRowIndex() || m_modifyingRow == ImGui::TableGetRowIndex())
        {
            ImVec4 red = ImVec4(1., 0.3, 0.3, 1.); // todo: style sheet
            ImGui::PushStyleColor(ImGuiCol_Button, red);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, sofaimgui::blendColor(red, ImVec4(0.5,0.,0.,1.), 0.1)); // todo: style sheet
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, sofaimgui::blendColor(red, ImVec4(0.5,0.,0.,1.), 0.3)); // todo: style sheet

            if(ImGui::Button(ICON_FA_TRASH_CAN, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
            {
                parent->removeObject(component);
                clicked = true;
            }
            ImGui::SetItemTooltip("Delete Component");

            ImGui::PopStyleColor(3);
        }
    }
    return clicked;
}

}

