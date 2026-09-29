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
#include <misc/cpp/imgui_stdlib.h>

#include <ProgramStyle.h>
#include <SofaImGui/models/actions/Custom.h>
#include <SofaImGui/widgets/ProgramWidget.h>


namespace sofaimgui::models::actions {

bool Custom::CustomView::showBlock(const std::string &label, const ImVec2 &size)
{
    bool hasValuesChanged = false;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    sofaimgui::widgets::BeginBlock(label, size, ProgramColors().CustomBlockBg);
    sofaimgui::widgets::BlockHeader(ICON_FA_SLIDERS, custom.getComment(), hasValuesChanged);

    sofaimgui::widgets::BlockNewLine();

    { // Duration
        sofaimgui::widgets::BeginBlockLine("duration");
        double duration = custom.getDuration();
        std::string id = "##duration" + std::to_string(window->DC.CursorPos.x);
        if (ImGui::InputDouble(id.c_str(), &duration, 0, 0, "%0.2f", ImGuiInputTextFlags_CharsNoBlank))
        {
            hasValuesChanged = true;
            custom.setDuration(duration);
        }
        sofaimgui::widgets::EndBlockLine();
    }

    { // Speed
        sofaimgui::widgets::BeginBlockLine("speed");
        std::string id = "##speed" + std::to_string(window->DC.CursorPos.x);
        double speed = custom.getSpeed();
        if (ImGui::InputDouble(id.c_str(), &speed, 0, 0, "%0.2f", ImGuiInputTextFlags_CharsNoBlank))
        {
            hasValuesChanged = true;
            custom.setSpeed(speed);
        }
        sofaimgui::widgets::EndBlockLine();
    }

    sofaimgui::widgets::BlockNewLine();

    { // Values
        sofaimgui::widgets::BeginBlockLine("start-end");
        std::string idStart = "##startValue" + std::to_string(window->DC.CursorPos.x);
        if (ImGui::InputDouble(idStart.c_str(), &custom.m_startValue, 0, 0, "%0.2f", ImGuiInputTextFlags_CharsNoBlank))
        {
            custom.computeSpeed();
            hasValuesChanged = true;
        }
        ImGui::SetItemTooltip("Start value");

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ProgramColors().Text);
        ImGui::Text("-");
        ImGui::PopStyleColor();
        ImGui::SameLine();

        std::string idEnd = "##endValue" + std::to_string(window->DC.CursorPos.x);
        if (ImGui::InputDouble(idEnd.c_str(), &custom.m_endValue, 0, 0, "%0.2f", ImGuiInputTextFlags_CharsNoBlank))
        {
            custom.computeSpeed();
            hasValuesChanged = true;
        }
        ImGui::SetItemTooltip("End Value");

        sofaimgui::widgets::EndBlockLine();
    }

    sofaimgui::widgets::BlockNewLine();

    { // Data
        sofaimgui::widgets::BeginBlockLine("data");

        auto data = custom.m_data;
        std::string label = (data && data->isValid())? data->getLabel(): "drop a data here";
        std::string tooltip = (data && data->isValid())? data->getData()->getPathName(): "Data";

        ImGui::BeginDisabled();
        ImGui::PushItemWidth(ImGui::CalcTextSize(label.c_str()).x + ImGui::GetStyle().FramePadding.x * 2);
        std::string idData = "##data" + std::to_string(window->DC.CursorPos.y);
        ImGui::InputText(idData.c_str(), &label);
        ImGui::SetItemTooltip("%s", tooltip.c_str());
        ImGui::PopItemWidth();
        ImGui::EndDisabled();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_DATAWIDGET"))
            {
                sofa::core::objectmodel::BaseData* dropedData = static_cast<sofa::core::objectmodel::BaseData*>(payload->Data);
                if (dropedData)
                {
                    custom.m_data = std::make_shared<guidata::GUIData>(std::make_shared<guidata::OwnedBaseData>(dropedData, false),
                                                                       std::make_shared<guidata::OwnedBaseData>(nullptr, false),
                                                                       std::make_shared<guidata::OwnedBaseData>(nullptr, false),
                                                                       dropedData->getName(),
                                                                       guidata::GUIData::DEFAULTGROUP,
                                                                       "");
                }
            }
            ImGui::EndDragDropTarget();
        }
        sofaimgui::widgets::EndBlockLine();
    }

    widgets::EndBlock(size);

    return hasValuesChanged;
}

} // namespace


