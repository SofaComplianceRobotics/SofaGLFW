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
#pragma once

#include <SofaImGui/windows/BaseWindow.h>
#include <sofa/simulation/Node.h>
#include <SofaImGui/config.h>

#include <SofaGLFW/BaseGUIEngine.h>

#include <imgui.h>
#include <sofa/simulation/Node.h>
#include <SimpleIni.h>


namespace sofaimgui::windows
{
    class SOFAIMGUI_API LogWindow : public BaseWindow
    {
    public:
        LogWindow(const std::string& name);
        ~LogWindow() = default;

        std::string getDescription() override;

    protected:

        bool m_ws_autoScroll{true};
        bool m_ws_showInfo{true};

        sofa::Index m_firstMessageIndex{0};

        void internalShowWindow() override;
        void registerAndLoadWindowSettings() override;

        void showButtons();
        void showSettingsButton();
        void showExportButton();
        void showCopyLogButton();
        void showClearButton();

        void showLogs();

        void clearLogs();
        void addMessageContextMenu(const std::string &message);
        void messagesToStringStream(std::stringstream &output);

    private:
        const std::vector<sofa::helper::logging::Message>& m_messages;
    };
}
