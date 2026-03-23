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
#include <SofaImGui/widgets/Widgets.h>
#include <imgui_internal.h>
#include <sofa/helper/system/FileSystem.h>
#include <sofa/helper/logging/Messaging.h>
#include <SofaImGui/FooterStatusBar.h>
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
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();

    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetColorU32(ImGuiCol_Header));
    if (ImGui::BeginViewportSideBar("##FooterStatusBar", viewport, ImGuiDir_Down, height, window_flags))
    {
        if (ImGui::BeginMenuBar())
        {
            // Display the software version in the right of the status bar
            std::string version = "SOFA v" + std::string(SOFA_VERSION_STR);
            float length = ImGui::CalcTextSize(version.c_str()).x;
            float right = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - length - 2 * ImGui::GetStyle().ItemSpacing.x;
            ImGui::SetCursorPosX(right); // Set the position to the middle of the bar
            ImGui::TextDisabled("%s", version.c_str());

            ImGui::EndMenuBar();
        }
        ImGui::End();
    }
    ImGui::PopStyleColor();
}

void FooterStatusBar::showTempMessageOnStatusBar()
{
    static float infoRefreshTime = 0.;
    float messageLifeSpan = m_tempMessagePath.empty()? m_tempMessageLifeSpan: m_tempMessageLifeSpan*2.;

    if (ImGui::Begin("##FooterStatusBar"))
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
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.4f, 0.f, 1.f));
                        icon = ICON_FA_CIRCLE_EXCLAMATION;
                        break;
                    }
                    case MessageType::MERROR:
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.f, 0.f, 1.f));
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
            ImGui::LocalTextLinkOpenURL(m_tempMessagePath.c_str(), m_tempMessagePath.c_str());
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

}
