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
#include <SofaImGui/windows/LogWindow.h>
#include <SofaImGui/windows/WindowsSettingsName.h>
#include <SofaImGui/ImGuiGUIEngine.h>
#include <SofaImGui/widgets/DataWidget.h>
#include <SofaImGui/widgets/Widgets.h>

#include <iomanip>
#include <sofa/helper/logging/LoggingMessageHandler.h>
#include <sofa/core/loader/SceneLoader.h>
#include <sofa/simulation/SceneLoaderFactory.h>
#include <sofa/simulation/Simulation.h>
#include <sofa/helper/AdvancedTimer.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/component/visual/LineAxis.h>
#include <sofa/gui/common/BaseGUI.h>
#include <sofa/simulation/Node.h>

#include <imgui.h>
#include <nfd.h>
#include <IconsFontAwesome6.h>
#include <fstream>

namespace sofaimgui::windows {

LogWindow::LogWindow(const std::string& name)
    : BaseWindow(name)
    , m_messages(sofa::helper::logging::MainLoggingMessageHandler::getInstance().getMessages())
{
    FooterStatusBar::getInstance().setLogStatusCallback(
        [this]() {
            this->setOpen(true);
            ImGui::SetWindowFocus(this->getLabel().c_str());
        }
    );
}

std::string LogWindow::getDescription()
{
    return "Inspect the logs.";
}

void LogWindow::registerAndLoadWindowSettings()
{
    registerAndLoadWindowSetting(WS_LOG_AUTOSCROLL, m_ws_autoScroll, WindowsSettings::BOOL);
    registerAndLoadWindowSetting(WS_LOG_SHOWINFO, m_ws_showInfo, WindowsSettings::BOOL);
}

void LogWindow::internalShowWindow()
{
    showButtons();
    showLogs();
}

void LogWindow::showButtons()
{
    showSettingsButton();

    ImGui::SameLine();

    showExportButton();

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    showCopyLogButton();

    ImGui::SameLine();

    showClearButton();
}

void LogWindow::showSettingsButton()
{
    if (sofaimgui::widgets::Button(ICON_FA_BARS))
        ImGui::OpenPopup("##LogSettings");

    if (ImGui::BeginPopup("##LogSettings"))
    {
        sofaimgui::widgets::CheckBox("Automatic scroll", &m_ws_autoScroll);
        sofaimgui::widgets::CheckBox("Show info", &m_ws_showInfo);
        ImGui::EndPopup();
    }
}

void LogWindow::showExportButton()
{
    if (sofaimgui::widgets::Button(ICON_FA_FILE_EXPORT))
    {
        nfdchar_t *outPath;
        const nfdresult_t result = NFD_SaveDialog(&outPath, nullptr, 0, nullptr, "log.txt");
        if (result == NFD_OKAY)
        {
            std::ofstream outputFile;
            outputFile.open(outPath, std::ios::out);

            if (outputFile.is_open())
            {
                std::stringstream s;
                messagesToStringStream(s);
                outputFile << s.rdbuf();
                outputFile.close();

            } else
            {
                std::cout << "Failed to open the file " << outPath << std::endl;
            }
            NFD_FreePath(outPath);
        }
    }
    ImGui::SetItemTooltip("Export Logs");
}

void LogWindow::showCopyLogButton()
{
    if (sofaimgui::widgets::Button(ICON_FA_COPY))
    {
        std::stringstream s;
        messagesToStringStream(s);
        ImGui::SetClipboardText(s.str().c_str());
    }

    ImGui::SetItemTooltip("Copy Logs");
}

void LogWindow::showClearButton()
{
    if (sofaimgui::widgets::Button(ICON_FA_BROOM))
        clearLogs();
    ImGui::SetItemTooltip("Clear Logs");
}

void LogWindow::showLogs()
{
    unsigned int i {};
    const int digits = [this]()
    {
        int d = 0;
        auto s = this->m_messages.size();
        while (s != 0) { s /= 10; d++; }
        return d;
    }();

    std::size_t nbRows = 0;
    if (ImGui::BeginTable("logTable", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY))
    {
        static std::string messageToCopy;
        bool openMessageContextMenu = false;

        ImGui::TableSetupColumn("logId", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("message type", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("sender", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("message", ImGuiTableColumnFlags_WidthStretch);
        for (sofa::Index index = m_firstMessageIndex; index<m_messages.size(); index++)
        {
            const auto& message = m_messages[index];
            if (!m_ws_showInfo && message.type() == sofa::helper::logging::Message::Info)
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
                case sofa::helper::logging::Message::Advice     : return ImGui::TextColored(ImColor(COLOR_DARK_GREEN), "[SUGGESTION]");
                case sofa::helper::logging::Message::Deprecated : return ImGui::TextColored(ImColor(COLOR_BLUE), "[DEPRECATED]");
                case sofa::helper::logging::Message::Warning    : return ImGui::TextColored(ImColor(COLOR_ORANGE), "[WARNING]");
                case sofa::helper::logging::Message::Info       : return ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "[INFO]");
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
            ImGui::TextDisabled("[%s]", sender.c_str());

            if (nfo && ImGui::IsItemHovered() && nfo->m_component)
            {
                ImGui::SetTooltip("Path: %s", nfo->m_component->getPathName().c_str());
            }

            ImGui::TableNextColumn();
            std::string msgStr = message.message().str();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, COLOR_TRANSPARENT);
            ImGui::TextWrapped("%s", msgStr.c_str());
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            {
                openMessageContextMenu = true;
                messageToCopy = msgStr;
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }

        static std::size_t lastNbRows = 0;
        if (m_ws_autoScroll && lastNbRows < nbRows)
        {
            ImGui::SetScrollHereY(1.0f);
        }
        lastNbRows = nbRows;

        ImGui::EndTable();

        if (openMessageContextMenu)
        {
            ImGui::OpenPopup("##MessageContextMenu");
        }

        if (ImGui::BeginPopup("##MessageContextMenu"))
        {
            addMessageContextMenu(messageToCopy);
            ImGui::EndPopup();
        }
    }
}

void LogWindow::clearLogs()
{
    m_firstMessageIndex = m_messages.size();
    FooterStatusBar::getInstance().clearLogStatus(m_firstMessageIndex);
}

void LogWindow::addMessageContextMenu(const std::string& message)
{
    bool disable = m_messages.empty();
    if (disable)
        ImGui::BeginDisabled();

    if (ImGui::MenuItem("Copy line"))
        ImGui::SetClipboardText(message.c_str());

    if (disable)
        ImGui::EndDisabled();
}

void LogWindow::messagesToStringStream(std::stringstream& output)
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

    for (const auto& message : m_messages)
    {
        output << "[" << labelMap[message.type()] << "] [" << message.sender() << "] ";
        output << message.messageAsString() << std::endl;
    }
}

} // namespace
