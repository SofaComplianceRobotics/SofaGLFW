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

#include <sofa/type/vector.h>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include <IconsFontAwesome5.h>

#include <SofaImGui/windows/ProfilerWindow.h>
#include <SofaImGui/widgets/Widgets.h>

namespace sofaimgui::windows {

ProfilerWindow::ProfilerWindow(const std::string& name, const bool& isWindowOpen)
{
    m_workbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;

    m_name = name;
    m_isOpen = isWindowOpen;
}

void ProfilerWindow::showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI, const ImGuiWindowFlags &windowFlags)
{
    SOFA_UNUSED(baseGUI);

    sofa::helper::AdvancedTimer::setEnabled("Animate", m_isOpen);
    if (isEnabledInWorkbench() && isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &m_isOpen, windowFlags))
        {
            sofa::helper::AdvancedTimer::setInterval("Animate", 1);
            sofa::helper::AdvancedTimer::setOutputType("Animate", "gui");

            auto groot = baseGUI->getRootNode().get();
            if (groot->animate_.getValue())
                ImGui::BeginDisabled();

            static std::unordered_set<int> selectedTimers;
            static std::deque< sofa::type::vector<sofa::helper::Record> > allRecords;
            int maxTimeWindowSize = 5000;
            float inputWidth = ImGui::CalcTextSize(std::to_string(maxTimeWindowSize).c_str()).x * 5;

            // Chart
            if (ImGui::BeginChild("##Chart", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None))
            {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Tme Window Size:");
                ImGui::SameLine();
                ImGui::PushItemWidth(inputWidth);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.);
                ImGui::InputInt("##TimeWindowSizeInput", &m_timeWindowSize);
                ImGui::PopStyleVar();
                ImGui::PopItemWidth();
                m_timeWindowSize = std::clamp(m_timeWindowSize, 10, maxTimeWindowSize);
                m_selectedFrame = std::min(m_selectedFrame, m_timeWindowSize - 1);

                if (groot->animate_.getValue())
                {
                    sofa::type::vector<sofa::helper::Record> _records = sofa::helper::AdvancedTimer::getRecords("Animate");
                    allRecords.emplace_back(std::move(_records));

                    while (allRecords.size() >= size_t(m_timeWindowSize))
                        allRecords.pop_front();
                }

                showChart(allRecords, selectedTimers);
            }
            ImGui::EndChild();

            ImGui::SameLine();

            // Table
            if (ImGui::BeginChild("##Table", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None))
            {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Selected Frame:");
                ImGui::SameLine();
                ImGui::PushItemWidth(inputWidth);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.);
                ImGui::InputInt("##FrameInput", &m_selectedFrame);
                ImGui::PopStyleVar();
                ImGui::PopItemWidth();
                m_selectedFrame = std::clamp(m_selectedFrame, 0, int(allRecords.size()) - 1);
                ImGui::SameLine();
                ImGui::TextDisabled("(duration in ms: %0.2f)", m_selectedFrameDuration);

                if (ImGui::BeginChild("##SelectedFrameTable", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding))
                {
                    showTable(allRecords, selectedTimers);
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();

            if (groot->animate_.getValue())
                ImGui::EndDisabled();
        }
        ImGui::End();
    }
}

SReal ProfilerWindow::convertInMs(sofa::helper::system::thread::ctime_t t)
{
    static SReal timer_freqd = static_cast<SReal>(sofa::helper::system::thread::CTime::getTicksPerSec());
    return 1000.0 * static_cast<SReal>(t) / static_cast<SReal>(timer_freqd);
}

void ProfilerWindow::showChart(const std::deque< sofa::type::vector<sofa::helper::Record> >& allRecords,
                               std::unordered_set<int>& selectedTimers)
{
    struct Chart
    {
        std::string label;
        sofa::type::vector<float> values;
    };

    sofa::type::vector<float> frameChart;
    frameChart.reserve(allRecords.size());

    sofa::type::vector<Chart> charts;
    charts.reserve(selectedTimers.size());

    for (const auto& records : allRecords)
    {
        if (records.size() >= 2)
        {
            const auto tMin = records.front().time;
            const auto tMax = records.back().time;
            const auto frameDuration = convertInMs(tMax - tMin);
            frameChart.push_back(frameDuration);
        }
        else
        {
            frameChart.push_back(0.);
        }
    }

    for (const unsigned int timerId : selectedTimers)
    {
        Chart chart;
        for (const auto& records : allRecords)
        {
            float value = 0.f;
            sofa::helper::system::thread::ctime_t t0;
            for (const auto& rec : records)
            {
                if (timerId == rec.id)
                {
                    chart.label = rec.label;
                    if (rec.type == sofa::helper::Record::RBEGIN || rec.type == sofa::helper::Record::RSTEP_BEGIN || rec.type == sofa::helper::Record::RSTEP)
                    {
                        t0 = rec.time;
                    }
                    if (rec.type == sofa::helper::Record::REND || rec.type == sofa::helper::Record::RSTEP_END)
                    {
                        value += convertInMs(rec.time - t0);
                    }
                }
            }
            chart.values.push_back(value);
        }
        charts.push_back(chart);
    }

    if (ImPlot::BeginPlot("##ProfilerChart", ImVec2(-1, -1), ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus))
    {
        double selectedFrame = m_selectedFrame;
        auto plot = ImPlot::GetCurrentPlot();
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && plot->Hovered)
            selectedFrame = plot->XAxis(0).PixelsToPlot(ImGui::GetMousePos().x);

        static ImPlotAxisFlags xflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoHighlight;
        static ImPlotAxisFlags yflags = ImPlotAxisFlags_NoHighlight;
        ImPlot::SetupAxes("Time Step","Duration (ms)", xflags, yflags);
        ImPlot::SetupAxesLimits(0, m_timeWindowSize, 0, 10);
        ImPlot::DragLineX(0, &selectedFrame, IMPLOT_AUTO_COL);
        ImPlot::PlotLine("Total", frameChart.data(), frameChart.size());
        for (const auto& chart : charts)
        {
            ImPlot::PlotLine(chart.label.c_str(), chart.values.data(), chart.values.size());
        }
        m_selectedFrame = round(selectedFrame);
        ImPlot::EndPlot();
    }
}

void ProfilerWindow::showTable(const std::deque< sofa::type::vector<sofa::helper::Record> >& allRecords,
                               std::unordered_set<int>& selectedTimers)
{
    if (m_selectedFrame >= 0 && m_selectedFrame < int(allRecords.size()))
    {
        const auto records = allRecords[m_selectedFrame];
        if (!records.empty())
        {
            auto tStart = records.front().time;
            auto tEnd = tStart;
            std::unordered_map<unsigned int, SReal > duration;
            std::stack<sofa::helper::system::thread::ctime_t> durationStack;
            std::stack<unsigned int> timerIdStack;
            unsigned int timerIdCounter {};
            for (const auto& rec : allRecords[m_selectedFrame])
            {
                tStart = std::min(tStart, rec.time);
                tEnd = std::max(tEnd, rec.time);

                if (rec.type == sofa::helper::Record::RBEGIN || rec.type == sofa::helper::Record::RSTEP_BEGIN || rec.type == sofa::helper::Record::RSTEP)
                {
                    durationStack.push(rec.time);
                    timerIdStack.push(timerIdCounter++);
                }
                if (rec.type == sofa::helper::Record::REND || rec.type == sofa::helper::Record::RSTEP_END)
                {
                    const auto t = durationStack.top();
                    durationStack.pop();
                    duration[timerIdStack.top()] = convertInMs(rec.time - t);
                    timerIdStack.pop();
                }
            }

            ImGui::SameLine();
            m_selectedFrameDuration = convertInMs(tEnd - tStart);

            static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
                                           ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody;
            if (ImGui::BeginTable("profilerTable", 3, flags))
            {
                std::stack<bool> openStack;

                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Percent (%)", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("A").x * 12.0f);
                ImGui::TableSetupColumn("Duration (ms)", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("A").x * 12.0f);
                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                ImGui::TableHeadersRow();

                int node_clicked = -1;
                static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
                timerIdCounter = 0;
                for (auto it = records.begin(); it != records.end(); ++it)
                {
                    const auto& rec = *it;
                    if (rec.type == sofa::helper::Record::RBEGIN || rec.type == sofa::helper::Record::RSTEP_BEGIN || rec.type == sofa::helper::Record::RSTEP)
                    {
                        ImGui::PushID(timerIdCounter);
                        if (openStack.empty() || openStack.top())
                        {
                            ImGuiTreeNodeFlags node_flags = base_flags;
                            if (selectedTimers.find(rec.id) != selectedTimers.end())
                            {
                                node_flags |= ImGuiTreeNodeFlags_Selected;
                            }

                            if (it + 1 != records.end())
                            {
                                const auto& nextRec = *(it + 1);
                                if (it->label == nextRec.label && (nextRec.type == sofa::helper::Record::REND || nextRec.type == sofa::helper::Record::RSTEP_END))
                                {
                                    node_flags |= ImGuiTreeNodeFlags_Leaf;
                                }
                            }

                            ImGui::TableNextRow();

                            ImGui::TableNextColumn();
                            const bool isOpen = ImGui::TreeNodeEx(rec.label.c_str(), node_flags);
                            openStack.push(isOpen);

                            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                                node_clicked = rec.id;

                            const auto& d = (rec.label == "Animate") ? m_selectedFrameDuration : duration[timerIdCounter];

                            ImVec4 color;
                            color.w = 1.f;
                            const auto ratio = (rec.label == "Animate") ? 1. : d/m_selectedFrameDuration;
                            constexpr auto clamp = [](double d){ return std::max(0., std::min(1., d));};
                            ImGui::ColorConvertHSVtoRGB(120./360. * clamp(1.-ratio*10.), 0.72f, 0.72f, color.x,color.y, color.z);
                            ImGui::TableNextColumn();
                            ImGui::TextColored(color, "%.2f", 100 * ratio);

                            ImGui::TableNextColumn();
                            ImGui::TextColored(color, "%.2f", d);
                        }
                        else
                        {
                            openStack.push(false);
                        }
                        ImGui::PopID();
                        ++timerIdCounter;
                    }
                    if (rec.type == sofa::helper::Record::REND || rec.type == sofa::helper::Record::RSTEP_END)
                    {
                        if (openStack.top())
                        {
                            ImGui::TreePop();
                        }
                        openStack.pop();
                    }
                }
                while(!openStack.empty())
                {
                    if (openStack.top())
                    {
                        ImGui::TreePop();
                    }
                    openStack.pop();
                }

                ImGui::EndTable();

                if (node_clicked != -1)
                {
                    auto it = selectedTimers.find(node_clicked);
                    if (it == selectedTimers.end())
                    {
                        selectedTimers.insert(node_clicked);
                    }
                    else
                    {
                        selectedTimers.erase(it);
                    }
                }
            }
        }
        return;
    }

    displayDisabledInfoMessage("No records to display for the selected frame.");
}
} // namespace
