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
#include <sofa/core/objectmodel/Data.h>
#include <SofaImGui/widgets/ScalarWidget.h>
#include <imgui.h>
#include <string>


namespace sofaimgui {

using namespace sofa;

template< sofa::Size N, typename ValueType>
void showVecTableHeader(Data<sofa::type::vector<sofa::type::Vec<N, ValueType> > >&)
{
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    for (unsigned int i = 0; i < N; ++i)
    {
        ImGui::TableSetupColumn(std::to_string(i).c_str());
    }
}

template<typename ValueType>
void showVecTableHeader(Data<type::vector<ValueType> >&)
{
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Value");
}

template<typename ValueType>
void showVecTableHeader(Data<type::vector<type::Vec<1, ValueType> > >&)
{
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("X");
}

template<typename ValueType>
void showVecTableHeader(Data<sofa::type::vector<sofa::type::Vec<2, ValueType> > >&)
{
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
}

template<typename ValueType>
void showVecTableHeader(Data<sofa::type::vector<sofa::type::Vec<3, ValueType> > >&)
{
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
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
        showScalarWidget("", tableLabel + ImGui::TableGetColumnName(i++) + std::to_string(v), v);
    }
    ImGui::PopID();
    return false;
}

template<typename ValueType>
bool showLine(unsigned int lineNumber, const std::string& tableLabel, ValueType& value)
{
    ImGui::TableNextColumn();
    return showScalarWidget("", tableLabel + std::to_string(lineNumber), value);
}

template<class T>
void showVectorWidget(Data<T>& data)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_RowBg;
    const auto nbColumns = data.getValueTypeInfo()->size() + 1;
    const auto tableLabel = data.getName() + data.getOwner()->getPathName();

    ImVec2 innerWidth = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 10);
    if (ImGui::BeginTable(tableLabel.c_str(), nbColumns, flags, innerWidth))
    {
        showVecTableHeader(data);

        ImGui::TableHeadersRow();

        auto accessor = helper::getWriteAccessor(data);
        bool anyChange = false;
        for (std::size_t i = 0; i < accessor.size(); ++i)
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

        ImGui::EndTable();
    }
}


template< sofa::Size N, typename ValueType>
void showVecTableHeader(Data<sofa::type::vector<sofa::defaulttype::RigidCoord<N, ValueType> > >&)
{
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    for (unsigned int i = 0; i < sofa::defaulttype::RigidCoord<N, ValueType>::total_size; ++i)
    {
        ImGui::TableSetupColumn(std::to_string(i).c_str());
    }
}

template<typename ValueType>
void showVecTableHeader(Data<sofa::type::vector<sofa::defaulttype::RigidCoord<3, ValueType> > >&)
{
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
    ImGui::TableSetupColumn("Z");

    ImGui::TableSetupColumn("qX");
    ImGui::TableSetupColumn("qY");
    ImGui::TableSetupColumn("qZ");
    ImGui::TableSetupColumn("qW");
}

template<typename ValueType>
void showVecTableHeader(Data<sofa::type::vector<sofa::defaulttype::RigidCoord<2, ValueType> > >&)
{
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");

    ImGui::TableSetupColumn("w");
}

template< sofa::Size N, typename ValueType>
void showWidgetT(Data<sofa::type::vector<sofa::defaulttype::RigidCoord<N, ValueType> > >& data)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_RowBg;

    ImVec2 innerWidth = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 10);
    if (ImGui::BeginTable((data.getName() + data.getOwner()->getPathName()).c_str(), sofa::defaulttype::RigidCoord<N, ValueType>::total_size + 1, flags, innerWidth))
    {
        showVecTableHeader(data);

        ImGui::TableHeadersRow();

        unsigned int counter {};
        for (auto& vec : *sofa::helper::getWriteAccessor(data))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%d", counter++);
            int i=0;
            for (auto& v : vec.getCenter())
            {
                ImGui::TableNextColumn();
                ImGui::PushID(i++);
                showScalarWidget("", "pos" + std::to_string(counter), v);
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
                    showScalarWidget("", "orien" + std::to_string(counter), v);
                    ImGui::PopID();
                }
            }
        }

        ImGui::EndTable();
    }
}

}

