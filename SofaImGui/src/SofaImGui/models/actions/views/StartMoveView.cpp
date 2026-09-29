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

#include <IconsFontAwesome6.h>
#include <GUIColors.h>
#include <imgui_internal.h>
#include <ProgramStyle.h>

#include <SofaImGui/models/actions/StartMove.h>
#include <SofaImGui/widgets/ProgramWidget.h>

namespace sofaimgui::models::actions {

bool StartMove::StartMoveView::showBlock(const std::string &label,
                                         const ImVec2 &size)
{
    bool hasValuesChanged = false;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    sofaimgui::widgets::BeginBlock(label, size, ProgramColors().StartMoveBlockBg);
    sofaimgui::widgets::BlockHeader(ICON_FA_FLAG, start.getComment(), hasValuesChanged);

    sofaimgui::widgets::BlockNewLine();

    { // Way point position
        sofaimgui::widgets::BeginBlockLine("wp.pos");

        RigidCoord waypoint = start.getWaypoint();
        for (int i=0; i<3; i++)
        {
            std::string id = "##wp" + std::to_string(window->DC.CursorPos.x + i);
            if (ImGui::InputDouble(id.c_str(), &waypoint[i], 0, 0, "%0.f", ImGuiInputTextFlags_CharsNoBlank))
            {
                hasValuesChanged = true;
                start.setWaypoint(waypoint);
                start.computeSpeed();
            }
            ImGui::SameLine();
        }

        sofaimgui::widgets::EndBlockLine();
    }

    sofaimgui::widgets::BlockNewLine();

    { // Way point rotation
        std::string label = "wp.rot ";
        label += (start.isFreeInRotation()? ICON_FA_LOCK_OPEN: ICON_FA_LOCK);
        label += "##wp.rot" + std::to_string(window->DC.CursorPos.x);

        if (sofaimgui::widgets::BeginBlockLockLine(label.c_str()))
        {
            start.setFreeInRotation(!start.isFreeInRotation());
        }
        ImGui::SetItemTooltip("When unlocked, TCP movement is free in rotation.");

        RigidCoord waypoint = start.getWaypoint();
        for (int i=3; i<7; i++)
        {
            std::string id = "##wp" + std::to_string(window->DC.CursorPos.x + i);
            if(start.isFreeInRotation())
                ImGui::BeginDisabled();
            if (ImGui::InputDouble(id.c_str(), &waypoint[i], 0, 0, "%0.2f", ImGuiInputTextFlags_CharsNoBlank))
            {
                hasValuesChanged = true;
                start.setWaypoint(waypoint);
                start.computeSpeed();
            }
            if(start.isFreeInRotation())
                ImGui::EndDisabled();
            ImGui::SameLine();
        }

        sofaimgui::widgets::EndBlockLine();
    }

    sofaimgui::widgets::EndBlock(size);
    return hasValuesChanged;
}

} // namespace


