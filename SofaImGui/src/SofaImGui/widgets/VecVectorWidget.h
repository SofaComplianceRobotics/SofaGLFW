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
#include "GUIColors.h"
#include "IconsFontAwesome6.h"
#include <sofa/core/objectmodel/Data.h>
#include <SofaImGui/widgets/ScalarWidget.h>
#include <imgui.h>
#include <string>


namespace sofaimgui::widgets
{

using namespace sofa;

static const ImGuiTableFlags tableflags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_NoBordersInBody;
static const int maxNbRowsDisplayed = 11;
static const int maxNbIndicesLoaded = 1000;

void tableSetupCorner(core::objectmodel::BaseData* data)
{
    if (data)
        ImGui::TableSetupColumn(ICON_FA_GRIP_VERTICAL, ImGuiTableColumnFlags_WidthFixed);
    else
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
}

void showTableHeadersRow(core::objectmodel::BaseData* data)
{
    ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    const int columnsCount = ImGui::TableGetColumnCount();
    for (int columnIndex = 0; columnIndex < columnsCount; columnIndex++)
    {
        if (!ImGui::TableSetColumnIndex(columnIndex))
            continue;

        std::string axisLabel = ImGui::TableGetColumnName(columnIndex);
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
        if (strcmp(axisLabel.c_str(), "X")==0 || strcmp(axisLabel.c_str(), "rX")==0 || strcmp(axisLabel.c_str(), "qX")==0)
            color = COLOR_RED;
        else if (strcmp(axisLabel.c_str(), "Y")==0 || strcmp(axisLabel.c_str(), "rY")==0 || strcmp(axisLabel.c_str(), "qY")==0)
            color = COLOR_GREEN;
        else if (strcmp(axisLabel.c_str(), "Z")==0 || strcmp(axisLabel.c_str(), "rZ")==0 || strcmp(axisLabel.c_str(), "qZ")==0)
            color = COLOR_BLUE;

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TableHeader(ImGui::TableGetColumnName(columnIndex));
        ImGui::PopStyleColor();

        if (columnIndex == 0)
        {
            if(data && ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("_DATAWIDGET", data, sizeof(data), 0, false);
                ImGui::Text("%s", data->getName().c_str());
                ImGui::EndDragDropSource();
            }
        }
    }
}

void showStartEnd(sofa::core::objectmodel::BaseData* data, const int&dataSize, int& start, int& end)
{
    if (data->isReadOnly())
    {
        ImGui::PushItemFlag(ImGuiItemFlags_ReadOnly, false);
        ImGui::PopStyleColor();
    }

    auto sizeStr = std::to_string(dataSize);
    int inputWidth = ImGui::CalcTextSize(sizeStr.c_str()).x + ImGui::GetFrameHeightWithSpacing() * 2;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Start-End");

    ImGui::SameLine();

    { // Input start
        ImGui::PushItemWidth(inputWidth);
        ImGui::InputInt(("##startRowIndex"+data->getPathName()).c_str(), &start);
        start = std::min(std::max(start, 0), dataSize - 1);
        ImGui::PopItemWidth();
    }

    ImGui::SameLine();
    ImGui::Text("-");
    ImGui::SameLine();

    { // Input end
        ImGui::PushItemWidth(inputWidth);
        ImGui::InputInt(("##endRowIndex"+data->getPathName()).c_str(), &end);
        end = std::min(std::max(end, start + 1), dataSize);
        ImGui::PopItemWidth();
    }

    ImGui::SameLine();
    ImGui::TextDisabled("%s", (" / " + sizeStr).c_str());

    if (data->isReadOnly())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        ImGui::PopItemFlag();
    }
}

void showLoadMore(sofa::core::objectmodel::BaseData* data, const int& dataSize, int& end)
{
    if (end < dataSize)
    {
        if (ImGui::Button(("Show more##"+data->getPathName()).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
            end += maxNbIndicesLoaded;
    }
}


/***********************************************************************************************************************
 * Vec
 **********************************************************************************************************************/


template< sofa::Size N, typename ValueType>
void setupVecTableHeader(Data<sofa::type::Vec<N, ValueType> >& data)
{
    tableSetupCorner(data.getData());
    for (unsigned int i = 0; i < N; ++i)
    {
        ImGui::TableSetupColumn(std::to_string(i).c_str());
    }
}

template<typename ValueType>
void setupVecTableHeader(Data<sofa::type::Vec<1, ValueType> >&)
{
    ImGui::TableSetupColumn("X");
}

template<typename ValueType>
void setupVecTableHeader(Data<sofa::type::Vec<2, ValueType> >&)
{
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
}

template<typename ValueType>
void setupVecTableHeader(Data<sofa::type::Vec<3, ValueType> >&)
{
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
    ImGui::TableSetupColumn("Z");
}

template< sofa::Size N, typename ValueType>
void showWidgetT(Data<sofa::type::Vec<N, ValueType> >& data)
{
    if (ImGui::BeginTable((data.getName() + (data.getOwner() ? data.getOwner()->getPathName() : "")).c_str(), N, tableflags))
    {
        setupVecTableHeader(data);
        showTableHeadersRow(data.getData());

        ImGui::TableNextRow();
        int i=0;
        for (auto& v : *sofa::helper::getWriteAccessor(data))
        {
            ImGui::TableNextColumn();
            ImGui::PushID(i);
            ImGui::PushItemWidth(-1); // Fit container width
            showScalarWidget(ImGui::TableGetColumnName(i++), v);
            ImGui::PopItemWidth();
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}


/***********************************************************************************************************************
 * Vectors of Vec
 **********************************************************************************************************************/


template< Size N, typename ValueType>
void setupVecTableHeader(Data<type::vector<type::Vec<N, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    for (unsigned int i = 0; i < N; ++i)
    {
        ImGui::TableSetupColumn(std::to_string(i).c_str());
    }
}

template<typename ValueType>
void setupVecTableHeader(Data<type::vector<ValueType> >& data)
{
    tableSetupCorner(data.getData());
    ImGui::TableSetupColumn("Value");
}

template<typename ValueType>
void setupVecTableHeader(Data<type::vector<type::Vec<1, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    ImGui::TableSetupColumn("X");
}

template<typename ValueType>
void setupVecTableHeader(Data<type::vector<type::Vec<2, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
}

template<typename ValueType>
void setupVecTableHeader(Data<type::vector<type::Vec<3, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
    ImGui::TableSetupColumn("Z");
}

template<Size N, typename ValueType>
bool showLine(unsigned int lineNumber, const std::string& tableLabel, type::Vec<N, ValueType>& vec)
{
    int i=0;
    ImGui::PushID(lineNumber);
    for (auto& v : vec)
    {
        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-1); // Fit container width
        showScalarWidget(tableLabel + ImGui::TableGetColumnName(i++) + std::to_string(v), v);
        ImGui::PopItemWidth();
    }
    ImGui::PopID();
    return false;
}

template<typename ValueType>
bool showLine(unsigned int lineNumber, const std::string& tableLabel, ValueType& value)
{
    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-1); // Fit container width
    bool result = showScalarWidget(tableLabel + std::to_string(lineNumber), value);
    ImGui::PopItemWidth();
    return result;
}

template<class T>
void showVectorWidget(Data<T>& data)
{
    const auto nbColumns = data.getValueTypeInfo()->size() + 1;
    const auto tableLabel = data.getName() + (data.getOwner() ? data.getOwner()->getPathName() : "");

    auto accessor = helper::getWriteAccessor(data);
    int dataSize = accessor->size();
    static bool loadMore = false;

    if (dataSize > 0)
    {
        static int start{0};
        static int end{std::min(dataSize, maxNbIndicesLoaded)};
        ImVec2 outerSize = ImVec2(0.0f, (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.y) * std::fmin(end - start + 0.75, maxNbRowsDisplayed));

        if (dataSize > maxNbRowsDisplayed)
            showStartEnd(data.getData(), dataSize, start, end);

        if (ImGui::BeginTable(tableLabel.c_str(), nbColumns, tableflags | ImGuiTableFlags_ScrollY, outerSize))
        {
            ImGui::TableSetColumnWidth(0, ImGui::CalcTextSize(std::to_string(end).c_str()).x);
            setupVecTableHeader(data);
            showTableHeadersRow(data.getData());

            bool anyChange = false;
            for (int i = start; i < end; ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%zu", i);
                auto& vec = accessor[i];
                if (showLine(i, tableLabel, vec))
                {
                    anyChange = true;
                    data.setDirtyValue();
                }
            }
            if (anyChange)
            {
                data.updateIfDirty();
            }

            loadMore = (ImGui::GetScrollY() == ImGui::GetScrollMaxY());
            ImGui::EndTable();
        }

        if (dataSize > maxNbRowsDisplayed && loadMore)
            showLoadMore(data.getData(), dataSize, end);
    }
}


/***********************************************************************************************************************
 * Vectors of Rigid
 **********************************************************************************************************************/


template< Size N, typename ValueType>
void setupVecTableHeader(Data<type::vector<defaulttype::RigidCoord<N, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    for (unsigned int i = 0; i < defaulttype::RigidCoord<N, ValueType>::total_size; ++i)
    {
        ImGui::TableSetupColumn(std::to_string(i).c_str());
    }
}

template<typename ValueType>
void setupVecTableHeader(Data<type::vector<defaulttype::RigidCoord<3, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
    ImGui::TableSetupColumn("Z");

    ImGui::TableSetupColumn("qX");
    ImGui::TableSetupColumn("qY");
    ImGui::TableSetupColumn("qZ");
    ImGui::TableSetupColumn("qW");
}

template<typename ValueType>
void setupVecTableHeader(Data<type::vector<defaulttype::RigidCoord<2, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");

    ImGui::TableSetupColumn("w");
}

template< Size N, typename ValueType>
void setupVecTableHeader(Data<type::vector<defaulttype::RigidDeriv<N, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    for (unsigned int i = 0; i < defaulttype::RigidDeriv<N, ValueType>::total_size; ++i)
    {
        ImGui::TableSetupColumn(std::to_string(i).c_str());
    }
}

template<typename ValueType>
void setupVecTableHeader(Data<type::vector<defaulttype::RigidDeriv<3, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
    ImGui::TableSetupColumn("Z");

    ImGui::TableSetupColumn("rX");
    ImGui::TableSetupColumn("rY");
    ImGui::TableSetupColumn("rZ");
}

template<typename ValueType>
void setupVecTableHeader(Data<type::vector<defaulttype::RigidDeriv<2, ValueType> > >& data)
{
    tableSetupCorner(data.getData());
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");

    ImGui::TableSetupColumn("w");
}

template< Size N, typename ValueType>
void showRigidLine(defaulttype::RigidCoord<N, ValueType>& vec, const unsigned int& counter)
{
    int j=0;
    for (auto& v : vec.getCenter())
    {
        ImGui::TableNextColumn();
        ImGui::PushID(j++);
        ImGui::PushItemWidth(-1); // Fit container width
        showScalarWidget("pos" + std::to_string(counter), v);
        ImGui::PopItemWidth();
        ImGui::PopID();
    }
    if constexpr (std::is_scalar_v<std::decay_t<decltype(vec.getOrientation())> >)
    {
        ImGui::TableNextColumn();
        ImGui::Text("%f", vec.getOrientation());
    }
    else
    {
        for (unsigned int i = 0 ; i < 4; ++i)
        {
            ImGui::TableNextColumn();
            auto& v = vec.getOrientation()[i];
            ImGui::PushID(i);
            ImGui::PushItemWidth(-1); // Fit container width
            showScalarWidget("orien" + std::to_string(counter), v);
            ImGui::PopItemWidth();
            ImGui::PopID();
        }
    }
}


template< Size N, typename ValueType>
void showRigidLine(defaulttype::RigidDeriv<N, ValueType>& vec, const unsigned int& counter)
{
    int j=0;
    for (auto& v : vec.getVCenter())
    {
        ImGui::TableNextColumn();
        ImGui::PushID(j++);
        ImGui::PushItemWidth(-1); // Fit container width
        showScalarWidget("pos" + std::to_string(counter), v);
        ImGui::PopItemWidth();
        ImGui::PopID();
    }
    if constexpr (std::is_scalar_v<std::decay_t<decltype(vec.getVOrientation())> >)
    {
        ImGui::TableNextColumn();
        ImGui::Text("%f", vec.getVOrientation());
    }
    else
    {
        for (unsigned int i = 0 ; i < 3; ++i)
        {
            ImGui::TableNextColumn();
            auto& v = vec.getVOrientation()[i];
            ImGui::PushID(i);
            ImGui::PushItemWidth(-1); // Fit container width
            showScalarWidget("orien" + std::to_string(counter), v);
            ImGui::PopItemWidth();
            ImGui::PopID();
        }
    }
}

template<typename ValueType>
void showWidgetT(Data<type::vector<ValueType> >& data)
{
    auto accessor = helper::getWriteAccessor(data);
    int dataSize = accessor->size();
    static bool loadMore = false;

    if (dataSize > 0)
    {
        static int start{0};
        static int end{std::min(dataSize, maxNbIndicesLoaded)};
        ImVec2 outerSize = ImVec2(0.0f, (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.y) * std::fmin(end - start + 0.75, maxNbRowsDisplayed));

        if (dataSize > maxNbRowsDisplayed)
            showStartEnd(data.getData(), dataSize, start, end);

        if (ImGui::BeginTable((data.getName() + (data.getOwner() ? data.getOwner()->getPathName() : "")).c_str(),
                              ValueType::total_size + 1,
                              tableflags | ImGuiTableFlags_ScrollY,
                              outerSize))
        {
            ImGui::TableSetColumnWidth(0, ImGui::CalcTextSize(std::to_string(end).c_str()).x);
            setupVecTableHeader(data);
            showTableHeadersRow(data.getData());

            auto& vecs = *sofa::helper::getWriteAccessor(data);
            for (int index=start; index<end; index++)
            {
                auto& vec = vecs[index];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%d", index);
                showRigidLine(vec, index);
            }

            loadMore = (ImGui::GetScrollY() == ImGui::GetScrollMaxY());
            ImGui::EndTable();
        }

        if (dataSize > maxNbRowsDisplayed && loadMore)
            showLoadMore(data.getData(), dataSize, end);
    }
}

/***********************************************************************************************************************
 * Topology elements
 **********************************************************************************************************************/

template< typename GeometryElement>
void showWidgetT(Data<sofa::type::vector<sofa::topology::Element<GeometryElement> > >& data)
{
    auto accessor = helper::getWriteAccessor(data);
    int dataSize = accessor->size();
    static bool loadMore = false;

    if (dataSize > 0)
    {
        static int start{0};
        static int end{std::min(dataSize, maxNbIndicesLoaded)};
        ImVec2 outerSize = ImVec2(0.0f, (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.y) * std::fmin(end - start + 0.75, maxNbRowsDisplayed));

        if (dataSize > maxNbRowsDisplayed)
            showStartEnd(data.getData(), dataSize, start, end);

        constexpr auto N = sofa::topology::Element<GeometryElement>::static_size;
        if (ImGui::BeginTable((data.getName() + (data.getOwner() ? data.getOwner()->getPathName() : "")).c_str(),
                              N + 1,
                              tableflags | ImGuiTableFlags_ScrollY,
                              outerSize))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetColumnWidth(0, ImGui::CalcTextSize(std::to_string(end).c_str()).x);
            for (unsigned int i = 0; i < N; ++i)
                ImGui::TableSetupColumn(std::to_string(i).c_str());

            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
            ImGui::TableHeadersRow();

            auto& vecs = *sofa::helper::getWriteAccessor(data);
            for (int index=start; index<end; index++)
            {
                auto& vec = vecs[index];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%d", index);
                unsigned int i=0;
                for (auto& v : vec)
                {
                    int vui = v;
                    ImGui::TableNextColumn();
                    ImGui::PushID(index);
                    ImGui::PushItemWidth(-1); // Fit container width
                    ImGui::BeginDisabled();
                    ImGui::InputInt((std::string("##") + ImGui::TableGetColumnName(i++) + std::to_string(index)).c_str(), &vui, 0, 0, ImGuiInputTextFlags_None);
                    ImGui::EndDisabled();
                    ImGui::PopItemWidth();
                    ImGui::PopID();
                }
            }

            loadMore = (ImGui::GetScrollY() == ImGui::GetScrollMaxY());
            ImGui::EndTable();
        }

        if (dataSize > maxNbRowsDisplayed && loadMore)
            showLoadMore(data.getData(), dataSize, end);
    }
}

}

