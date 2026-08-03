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

#include "IconsFontAwesome6.h"
#include <SofaImGui/windows/DashboardWindow.h>
#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/widgets/ImGuiDataWidget.h>
#include <imgui_internal.h>


namespace sofaimgui::windows {

DashboardWindow::DashboardWindow(const std::string& name, const bool& isWindowOpen)
    : BaseWindow(name, isWindowOpen)
{
    m_workbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;
}

std::string DashboardWindow::getDescription()
{
    return "Simulation data viewer.";
}

void DashboardWindow::showWindow(const ImGuiWindowFlags& windowFlags)
{
    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &m_isOpen, windowFlags))
        {
            showOptionButtons();
            showInfoMessage("Drag and drop data to this window (eg. from component or node window).");
            showGUIData();

            // Fill the available window space with an invisible item defining a area to drop data
            ImVec2 dropRegion = ImGui::GetContentRegionAvail();
            ImGui::Dummy(ImVec2(dropRegion.x, fmax(ImGui::GetFrameHeightWithSpacing() * 5, dropRegion.y)));
            dropGUIData();

            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                ImGui::OpenPopup("##DashboardContextMenu");

            if (ImGui::BeginPopup("##DashboardContextMenu"))
            {
                addDashbordContextMenu();
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }
}

void DashboardWindow::showGUIData()
{
    ImGui::Spacing();
    int k = 0;
    for (auto& itGroup : m_groupedGUIData)
    {
        ImGui::PushID(k++);
        std::string groupName = itGroup.first;

        if (m_expandAll)
            ImGui::SetNextItemOpen(true);
        if (m_collapseAll)
            ImGui::SetNextItemOpen(false);

        if (ImGui::CollapsingHeader(groupName.empty()? "Misc": groupName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) // Group title
        {
            ImGui::Indent();
            int i = 0;
            for (models::guidata::GUIData::SPtr data : itGroup.second)
            {
                if (data)
                {
                    ImGui::PushID(i++);
                    if (m_expandAll)
                        ImGui::SetNextItemOpen(true);

                    if (m_showHelp)
                    {
                        if (ImGui::CollapsingHeader(data->label.c_str()))
                        {
                            ImGui::Indent();
                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
                            ImGui::TextWrapped("%s", data->help.c_str());
                            ImGui::PopStyleColor();
                            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                                ImGui::OpenPopup("##GUIDataContextMenu");

                            showWidget(data, false);
                            ImGui::Unindent();
                        }
                    }
                    else
                    {
                        showWidget(data, true);
                    }
                    if (ImGui::BeginPopup("##GUIDataContextMenu"))
                    {
                        addDataContextMenu(data);
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
            }
            ImGui::Unindent();
        }
        ImGui::PopID();
    }
}

void DashboardWindow::showWidget(models::guidata::GUIData::SPtr data, bool showName)
{
    bool noOwner = (data->getData()->getOwner()->toBaseComponent()->getContext()==sofa::core::objectmodel::BaseContext::getDefault());
    ImGui::AlignTextToFramePadding();
    if (noOwner)
    {
        ImGui::Text(ICON_FA_TRIANGLE_EXCLAMATION);
        ImGui::SetItemTooltip("Data is not used in the simulation");
        ImGui::SameLine();
    }

    if (showName)
    {
        ImGui::Text("%s ", data->label.c_str()); // Value description
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            ImGui::OpenPopup("##GUIDataContextMenu");
        ImGui::SetItemTooltip("%s", data->help.c_str());
        ImGui::SameLine();
    }

    sofaimgui::showWidget(*data->getData());

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        ImGui::OpenPopup("##GUIDataContextMenu");
}

void DashboardWindow::showOptionButtons()
{
    m_expandAll = ImGui::LocalButton(ICON_FA_EXPAND);
    ImGui::SetItemTooltip("Expand all");
    ImGui::SameLine();

    m_collapseAll = ImGui::LocalButton(ICON_FA_COMPRESS);
    ImGui::SetItemTooltip("Collapse all");
    ImGui::SameLine();

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    if (ImGui::LocalButton(ICON_FA_TRASH_CAN))
        clearWindow();
    ImGui::SetItemTooltip("Clear Dashboard");
    ImGui::SameLine();

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    if (ImGui::LocalButton(ICON_FA_CIRCLE_QUESTION))
        m_showHelp = !m_showHelp;
    ImGui::SetItemTooltip("Show/Hide help");
}

void DashboardWindow::dropGUIData()
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_DATAWIDGET"))
        {
            sofa::core::objectmodel::BaseData* data = static_cast<sofa::core::objectmodel::BaseData*>(payload->Data);
            if (data)
            {
                addData(data->getName(),
                        std::pair<sofa::core::BaseData*, bool>(data, false),
                        std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                        std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                        data->getOwner()? data->getOwner()->getPathName(): models::guidata::GUIData::DEFAULTGROUP,
                        data->getHelp());
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void DashboardWindow::addDataContextMenu(models::guidata::GUIData::SPtr data)
{
    if (ImGui::MenuItem("Remove"))
        removeGUIData(data);
}

void DashboardWindow::addDashbordContextMenu()
    {
        bool disable = m_GUIData.empty();
        if (disable)
            ImGui::BeginDisabled();

        if (ImGui::MenuItem("Clear Dashboard"))
            clearWindow();

        if (disable)
            ImGui::EndDisabled();
    }

} // namespace sofaimgui::windows
