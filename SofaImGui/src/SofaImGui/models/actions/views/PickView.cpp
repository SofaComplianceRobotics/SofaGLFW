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

#include <GUIColors.h>
#include <IconsFontAwesome6.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <ProgramStyle.h>
#include <SofaImGui/models/actions/Pick.h>
#include <SofaImGui/widgets/ProgramWidget.h>
#include <SofaImGui/widgets/Widgets.h>


namespace sofaimgui::models::actions {

bool Pick::PickView::showBlock(const std::string &label,
                               const ImVec2 &size)
{
    bool hasValuesChanged = false;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    sofaimgui::widgets::BeginBlock(label, size, ProgramColors().PickBlockBg);
    sofaimgui::widgets::BlockHeader(ICON_FA_HAND, pick.getComment(), hasValuesChanged);

    sofaimgui::widgets::BlockNewLine();

    { // Duration
        sofaimgui::widgets::BeginBlockLine("duration");
        std::string id = "##duration" + std::to_string(window->DC.CursorPos.x);
        double duration = pick.getDuration();
        if (ImGui::InputDouble(id.c_str(), &duration, 0, 0, "%0.2f", ImGuiInputTextFlags_CharsNoBlank))
        {
            hasValuesChanged = true;
            pick.setDuration(duration);
        }
        sofaimgui::widgets::EndBlockLine();
    }

    sofaimgui::widgets::BlockNewLine();

    { // Closing/opening distances
        sofaimgui::widgets::BeginBlockLine("distances");

        std::string idClosing = "##closing" + std::to_string(window->DC.CursorPos.x);
        double distance = pick.getClosingDistance();
        if (ImGui::InputDouble(idClosing.c_str(), &distance, 0, 0, "%0.0f", ImGuiInputTextFlags_CharsNoBlank))
        {
            hasValuesChanged = true;
            pick.setClosingDistance(distance);
        }
        ImGui::SetItemTooltip("Closing distance");

        ImGui::SameLine();

        std::string idOpening = "##opening" + std::to_string(window->DC.CursorPos.x);
        distance = pick.getOpeningDistance();
        if (ImGui::InputDouble(idOpening.c_str(), &distance, 0, 0, "%0.0f", ImGuiInputTextFlags_CharsNoBlank))
        {
            hasValuesChanged = true;
            pick.setOpeningDistance(distance);
        }
        ImGui::SetItemTooltip("Opening distance");

        sofaimgui::widgets::EndBlockLine();
    }

    sofaimgui::widgets::BlockNewLine();

    { // Release
        sofaimgui::widgets::BeginBlockLine("release");
        std::string id = "##release" + std::to_string(window->DC.CursorPos.x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ProgramColors().FrameBg);
        ImGui::PushStyleColor(ImGuiCol_Text, ProgramColors().FrameText);
        sofaimgui::widgets::ToggleButton(id.c_str(), &pick.m_release);
        ImGui::PopStyleColor(2);
        sofaimgui::widgets::EndBlockLine();
    }

    widgets::EndBlock(size);

    return hasValuesChanged;
}

} // namespace


