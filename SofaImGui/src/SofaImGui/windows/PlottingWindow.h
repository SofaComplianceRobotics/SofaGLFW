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
#include <SofaImGui/Workbench.h>
#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

namespace sofaimgui::windows {


class SOFAIMGUI_API PlottingWindow : public BaseWindow
{
   public:

    static const int MAX_NB_PLOT {4};

    struct RollingBuffer
    {
        float span = 20.f;
        float xStart = 0.f;
        float ratio = 1.f;
        ImVector<ImVec2> data;
        RollingBuffer()
        {
            clear();
        }
        void addPoint(float x, float y)
        {
            float xmod = fmodf(x - xStart, span);
            if (!data.empty() && xmod < data.back().x - xStart)
            {
                xStart = data.front().x;
                data.erase(data.begin());
            }
            data.push_back(ImVec2(x, y * ratio));
        }
        void clear()
        {
            if (!data.empty())
                xStart = data.front().x;

            data.clear();
            data.reserve(2000);
        }
    };

    PlottingWindow(const std::string& name);
    ~PlottingWindow() = default;

    std::string getDescription() override;

    sofaimgui::models::guidata::GUIData::SPtr addData(const std::string& label,
                                                      const std::pair<sofa::core::BaseData*, bool>& data,
                                                      const int& subplotIndex = 0);

    models::guidata::GUIData::SPtr addGUIData(models::guidata::GUIData::SPtr guidata) override;

   protected:
    std::map<sofa::Index, std::set<sofaimgui::models::guidata::GUIData::SPtr>> m_data;
    std::map<sofaimgui::models::guidata::GUIData::SPtr, RollingBuffer> m_buffers;
    float m_ratio[MAX_NB_PLOT] = {1, 1, 1, 1};

    long m_ws_nbRows{1};
    long m_nbCols{1};

    void beforeShowWindow() override;
    void internalShowWindow() override;
    void registerAndLoadWindowSettings() override;

    void clearWindow() override;
    bool isEnabledByState() override {return !m_GUIData.empty();}

    void exportData();
    void setDataSubplot(models::guidata::GUIData::SPtr data, const int& subplotIndex);
    void showButtons();
    void showPlots();
    void showMenu();
    void showMenu(ImPlotPlot &plot, const sofa::Index &idSubplot);
};

}


