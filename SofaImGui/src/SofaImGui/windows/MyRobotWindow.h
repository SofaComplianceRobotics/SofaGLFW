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
#include <imgui.h>

namespace sofaimgui::windows {

class SOFAIMGUI_API MyRobotWindow : public BaseWindow
{
   public:
    MyRobotWindow(const std::string& name, const bool& isWindowOpen);
    ~MyRobotWindow() = default;

    void showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags) override;
    std::string getDescription() override;

    struct Connection{
        std::vector<std::string> ports;
        int portId;
        std::function<std::vector<std::string>()> listAvailablePortsCallback;
    };

	enum Section {
        NONE,           // Not displayed
		INFORMATION,    // Displayed in Information section
		SETTINGS,       // Displayed in Settings section
	};

    void clearWindow() override;
    void setAvailablePorts(const std::vector<std::string> &ports);
    std::string getSelectedPort();
    Connection& getConnection();
	sofaimgui::models::GUIData::SPtr addData(const std::string& label,
                                            const std::pair<sofa::core::BaseData*, bool>& data,
                                            const std::pair<sofa::core::BaseData*, bool>& min = std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                                            const std::pair<sofa::core::BaseData*, bool>& max = std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                                            const std::string& group = "",
                                            const std::string& tooltip = "",
                                            Section section = Section::NONE);
	void removeGUIData(sofaimgui::models::GUIData::SPtr guiData) override;


   protected:

    Connection m_connection;
	std::map<Section, std::unordered_set<sofaimgui::models::GUIData::SPtr>> m_sectionedGUIData;

    bool enabled() override;

    bool isInEmptyGroup(const std::string &group);
    bool showSliderDouble(const std::string &name, double* v, const double& min, const double& max, const int nbIndents);
};

}


