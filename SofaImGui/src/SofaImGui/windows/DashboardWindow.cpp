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

    DashboardWindow::DashboardWindow(const std::string& name,
        const bool& isWindowOpen)
    {
        m_workbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;
        m_defaultIsOpen = false;
        m_name = name;
        m_isOpen = isWindowOpen;
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
                showInfoMessage("Drag and drop data to this window (eg. from component or node window).");
                showGUIData();

                // Fill the available window space with an invisible item defining a area to drop data
                ImGui::Dummy(ImGui::GetContentRegionAvail());
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
            if (ImGui::CollapsingHeader(groupName.empty()? "Misc": groupName.c_str())) // Group title
            {
                ImGui::Indent();
                int i = 0;
                for (models::guidata::GUIData::SPtr data : itGroup.second)
                {
                    if (data)
                    {
                        ImGui::PushID(i++);
                        {
                            ImGui::AlignTextToFramePadding();
                            bool noOwner = (data->getData()->getOwner()->toBaseComponent()->getContext()==sofa::core::objectmodel::BaseContext::getDefault());

                            if (noOwner)
                            {
                                ImGui::Text(ICON_FA_TRIANGLE_EXCLAMATION);
                                ImGui::SetItemTooltip("Data has no owner");
                                ImGui::SameLine();
                            }

                            ImGui::Text("%s ", data->label.c_str()); // Value description

                            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                                ImGui::OpenPopup("##GUIDataContextMenu");

                            if (ImGui::BeginPopup("##GUIDataContextMenu"))
                            {
                                addDataContextMenu(data);
                                ImGui::EndPopup();
                            }

                            ImGui::SameLine();

                            if (noOwner)
                                ImGui::BeginDisabled();
                            ImGui::TextDisabled("%s", data->getData()->getHelp().c_str());
                            showWidget(*data->getData());
                            if (noOwner)
                                ImGui::EndDisabled();
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::Unindent();
            }
            ImGui::PopID();
        }
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
