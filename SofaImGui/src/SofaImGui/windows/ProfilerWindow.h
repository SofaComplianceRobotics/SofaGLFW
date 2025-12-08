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

#include <sofa/helper/AdvancedTimer.h>
#include <unordered_set>
#include <SofaImGui/windows/BaseWindow.h>

namespace sofaimgui::windows
{

/**
 * @brief Profiler Window.
 *
 * Displays profiling information, including frame durations, timer percentages, and timer durations.
 */
class ProfilerWindow : public BaseWindow
{
   public:
    ProfilerWindow(){}
    ProfilerWindow(const std::string& name, const bool& isWindowOpen);
    ~ProfilerWindow()=default;

    void showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags) override;

protected:

    int m_timeWindowSize{150};
    int m_selectedFrame{0};
    float m_selectedFrameDuration{0.};

    SReal convertInMs(sofa::helper::system::thread::ctime_t t);
    void showChart(const std::deque<sofa::type::vector<sofa::helper::Record> > &allRecords, std::unordered_set<int>& selectedTimers);
    void showTable(const std::deque<sofa::type::vector<sofa::helper::Record> > &allRecords, std::unordered_set<int> &selectedTimers);
};

} // namespace
