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
#include <SofaImGui/models/actions/Wait.h>
#include <SofaImGui/widgets/ProgramWidget.h>


namespace sofaimgui::models::actions {

bool Wait::WaitView::showBlockInternal(const std::string &label,
                                       const ImVec2 &size,
                                       const bool & isSelected)
{
    bool hasValuesChanged = false;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    widgets::BeginBlock(label, size, ProgramColors().WaitBlockBg, isSelected);
    widgets::BlockHeader(ICON_FA_CIRCLE_PAUSE, wait.getComment(), hasValuesChanged);

    widgets::BlockNewLine();

    { // Duration
        widgets::BeginBlockLine("duration");
        double duration = wait.getDuration();
        std::string id = "##duration" + std::to_string(window->DC.CursorPos.x);
        if (ImGui::InputDouble(id.c_str(), &duration, 0, 0, "%0.2f", ImGuiInputTextFlags_CharsNoBlank))
        {
            hasValuesChanged = true;
            wait.setDuration(duration);
        }
        widgets::EndBlockLine();
    }

    widgets::EndBlock(label, size);
    return hasValuesChanged;
}

} // namespace


