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
#include "Style.h"
#include <SofaImGui/widgets/Widgets.h>
#include <sofa/helper/system/FileRepository.h>
#include <SofaImGui/windows/RecordVideoWindow.h>
#include <sofa/helper/system/FileSystem.h>
#include <sofa/gui/common/BaseGUI.h>
#include <SofaImGui/FooterStatusBar.h>
#include <nfd.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace sofaimgui::windows {

RecordVideoWindow::RecordVideoWindow(const std::string& name,
                                     const bool& isWindowOpen)
{
    m_defaultIsOpen = false;
    m_name = name;
    m_isOpen = isWindowOpen;
}

std::string RecordVideoWindow::getDescription()
{
    return "Record video of the simulation.";
}

void RecordVideoWindow::showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags)
{
    SOFA_UNUSED(baseGUI);

    if (isOpen())
    {
        ImGui::SetNextWindowSize(ImVec2(0., 0.), ImGuiCond_Once);
        if (ImGui::Begin(getName().c_str(), &m_isOpen, windowFlags))
        {
            // float rightPosition = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - ImGui::GetFrameHeightWithSpacing()*2.;

            static bool record = false;
            ImVec2 buttonSize(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());

            if (record)
                ImGui::BeginDisabled();

            // Output file
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Output file");
            ImGui::SameLine();
            static std::string filename = baseGUI->generateFilename("video", "");
            ImGui::InputText("##OutputFile", &filename);
            if (filename.empty())
                filename = baseGUI->generateFilename("video", "");
            ImGui::SameLine();
            ImGui::TextDisabled(".mp4");

            // Interval time
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Interval time");
            ImGui::SameLine();

            static float start_time = 0.;
            ImGui::LocalInputFloat("##StartTime", &start_time);

            ImGui::SameLine();
            ImGui::Text("-");
            ImGui::SameLine();

            static float end_time = std::numeric_limits<float>::infinity();
            ImGui::LocalInputFloat("##EndTime", &end_time);

            // Current time
            const auto& current_time = baseGUI->getRootNode()->getTime();

            if (record)
                ImGui::EndDisabled();

            ImVec4 red = ImVec4(1., 0.3, 0.3, 1.);
            ImGui::PushStyleColor(ImGuiCol_Button, red);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, sofaimgui::blendColor(red, ImVec4(0.5,0.,0.,1.), 0.1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, sofaimgui::blendColor(red, ImVec4(0.5,0.,0.,1.), 0.3));

            // ImGui::SetCursorPosX(rightPosition); // Set the position to the right of the area
            static bool clicked = false;
            if (ImGui::Button(record? ICON_FA_STOP " Stop": "⬤ Record"))
            {
                clicked = true;
                record = !record;
            }
            ImGui::SetItemTooltip(record? "Stop recording": "Start recording");

            if ((clicked && current_time >= start_time) || (record && current_time >= end_time))
            {
                clicked = false;
                baseGUI->setVideoFilename(filename + ".mp4");
                if(baseGUI->toggleVideoRecording())
                    showRecordingMessage(baseGUI);
                else
                    FooterStatusBar::getInstance().setTempMessage("Something went wrong with the video, check the Log Window", FooterStatusBar::MERROR);
                record = baseGUI->isVideoRecording();
            }

            ImGui::PopStyleColor(3);
        }
        ImGui::End();
    }
}

void RecordVideoWindow::showRecordingMessage(sofaglfw::SofaGLFWBaseGUI *baseGUI)
{
    bool recording = baseGUI->isVideoRecording();
    std::string message = recording? "Start": "Finished";
    message += " recording to:" + (recording? " " + baseGUI->getVideoFilePath() : "");
    FooterStatusBar::getInstance().setTempMessage(message, FooterStatusBar::MINFO, recording? "": baseGUI->getVideoFilePath());
}
} // namespace


