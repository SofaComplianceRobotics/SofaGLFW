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
#include <imgui.h>
#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/config.h>
#include <sofa/core/objectmodel/Data.h>

#include <unordered_map>

namespace sofaimgui::widgets
{

struct SOFAIMGUI_API BaseDataWidget
{
    virtual ~BaseDataWidget() = default;
    virtual void showWidget(sofa::core::objectmodel::BaseData&) = 0;

    static void showWidgetAsText(sofa::core::objectmodel::BaseData& data);
};

template<class T>
struct DataWidget : BaseDataWidget
{
    using MyData = sofa::core::objectmodel::Data<T>;

    static std::string getType()
    {
        static const std::string type = []()
        {
            MyData d;
            return d.getValueTypeString();
        }();
        return type;
    }

    void showWidget(sofa::core::objectmodel::BaseData& data) override
    {
        if (data.isReadOnly())
        {
            ImGui::PushItemFlag(ImGuiItemFlags_ReadOnly, true);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        }

        if (MyData* d = dynamic_cast<MyData*>(&data))
        {
            showWidget(*d);
        }
        else
        {
            const void* rawPtr = data.getValueVoidPtr();
            if (const T* castedPtr = static_cast<const T*>(rawPtr))
            {
                showWidget(data, castedPtr);
            }
            else
            {
                showWidgetAsText(data);
            }
        }

        if (data.isReadOnly())
        {
            ImGui::PopStyleColor();
            ImGui::PopItemFlag();
        }
    }

    void showWidget(MyData& data)
    {
        showWidgetAsText(data);
    }

    ~DataWidget() override = default;

protected:
    /**
     * This method is called when the Data cannot be dynamic_cast from a BaseData.
     * Instead, the BaseData is provided, as well as the object.
     */
    void showWidget(sofa::core::objectmodel::BaseData& data, const T* object)
    {
        SOFA_UNUSED(object);
        showWidgetAsText(data);
    }
};

struct SOFAIMGUI_API DataWidgetFactory
{
    template<class T>
    static bool Add()
    {
        using Widget = DataWidget<T>;
        const auto it = factoryMap.emplace(Widget::getType(), std::make_unique<Widget>());
        dmsg_error_when(!it.second, "DataWidgetFactory")<< "Cannot add widget " << Widget::getType() << " into the factory";
        return it.second;
    }

    static BaseDataWidget* GetWidget(sofa::core::objectmodel::BaseData& data)
    {
        const auto it = factoryMap.find(data.getValueTypeString());
        if (it != factoryMap.end())
            return it->second.get();
        return nullptr;
    }

private:
    inline static std::unordered_map<std::string, std::unique_ptr<BaseDataWidget> > factoryMap;
};

inline bool showSliderDouble(const std::string& label, double* v, const double& min, const double& max)
{
    bool hasValueChanged = false;
    const float inputWidth = ImGui::CalcTextSize("-100000,00").x + ImGui::GetFrameHeight() / 2 + ImGui::GetStyle().FramePadding.x;
    const float sliderWidth = ImGui::GetContentRegionAvail().x - inputWidth;

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
    ImGui::PushItemWidth(sliderWidth);
    if (ImGui::SliderScalar(("##SettingSlider" + label).c_str() , ImGuiDataType_Double, v, &min, &max, "%0.2f", ImGuiSliderFlags_NoInput))
        hasValueChanged=true;
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    const double step = max - min;

    if (sofaimgui::widgets::InputDouble(("##SettingInput" + label).c_str(), v, powf(10.0f, floorf(log10f(step * 0.01))), step * 0.1))
        hasValueChanged=true;

    return hasValueChanged;
}

inline void showWidget(sofa::core::objectmodel::BaseData& data,
                       const sofa::core::objectmodel::BaseData* min=nullptr,
                       const sofa::core::objectmodel::BaseData* max=nullptr)
{
    auto* widget = DataWidgetFactory::GetWidget(data);
    std::string tooltip = data.isReadOnly()? "(read only) ": "";
    tooltip += "data type: ";
    tooltip += data.getData()->getValueTypeString();

    ImGui::PushItemWidth(-1); // Fit container width
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    if (widget)
    {
        if (min == nullptr || max == nullptr)
        {
            widget->showWidget(data);
            ImGui::SetItemTooltip("%s", tooltip.c_str());
        }
        else
        {
            double d = data.getValueTypeInfo()->getScalarValue(data.getValueVoidPtr(), 0);
            const double dmin = min->getValueTypeInfo()->getScalarValue(min->getValueVoidPtr(), 0);
            const double dmax = max->getValueTypeInfo()->getScalarValue(max->getValueVoidPtr(), 0);
            if (showSliderDouble(("##"+data.getName()).c_str(), &d, dmin, dmax))
            {
                data.getValueTypeInfo()->setScalarValue(data.beginEditVoidPtr(), 0, d);
                data.endEditVoidPtr();
            }
        }
    }
    else
    {
        BaseDataWidget::showWidgetAsText(data);
        ImGui::SetItemTooltip("%s", tooltip.c_str());
    }
    ImGui::PopStyleVar();
    ImGui::PopItemWidth();

    if(ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("_DATAWIDGET", data.getData(), sizeof(data), 0, false);
        ImGui::Text("%s", data.m_name.c_str());
        ImGui::EndDragDropSource();
    }
}

}
