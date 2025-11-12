#pragma once

#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <map>

namespace ImGui
{

class MovePad
{
public:
    MovePad() = default;
    ~MovePad() = default;

    MovePad(const char* label, const char* labelPadH, const char* labelPadV, const char* labelSlider,
        double* valuePadH, double* valuePadV, double* valueSlider,
        const double* minPadH, const double* maxPadH,
        const double* minPadV, const double* maxPadV,
        const double* minSlider, const double* maxSlider);

    bool showPad(sofaglfw::SofaGLFWBaseGUI *baseGUI);
    void setBounds(const char* axis, const double& min, const double& max) { m_minValues[getMappedAxis(axis)] = min; m_maxValues[getMappedAxis(axis)] = max; }

protected:
    const char* m_label = "##MovePad";
    ImVec2 m_mousePosPad = ImVec2(-1.0f, -1.0f);
    ImRect m_grabBBPadH, m_grabBBPadV, m_grabBBSlider, m_grabPad;
    const char* m_axis[3] = { "X", "Y", "Z" };
    std::map<const char*, bool> m_flippedAxis = { {"PadH", false}, {"PadV", false}, {"Slider", false} };
    std::map<const char*, const char*> m_mappedAxis = { {"PadH", "X"}, {"PadV", "Y"}, {"Slider", "Z"} };
    std::map<const char*, double*> m_values = { {"PadH", nullptr}, {"PadV", nullptr}, {"Slider", nullptr} };
    std::map<const char*, double> m_minValues = { {"PadH", 0.0}, {"PadV", 0.0}, {"Slider", 0.0} };
    std::map<const char*, double> m_maxValues = { {"PadH", 0.0}, {"PadV", 0.0}, {"Slider", 0.0} };
    std::map<const char*, ImRect> m_grabBBs = { {"PadH", ImRect()}, {"PadV", ImRect()}, {"Slider", ImRect()}};

    bool show1DPadSlider(char const* label,
                         double* value, const double* min, const double* max,
                         const ImRect& bb, const ImRect& containerBB, ImRect& grabBB,
                         const ImGuiID& id, ImGuiWindow* window, ImGuiSliderFlags flags=ImGuiSliderFlags_None);
    void swapAxis(const char* axisLabel, int axisIndex);
    const char* getMappedAxis(const char* axis);
};

}
