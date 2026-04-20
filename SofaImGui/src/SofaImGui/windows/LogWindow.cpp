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
#include <SofaImGui/ImGuiGUIEngine.h>
#include <SofaImGui/widgets/ImGuiDataWidget.h>
#include <SofaImGui/widgets/Widgets.h>

#include <iomanip>
#include <sofa/helper/logging/LoggingMessageHandler.h>
#include <sofa/core/loader/SceneLoader.h>
#include <sofa/simulation/SceneLoaderFactory.h>
#include <sofa/simulation/Simulation.h>
#include <sofa/helper/AdvancedTimer.h>
#include <imgui.h>
#include <nfd.h>
#include <IconsFontAwesome6.h>
#include <fstream>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/component/visual/LineAxis.h>
#include <sofa/gui/common/BaseGUI.h>
#include <sofa/simulation/graph/DAGNode.h>
#include <SofaImGui/windows/LogWindow.h>

namespace sofaimgui::windows {

LogWindow::LogWindow(const std::string& name, const bool& isWindowOpen)
{
    m_defaultIsOpen = false;
    m_name = name;
    m_isOpen = isWindowOpen;
}

std::string LogWindow::getDescription()
{
    return "Inspect the logs.";
}

void LogWindow::showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags)
{
    SOFA_UNUSED(baseGUI);

    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &m_isOpen, windowFlags))
        {
            unsigned int i {};
            const auto& messages = sofa::helper::logging::MainLoggingMessageHandler::getInstance().getMessages();
            const int digits = [&messages]()
            {
                int d = 0;
                auto s = messages.size();
                while (s != 0) { s /= 10; d++; }
                return d;
            }();

            static bool autoScroll = true;
            static bool showInfo = true;

            bool openOptions = false;
            if (ImGui::LocalButton(ICON_FA_BARS))
                openOptions = true;

            if (openOptions)
            {
                ImGui::OpenPopup("##LogSettings");
            }

            if (ImGui::BeginPopup("##LogSettings"))
            {
                ImGui::LocalCheckBox("Automatic scroll", &autoScroll);
                ImGui::LocalCheckBox("Show info", &showInfo);
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            if (ImGui::LocalButton(ICON_FA_FILE_EXPORT))
            {
                nfdchar_t *outPath;
                const nfdresult_t result = NFD_SaveDialog(&outPath, nullptr, 0, nullptr, "log.txt");
                if (result == NFD_OKAY)
                {
                    std::ofstream outputFile;
                    outputFile.open(outPath, std::ios::out);

                    if (outputFile.is_open())
                    {
                        for (const auto& message : messages)
                        {
                            static std::unordered_map<sofa::helper::logging::Message::Type, std::string> labelMap {
                                    {sofa::helper::logging::Message::Advice, "SUGGESTION"},
                                    {sofa::helper::logging::Message::Deprecated, "DEPRECATED"},
                                    {sofa::helper::logging::Message::Warning, "WARNING"},
                                    {sofa::helper::logging::Message::Info, "INFO"},
                                    {sofa::helper::logging::Message::Error, "ERROR"},
                                    {sofa::helper::logging::Message::Fatal, "FATAL"},
                                    {sofa::helper::logging::Message::TEmpty, "EMPTY"},
                            };
                            outputFile << "[" << labelMap[message.type()] << "]";
                            if (const auto* nfo = dynamic_cast<sofa::helper::logging::SofaComponentInfo*>(message.componentInfo().get()))
                            {
                                outputFile << " " << nfo->name();
                                if (nfo->m_component)
                                {
                                    outputFile << " (" << nfo->m_component->getPathName() << ")";
                                }
                            }
                            outputFile << " " << message.messageAsString() << std::endl;
                        }
                        outputFile.close();
                    } else
                    {
                        std::cout << "Failed to open the file " << outPath << std::endl;
                    }
                    NFD_FreePath(outPath);
                }
            }
            ImGui::SetItemTooltip("Export Logs");

            std::size_t nbRows = 0;
            if (ImGui::BeginTable("logTable", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY))
            {
                ImGui::TableSetupColumn("logId", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("message type", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("sender", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("message", ImGuiTableColumnFlags_WidthStretch);
                for (const auto& message : messages)
                {
                    if (!showInfo && message.type() == sofa::helper::logging::Message::Info)
                    {
                        continue;
                    }

                    ImGui::TableNextRow();
                    nbRows++;

                    ImGui::TableNextColumn();

                    std::stringstream ss;
                    ss << std::setfill('0') << std::setw(digits) << i++;
                    ImGui::TextDisabled("%s", ss.str().c_str());

                    ImGui::TableNextColumn();

                    constexpr auto writeMessageType = [](const sofa::helper::logging::Message::Type t)
                    {
                        switch (t)
                        {
                            case sofa::helper::logging::Message::Advice     : return ImGui::TextColored(ImColor(COLOR_DARK_GREY), "[SUGGESTION]");
                            case sofa::helper::logging::Message::Deprecated : return ImGui::TextColored(ImColor(COLOR_BLUE), "[DEPRECATED]");
                            case sofa::helper::logging::Message::Warning    : return ImGui::TextColored(ImColor(COLOR_ORANGE), "[WARNING]");
                            case sofa::helper::logging::Message::Info       : return ImGui::Text("[INFO]");
                            case sofa::helper::logging::Message::Error      : return ImGui::TextColored(ImColor(COLOR_RED), "[ERROR]");
                            case sofa::helper::logging::Message::Fatal      : return ImGui::TextColored(ImColor(COLOR_RED), "[FATAL]");
                            case sofa::helper::logging::Message::TEmpty     : return ImGui::Text("[EMPTY]");
                            default: return;
                        }
                    };
                    writeMessageType(message.type());

                    auto sender = message.sender();
                    auto* nfo = dynamic_cast<sofa::helper::logging::SofaComponentInfo*>(message.componentInfo().get());
                    if (nfo)
                    {
                        sender.append("(" + nfo->name() + ")");
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", sender.c_str());

                    if (nfo && ImGui::IsItemHovered() && nfo->m_component)
                    {
                        ImGui::SetTooltip("Path: %s", nfo->m_component->getPathName().c_str());
                    }

                    ImGui::TableNextColumn();
                    std::string msgStr = message.message().str();
                    int lineCount = 1;
                    lineCount += static_cast<int>(std::count(msgStr.begin(), msgStr.end(), '\n'));
                    float totalHeight = lineCount * ImGui::GetTextLineHeight();

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, COLOR_TRANSPARENT);
                    ImGui::InputTextMultiline(
                        ("##msg" + std::to_string(i)).c_str(),
                        const_cast<char*>(msgStr.c_str()), msgStr.size() + 1,
                        ImVec2(ImGui::GetContentRegionAvail().x, totalHeight),
                        ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_NoUndoRedo
                        );
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                }

                static std::size_t lastNbRows = 0;
                if (autoScroll && lastNbRows < nbRows)
                {
                    ImGui::SetScrollHereY(1.0f);
                }
                lastNbRows = nbRows;

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}

} // namespace
