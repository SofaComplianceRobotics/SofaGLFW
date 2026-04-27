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

#include <SofaImGui/windows/DataMonitorWindow.h>
#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/widgets/ImGuiDataWidget.h>
#include <imgui_internal.h>


namespace sofaimgui::windows {

    DataMonitorWindow::DataMonitorWindow(const std::string& name,
        const bool& isWindowOpen)
    {
        m_workbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;
        m_defaultIsOpen = false;
        m_name = name;
        m_isOpen = isWindowOpen;
    }

    std::string DataMonitorWindow::getDescription()
    {
        return "Simulation data viewer.";
    }


    void DataMonitorWindow::showWindow(const ImGuiWindowFlags& windowFlags)
    {
        SOFA_UNUSED(windowFlags);

        if (enabled() && isOpen())
        {
            ImGuiIO& io = ImGui::GetIO();
            const auto height = io.DisplaySize.y * 0.66; // Main window size
            const ImVec2 defaultSize = ImVec2(height * 0.66, height);

            ImGui::SetNextWindowSize(defaultSize, ImGuiCond_Once);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
            if (ImGui::Begin(getLabel().c_str(), &m_isOpen, ImGuiWindowFlags_NoDocking))
            {
                ImGui::Indent();
                ImGui::Spacing();
                int k = 0;
                for (auto& itGroup : m_groupedGUIData)
                {
                    ImGui::PushID(k++);
                    bool firsttime = true;

                    int i = 0;
                    for (auto& data : itGroup.second)
                    {
                        if (firsttime)
                        {
                            ImGui::TextDisabled("%s     ", data->group.c_str()); // Group title
                            ImGui::Indent();
                            firsttime = false;
                        }

                        ImGui::PushID(i++);
                        {
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("%s ", data->label.c_str()); // Value description
                            ImGui::SameLine();
                            BaseDataWidget::showWidgetAsText(*data->getData());
                        }
                        ImGui::PopID();
                    }
                    if (!firsttime)
                        ImGui::Unindent();
                    ImGui::PopID();
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }
    }
} // namespace sofaimgui::windows
