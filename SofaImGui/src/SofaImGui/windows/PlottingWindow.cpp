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
#include <sofa/type/Quat.h>

#include <SofaImGui/windows/PlottingWindow.h>

#include <imgui_internal.h>
#include <IconsFontAwesome6.h>

#include <iostream>
#include <fstream>
#include <SofaImGui/widgets/Widgets.h>
#include <nfd.h>

namespace sofaimgui::windows {

PlottingWindow::PlottingWindow(const std::string& name,
                               const bool& isWindowOpen)
{
    m_workbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;

    m_defaultIsOpen = true;
    m_name = name;
    m_isOpen = isWindowOpen;

    for (size_t i = 0; i < MAX_NB_PLOT; i++)
        m_data[i] = std::set<sofaimgui::models::guidata::GUIData::SPtr>();
}

std::string PlottingWindow::getDescription()
{
    return "Plot data over time.";
}

void PlottingWindow::clearWindow()
{
    m_data.clear();
    m_buffers.clear();
}

void PlottingWindow::exportData()
{
    nfdchar_t *outPath;
    size_t nbData = m_GUIData.size();

    const nfdresult_t result = NFD_SaveDialog(&outPath, nullptr, 0, nullptr, "plotting.csv");
    if (result == NFD_OKAY)
    {
        if (nbData)
        {
            std::ofstream outputFile;
            outputFile.open(outPath, std::ios::out);

            if (outputFile.is_open())
            {
                outputFile << "time,";
                for (const auto& d : m_buffers[0].data)
                    outputFile << d.x << ",";
                outputFile << "\n";

				size_t i = 0;
				for (auto& it : m_GUIData)
				{
                    outputFile << it->label<< ",";
                    auto buffer = m_buffers[i];
                    for (const auto& d : buffer.data)
                        outputFile << d.y << ",";
                    outputFile << "\n";
                    i++;
                }
                outputFile.close();
            }
        }
    }
}

sofaimgui::models::guidata::GUIData::SPtr PlottingWindow::addData(const std::string& label,
                                                                const std::pair<sofa::core::BaseData*, bool>& data,
                                                                const std::pair<sofa::core::BaseData*, bool>& min,
                                                                const std::pair<sofa::core::BaseData*, bool>& max,
                                                                const std::string& group,
                                                                const std::string& help)
{
    auto newData = BaseWindow::addData(label, data, min, max, group, help);
	m_data[0].insert(newData);
    return newData;;
}

void PlottingWindow::showWindow(const ImGuiWindowFlags &windowFlags)
{
    SOFA_UNUSED(windowFlags);
    auto groot = m_baseGUI->getRootNode().get();

    size_t nbData = m_GUIData.size();
    if (m_buffers.size() != nbData)
        m_buffers.resize(nbData);

    if(!m_GUIData.empty() && groot->getAnimate())
    {
        for (size_t k=0; k<nbData; k++)
        {
			auto& data = *std::next(m_GUIData.begin(), k);
            const sofa::defaulttype::AbstractTypeInfo* typeInfo = data->getData()->getValueTypeInfo();
            float value = typeInfo->getScalarValue(data->getData()->getValueVoidPtr(), 0);
            float time = groot->getTime();
            RollingBuffer& buffer = m_buffers[k];
            buffer.addPoint(time, value);
        }
    }
    
    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &m_isOpen, ImGuiWindowFlags_NoScrollbar))
        {
            if (!isEnabledInWorkbench() || !enabled())
                showInfoMessage("This window is used to plot data over time. It currently has no data registered or is disabled in the active workbench.");

            if (!isEnabledInWorkbench())
                ImGui::BeginDisabled();

            showButtons();
            showPlots();

            if (!isEnabledInWorkbench())
                ImGui::EndDisabled();
        }
        ImGui::End();
    }
}

void PlottingWindow::showButtons()
{
    auto positionRight = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - ImGui::GetFrameHeight() * 3 - ImGui::GetStyle().ItemSpacing.y * 4; // Get position for right buttons
    auto positionMiddle = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x / 2.f; // Get position for middle button

    // Clear button
    if (ImGui::Button("Clear"))
    {
        for(auto& buffer: m_buffers)
            buffer.clear();
    }

    ImGui::SameLine();

    // Export csv button
    if (!enabled())
        ImGui::BeginDisabled();

    if (ImGui::LocalButton(ICON_FA_FILE_EXPORT))
    {
        exportData();
    }

    if (!enabled())
    {
        ImGui::SetItemTooltip("No values to export");
        ImGui::EndDisabled();
    }
    else
    {
        ImGui::SetItemTooltip("Export data");
    }

    ImGui::SameLine();

    char text[] = "Time interval 20 s";
    ImGui::SetCursorPosX(positionMiddle - ImGui::CalcTextSize(text).x / 2.); // Set position to the middle of the header

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", text);

    ImGui::SameLine();
    ImGui::SetCursorPosX(positionRight); // Set position to right of the header

    if(ImGui::LocalButton("+##plotting"))
    {
        if (m_nbRows<MAX_NB_PLOT)
            m_nbRows+=1;
    }
    ImGui::SetItemTooltip("Show an additional subplot.");

    ImGui::SameLine();

    if (ImGui::LocalButton("-##plotting"))
    {
        if (m_nbRows>1)
            m_nbRows-=1;
    }
    ImGui::SetItemTooltip("Hide last subplot.");

    ImGui::SameLine();

    bool openOptions = false;
    if (ImGui::LocalButton(ICON_FA_BARS))
        openOptions = true;

    if (openOptions)
    {
        ImGui::OpenPopup("##MyPlotsContext");
    }

    if (ImGui::BeginPopup("##MyPlotsContext"))
    {
        showMenu();
        ImGui::EndPopup();
    }
}

void PlottingWindow::showPlots()
{
    static sofaimgui::models::guidata::GUIData::SPtr dragedData;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, COLOR_TRANSPARENT);

    bool portraitLayout = (ImGui::GetWindowWidth() * 0.75 < ImGui::GetWindowHeight());
    if (ImPlot::BeginSubplots("##myplots",
                              portraitLayout? m_nbRows: m_nbCols,
                              portraitLayout? m_nbCols: m_nbRows,
                              ImVec2(-1, -1),
                              ImPlotSubplotFlags_ShareItems
                              ))
    {
        for (auto& plots : m_data)
        {
            if (ImPlot::BeginPlot(("##" +std::to_string(plots.first)).c_str(), ImVec2(-1, 0),
                                  ImPlotFlags_NoMouseText | ImPlotFlags_NoMenus))
            {
                ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_Sort | ImPlotLegendFlags_Outside);
                ImPlot::SetupAxes("Time (s)", nullptr,
                                  ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoSideSwitch | ImPlotAxisFlags_NoHighlight,
                                  ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoSideSwitch | ImPlotAxisFlags_NoHighlight);

				size_t k = 0;
                for (auto& data: plots.second)
                {
                    RollingBuffer& buffer = m_buffers[k];

                    ImPlotSpec spec;
                    spec.Stride = 2 * sizeof(float);
                    ImPlot::PlotLine(data->label.c_str(),
                                     &buffer.data[0].x,
                                     &buffer.data[0].y,
                                     buffer.data.size(),
                                     spec);

                    if (ImPlot::BeginDragDropSourceItem(data->label.c_str())) {
                        dragedData = data;
                        ImGui::SetDragDropPayload("dragndrop", nullptr, 0);
                        ImPlot::ItemIcon(ImPlot::GetLastItemColor());
                        ImGui::SameLine();
                        ImGui::TextUnformatted(data->label.c_str());
                        ImPlot::EndDragDropSource();
                    }
                    k++;
                }

                if (ImPlot::BeginDragDropTargetPlot())
                {
                    if (ImGui::AcceptDragDropPayload("dragndrop"))
                    {
                        if (dragedData)
                        {
                            for (auto& subplots : m_data)
                            {
								if (subplots.second.contains(dragedData))
								{
									subplots.second.erase(dragedData);
									break;
								}
                            }
							m_data[plots.first].insert(dragedData);
                        }
                    }
                    ImPlot::EndDragDropTarget();
                }

                ImPlotContext& gp = *GImPlot;
                ImPlotPlot &plot  = *gp.CurrentPlot;

                ImPlot::EndPlot();

                ImGui::PushOverrideID(plot.ID);

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
                    !plot.Items.Legend.Hovered &&
                    plot.Hovered)
                {
                    ImGui::OpenPopup("##MyPlotContext");
                }

                if (ImGui::BeginPopup("##MyPlotContext"))
                {
                    ImGui::PopStyleColor();
                    showMenu(plot, plots.first);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, COLOR_TRANSPARENT);
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }
        }

        ImPlot::EndSubplots();
    }
    ImGui::PopStyleColor();
}

void PlottingWindow::showMenu()
{
    if (ImGui::BeginTable("Columns", 2, ImGuiTableFlags_None))
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Y axis ratio");
        ImGui::TableNextColumn();
        ImGui::SameLine();

        float ratio = m_ratio[0];
        ImGui::PushItemWidth(ImGui::CalcTextSize("-100000,00").x);
        if (ImGui::InputFloat("##Ratio", &ratio, 0, 0, "%0.2e"))
        {
            size_t nbData = m_buffers.size();
            for (size_t i=0; i<nbData; i++)
            {
                auto& buffer = m_buffers[i];
                for (auto& point: buffer.data)
                {
                    point.y /= buffer.ratio;
                    point.y *= ratio;
                }
                buffer.ratio = ratio;
            }

            for (size_t i=0; i<m_nbRows * m_nbCols; i++)
				m_ratio[i] = ratio;
        }
        ImGui::PopItemWidth();
        ImGui::EndTable();
    }

    ImGui::Separator();

    ImPlotContext& gp = *GImPlot;
    auto& plots  = gp.Plots;

    bool showMousePosition = !ImHasFlag(plots.GetByIndex(0)->Flags, ImPlotFlags_NoMouseText);
    ImGui::LocalCheckBox("Show mouse position", &showMousePosition);
    bool showGrid = !ImHasFlag(plots.GetByIndex(0)->XAxis(0).Flags, ImPlotAxisFlags_NoGridLines);
    ImGui::LocalCheckBox("Show grid", &showGrid);
    bool autofit = ImHasFlag(plots.GetByIndex(0)->XAxis(0).Flags, ImPlotAxisFlags_AutoFit);
    ImGui::LocalCheckBox("Auto fit content", &autofit);

    for (size_t i=0; i<m_nbRows * m_nbCols; i++)
    {
        auto plot = plots.GetByIndex(i);
        showMousePosition ? plot->Flags &= ~ImPlotFlags_NoMouseText : plot->Flags |= ImPlotFlags_NoMouseText;
        showGrid ? plot->XAxis(0).Flags &= ~ImPlotAxisFlags_NoGridLines : plot->XAxis(0).Flags |= ImPlotAxisFlags_NoGridLines;
        showGrid ? plot->YAxis(0).Flags &= ~ImPlotAxisFlags_NoGridLines : plot->YAxis(0).Flags |= ImPlotAxisFlags_NoGridLines;
        !autofit ? plot->XAxis(0).Flags &= ~ImPlotAxisFlags_AutoFit : plot->XAxis(0).Flags |= ImPlotAxisFlags_AutoFit;
        !autofit ? plot->YAxis(0).Flags &= ~ImPlotAxisFlags_AutoFit : plot->YAxis(0).Flags |= ImPlotAxisFlags_AutoFit;
    }
}

void PlottingWindow::showMenu(ImPlotPlot &plot, const size_t &idSubplot)
{
    ImGui::PushID(plot.ID);
    if (ImGui::BeginTable("Columns", 2, ImGuiTableFlags_None))
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Y axis ratio");
        ImGui::TableNextColumn();
        ImGui::SameLine();

        float ratio = m_ratio[idSubplot];
        ImGui::PushItemWidth(ImGui::CalcTextSize("-100000,00").x);
        if (ImGui::InputFloat(("##Ratio" + std::to_string(idSubplot)).c_str(), &ratio, 0, 0, "%0.2e"))
        {
            size_t nbData = m_GUIData.size();
            for (size_t i=0; i<nbData; i++)
            {
				auto& data = *std::next(m_GUIData.begin(), i);
                auto& buffer = m_buffers[i];
                if (m_data[idSubplot].contains(data))
                {
                    for (auto& point: buffer.data)
                    {
                        point.y /= buffer.ratio;
                        point.y *= ratio;
                    }
                    buffer.ratio = ratio;
                }
            }
            m_ratio[idSubplot] = ratio;
        }
        ImGui::PopItemWidth();
        ImGui::EndTable();
    }

    ImGui::Separator();

    bool showMousePosition = !ImHasFlag(plot.Flags, ImPlotFlags_NoMouseText);
    if (ImGui::LocalCheckBox("Show mouse position", &showMousePosition))
        ImFlipFlag(plot.Flags, ImPlotFlags_NoMouseText);

    bool showGrid = !ImHasFlag(plot.XAxis(0).Flags, ImPlotAxisFlags_NoGridLines);
    if (ImGui::LocalCheckBox("Show grid", &showGrid))
    {
        ImFlipFlag(plot.XAxis(0).Flags, ImPlotAxisFlags_NoGridLines);
        ImFlipFlag(plot.YAxis(0).Flags, ImPlotAxisFlags_NoGridLines);
    }

    bool autofit = ImHasFlag(plot.XAxis(0).Flags, ImPlotAxisFlags_AutoFit);
    if (ImGui::LocalCheckBox("Auto fit content", &autofit))
    {
        ImFlipFlag(plot.XAxis(0).Flags, ImPlotAxisFlags_AutoFit);
        ImFlipFlag(plot.YAxis(0).Flags, ImPlotAxisFlags_AutoFit);
    }

    ImGui::PopID();
}

}

