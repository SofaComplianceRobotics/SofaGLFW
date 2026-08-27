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

#include "GUIColors.h"
#include <IconsFontAwesome6.h>
#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/FooterStatusBar.h>
#include <imgui_internal.h>

#include <sofa/helper/logging/Messaging.h>
#include <sofa/helper/system/FileSystem.h>
#include <sofa/version.h>

#include <filesystem>

namespace sofaimgui {

FooterStatusBar &FooterStatusBar::getInstance()
{
    static FooterStatusBar footerStatusBar;
    return footerStatusBar;
}

void FooterStatusBar::showFooterStatusBar()
{
    ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNavFocus;
    float height = ImGui::GetFrameHeight();

    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetColorU32(ImGuiCol_Header));
    if (ImGui::BeginViewportSideBar(m_name.c_str(), viewport, ImGuiDir_Down, height, window_flags))
    {
        if (ImGui::BeginMenuBar())
        {
            // Display the software version in the right of the status bar
            std::string version = "SOFA v" + std::string(SOFA_VERSION_STR);
            float length = ImGui::CalcTextSize(version.c_str()).x;
            float right = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - length - 2 * ImGui::GetStyle().ItemSpacing.x; // Set the position to the right of the bar
            ImGui::SetCursorPosX(right);
            ImGui::TextDisabled("%s", version.c_str());

            ImGui::EndMenuBar();
        }

        showLogStatus();

        ImGui::End();
    }
    ImGui::PopStyleColor();
}

void FooterStatusBar::showTempMessageOnStatusBar()
{
    static float infoRefreshTime = 0.;
    float messageLifeSpan = m_tempMessagePath.empty()? m_tempMessageLifeSpan: m_tempMessageLifeSpan*2.;

    if (ImGui::Begin(m_name.c_str()))
    {
        if (ImGui::BeginMenuBar())
        {
            showFilePopUpModal();

            if (!m_tempMessage.empty())
            {
                if (m_refreshTempMessage)
                {
                    infoRefreshTime = (float)ImGui::GetTime();
                    m_refreshTempMessage = false;
                }

                if((float)ImGui::GetTime() - infoRefreshTime > messageLifeSpan)
                {
                    m_tempMessage.clear();
                    m_tempMessagePath.clear();
                }
                else 
                {
                    float length = ImGui::CalcTextSize((m_tempMessage + m_tempMessagePath).c_str()).x;
                    float center = (ImGui::GetWindowWidth() - length) * 0.5f;
                    ImGui::SetCursorPosX(center); // Set the position to the middle of the status bar

                    std::string icon;
                    switch (m_tempMessageType) {
                    case MessageType::MWARNING:
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_ORANGE);
                        icon = ICON_FA_CIRCLE_EXCLAMATION;
                        break;
                    }
                    case MessageType::MERROR:
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_RED);
                        icon = ICON_FA_CIRCLE_EXCLAMATION;
                        break;
                    }
                    default:
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
                        icon = ICON_FA_CIRCLE_INFO;
                        break;
                    }
                    }
                    ImGui::Text("%s", (icon + " " + m_tempMessage).c_str());

                    showPath();

                    ImGui::PopStyleColor();
                }
            }

            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}

void FooterStatusBar::showPath()
{
    if (!m_tempMessagePath.empty())
    {
        m_fileToOpen = m_tempMessagePath;
        ImGui::SameLine(0., ImGui::GetStyle().FramePadding.x);

        if (sofa::helper::system::FileSystem::exists(m_tempMessagePath, true))
        {
            ImGui::TextLink(m_tempMessagePath.c_str());
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                if (sofa::helper::system::FileSystem::isFile(m_tempMessagePath, true) && std::filesystem::path(m_tempMessagePath).has_parent_path())
                {
                    ImGui::OpenPopup(m_fileToOpenPopUpLabel.c_str());
                }
                else
                    sofa::helper::system::FileSystem::openFileWithDefaultApplication(m_tempMessagePath);
            }
        }
        else if (m_tempMessagePath.starts_with("http"))
        {
            sofaimgui::widgets::TextLinkOpenURL(m_tempMessagePath.c_str(), m_tempMessagePath.c_str());
        }
        else
        {
            ImGui::Text("%s", m_tempMessagePath.c_str());
        }
    }
}

void FooterStatusBar::showFilePopUpModal()
{
    if (ImGui::BeginPopupModal(m_fileToOpenPopUpLabel.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::NewLine();
        ImGui::Text("Path: %s", m_fileToOpen.c_str());
        ImGui::NewLine();

        if (ImGui::Button("Open File"))
        {
            sofa::helper::system::FileSystem::openFileWithDefaultApplication(m_fileToOpen);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Open File Location"))
        {
            sofa::helper::system::FileSystem::openFileWithDefaultApplication(std::filesystem::path(m_fileToOpen).parent_path().string());
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

void FooterStatusBar::setTempMessage(const std::string &message, const MessageType& type, const std::string &path)
{
    m_refreshTempMessage = true;
    m_tempMessageType = type;
    m_tempMessage = message;
    m_tempMessagePath = path;
    std::string from = "GUI";

    auto getFullMessage = [this] {
        return m_tempMessage + m_tempMessagePath;
    };

    switch (type) {
    case MessageType::MWARNING:
    {
        msg_warning(from) << getFullMessage();
        break;
    }
    case MessageType::MERROR:
    {
        msg_error(from) << getFullMessage();
        break;
    }
    default:
    {
        msg_info(from) << getFullMessage();
        break;
    }
    }
}

void FooterStatusBar::showLogStatus()
{
    // update log status if logs have changed
    if (m_previousLogMessagesIndex < m_logMessages.size())
    {
        auto higherStatus = std::max_element(m_logMessages.begin() + m_previousLogMessagesIndex,
                                            m_logMessages.end(),
                                            [](const auto& m1, const auto& m2) {return m1.type() < m2.type();}); // get max priority messsage
        if(higherStatus->type() > m_highestLogStatus)
            m_highestLogStatus = higherStatus->type();
    }

    // show button if needed
    if (m_highestLogStatus >= sofa::helper::logging::Message::Type::Deprecated)
    {
        if (ImGui::Begin("##FooterStatusBar"))
        {
            if (ImGui::BeginMenuBar())
            {
                const char* icon;
                ImColor color;
                if (m_highestLogStatus >= sofa::helper::logging::Message::Type::Error)
                {
                    icon = ICON_FA_CIRCLE_EXCLAMATION;
                    color = ImColor(COLOR_RED);
                }
                else if (m_highestLogStatus >= sofa::helper::logging::Message::Type::Warning)
                {
                    icon = ICON_FA_TRIANGLE_EXCLAMATION;
                    color = ImColor(COLOR_ORANGE);
                }
                else 
                {
                    icon = ICON_FA_CIRCLE_INFO;
                    color = ImColor(COLOR_BLUE);
                }
                
                float left = ImGui::GetStyle().ItemSpacing.x; // Set the position to the right of the bar
                ImGui::SetCursorPosX(left);

                ImGui::PushStyleColor(ImGuiCol_Button, COLOR_TRANSPARENT);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_TRANSPARENT);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_TRANSPARENT);
                ImGui::PushStyleColor(ImGuiCol_ButtonText, color.Value);

                if (ImGui::Button(icon))
                {
                    m_logStatusCallback();
                }
                ImGui::SetItemTooltip("Open Log");

                ImGui::SameLine(0,0);
                ImGui::TextDisabled("Check Logs");

                ImGui::PopStyleColor(4);

                ImGui::EndMenuBar();
            }
            ImGui::End();
        }
    }

    m_previousLogMessagesIndex = m_logMessages.size();
}

void FooterStatusBar::setLogStatusCallback(std::function<void()> f)
{
    m_logStatusCallback = f;
}

void FooterStatusBar::clearLogStatus(const sofa::Index& index)
{
    m_previousLogMessagesIndex=index;
    m_highestLogStatus=sofa::helper::logging::Message::Info;
}

}
