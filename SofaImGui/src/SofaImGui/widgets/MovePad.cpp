#define IMGUI_DEFINE_MATH_OPERATORS // import math operators

#include <SofaImGui/widgets/Buttons.h>
#include <SofaImGui/widgets/MovePad.h>

#include <IconsFontAwesome6.h>
#include <string>

namespace ImGui
{

MovePad::MovePad(const char* label, const char* labelPadH, const char* labelPadV, const char* labelSlider,
    double* valuePadH, double* valuePadV, double* valueSlider,
    const double* minPadH, const double* maxPadH,
    const double* minPadV, const double* maxPadV,
    const double* minSlider, const double* maxSlider)
{

    m_label = label;
    m_mappedAxis["PadH"] = labelPadH;
    m_mappedAxis["PadV"] = labelPadV;
    m_mappedAxis["Slider"] = labelSlider;

    m_minValues["PadH"] = *minPadH;
    m_minValues["PadV"] = *minPadV;
    m_minValues["Slider"] = *minSlider;

    m_maxValues["PadH"] = *maxPadH;
    m_maxValues["PadV"] = *maxPadV;
    m_maxValues["Slider"] = *maxSlider;

    m_values["PadH"] = valuePadH;
    m_values["PadV"] = valuePadV;
    m_values["Slider"] = valueSlider;
}


/**
* This widget is composed of a 2D pad and a vertical slider for the remaining dimension
**/
bool MovePad::showPad(sofaglfw::SofaGLFWBaseGUI* baseGUI)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID idPad = window->GetID(m_label);
    ImGuiID idPadH = window->GetID(idPad);
    ImGuiID idPadV = window->GetID(idPadH);
    ImGuiID idSlider = window->GetID(idPadV);
    const auto& wpos = ImGui::GetMainViewport()->Pos;

    // TODO: Move those to style
    float grabThickness = style.ScrollbarSize / 5.f;
    float grabRadius = grabThickness * 2.5f;
    double dragPadHThickness = grabThickness;
    double dragPadVThickness = grabThickness;
    double borderThickness = 2.0f;
    double lineThickness = 2.0f;
    double fCursorOff = 16.0f;
    const auto slidersRegionWidth = GetFrameHeight() * 8;

    const ImRect totalBB(window->DC.CursorPos, ImVec2(window->WorkRect.Max.x,
                                                      window->DC.CursorPos.y + window->WorkRect.GetWidth()- slidersRegionWidth));

    const ImVec2 containerSize = ImVec2(totalBB.GetWidth(), totalBB.GetHeight() - GetFrameHeight()*2.);
    const ImRect frameBB(totalBB.GetCenter() - ImVec2(containerSize.x/2.0, containerSize.y/2.0 - style.FramePadding.y ),
                         totalBB.GetCenter() + ImVec2(containerSize.x/2.0, containerSize.y/2.0));

    int padSize = frameBB.GetWidth() - slidersRegionWidth;
    padSize = std::min(frameBB.GetHeight(), std::max((float)padSize, frameBB.GetWidth() - slidersRegionWidth));
    auto padwidth = padSize + slidersRegionWidth + GetFrameHeight();
    const ImRect framePadBB(frameBB.GetCenter() - ImVec2(padwidth/2. - GetFrameHeight(), padSize/2.),
                            frameBB.GetCenter() - ImVec2(padwidth/2. - GetFrameHeight(), padSize/2.) + ImVec2(padSize, padSize));

    const ImRect framePadHBB(ImVec2(framePadBB.Min.x, framePadBB.Max.y + style.FramePadding.y * 2),
                             ImVec2(framePadBB.Max.x, framePadBB.Max.y + style.FramePadding.y * 2 + dragPadHThickness));
    const ImRect framePadVBB(ImVec2(framePadBB.Max.x + style.FramePadding.x * 2, framePadBB.Min.y),
                             ImVec2(framePadBB.Max.x + style.FramePadding.x * 2 + dragPadVThickness, framePadBB.Max.y));
    const ImRect frameSliderBB(ImVec2(framePadVBB.Max.x + slidersRegionWidth/2., framePadBB.Min.y),
                               ImVec2(framePadVBB.Max.x + slidersRegionWidth/2. + dragPadVThickness, framePadBB.Max.y));

    double fXLimit = fCursorOff / framePadBB.GetWidth();
    double fYLimit = fCursorOff / framePadBB.GetHeight();

    ImGui::ItemSize(totalBB, style.FramePadding.y);
    if (!ImGui::ItemAdd(totalBB, idPad, &frameBB, 0))
        return false;

    { // Show sliders
        const ImVec2 buttonSize = ImVec2(GetFrameHeight(), GetFrameHeight());

        PushStyleColor(ImGuiCol_ButtonText, GetColorU32(ImGuiCol_Text));
        { // PadH
            window->DC.CursorPos = (ImVec2(framePadHBB.Min.x - GetFrameHeight() - style.FramePadding.x , framePadHBB.GetCenter().y - GetFrameHeight()/2));
            PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            if (Button(ICON_FA_ARROWS_LEFT_RIGHT"##PadH", buttonSize))
            {
                m_flippedAxis["PadH"] = !m_flippedAxis["PadH"];
            }
            ImGui::PopStyleColor();
            show1DPadSlider(m_mappedAxis["PadH"], m_values["PadH"],
                            (m_flippedAxis["PadH"])?&m_maxValues["PadH"]:& m_minValues["PadH"],
                            (m_flippedAxis["PadH"])?&m_minValues["PadH"]:& m_maxValues["PadH"],
                            framePadHBB, totalBB, m_grabBBPadH, idPadH, window);
        }

        { // PadV
            window->DC.CursorPos = (ImVec2(framePadVBB.GetCenter().x - GetFrameHeight()/2, framePadVBB.Min.y - GetFrameHeight() - style.FramePadding.y));
            PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            if (Button(ICON_FA_ARROWS_UP_DOWN"##PadV", buttonSize))
            {
                m_flippedAxis["PadV"] = !m_flippedAxis["PadV"];
            }
            ImGui::PopStyleColor();
            show1DPadSlider(m_mappedAxis["PadV"], m_values["PadV"],
                            (m_flippedAxis["PadV"]) ? &m_maxValues["PadV"] : &m_minValues["PadV"],
                            (m_flippedAxis["PadV"]) ? &m_minValues["PadV"] : &m_maxValues["PadV"],
                            framePadVBB, totalBB, m_grabBBPadV, idPadV, window, ImGuiSliderFlags_Vertical);
        }

        { // Slider
            window->DC.CursorPos = (ImVec2(frameSliderBB.GetCenter().x - GetFrameHeight()/2, frameSliderBB.Min.y - GetFrameHeight() - style.FramePadding.y));
            PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            if (Button(ICON_FA_ARROWS_UP_DOWN"##Slider", buttonSize))
            {
                m_flippedAxis["Slider"] = !m_flippedAxis["Slider"];
            }
            ImGui::PopStyleColor();
            show1DPadSlider(m_mappedAxis["Slider"], m_values["Slider"],
                            (m_flippedAxis["Slider"]) ? &m_maxValues["Slider"] : &m_minValues["Slider"],
                            (m_flippedAxis["Slider"]) ? &m_minValues["Slider"] : &m_maxValues["Slider"],
                            frameSliderBB, totalBB, m_grabBBSlider, idSlider, window, ImGuiSliderFlags_Vertical);
        }
        ImGui::PopStyleColor();
    }

    bool padHovered = ImGui::ItemHoverable(framePadBB, idPad, g.LastItemData.ItemFlags);

    bool padPressed = padHovered && ImGui::IsMouseDown(0, idPad);
    bool makeActive = (padPressed || g.NavActivateId == idPad);
    if (makeActive && padPressed)
        ImGui::SetKeyOwner(ImGuiKey_MouseLeft, idPad);

    if (makeActive)
    {
        ImGui::SetActiveID(idPad, window);
        ImGui::SetFocusID(idPad, window);
        ImGui::FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    // Draw frame
    ImU32 frameColor = ImGui::GetColorU32(g.ActiveId == idPad ? ImGuiCol_FrameBgHovered : padHovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImGui::RenderNavCursor(framePadBB, idPad);
    ImGui::RenderFrame(framePadBB.Min, framePadBB.Max, frameColor, true, g.Style.FrameRounding);

    // Slider behavior
    bool valuePadHChanged = false;
    bool valuePadVChanged = false;
    bool valueSliderChanged = false;

    if (padHovered && ImGui::IsKeyPressed(ImGuiKey_LeftCtrl, false))
    {
        // With setMousePos, the parameter value is relative to the window top left corner
        m_mousePosPad = ImGui::GetIO().MousePos; // save mouse position when pressing ctrl
        const auto &center = m_grabBBSlider.GetCenter();
        baseGUI->setMousePos(center.x - wpos.x, center.y - wpos.y); // Needed for Wayland
        ImGui::TeleportMousePos(center);
    }
    else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        valueSliderChanged = ImGui::SliderBehavior(framePadBB, idPad, ImGuiDataType_Double, m_values["Slider"],
                                                    (m_flippedAxis["Slider"]) ? &m_maxValues["Slider"] : &m_minValues["Slider"],
                                                    (m_flippedAxis["Slider"]) ? &m_minValues["Slider"] : &m_maxValues["Slider"],
                                                   NULL, ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_Vertical, &m_grabBBSlider);
    }
    else
    {
        if (m_mousePosPad.x > 0)
        {
            const auto &center = m_grabPad.GetCenter();
            baseGUI->setMousePos(center.x - wpos.x, center.y - wpos.y); // Needed for Wayland
            ImGui::TeleportMousePos(center);
            m_mousePosPad = ImVec2(-1, -1);
        }

        valuePadHChanged = ImGui::SliderBehavior(framePadBB, idPad, ImGuiDataType_Double, m_values["PadH"],
                                                (m_flippedAxis["PadH"]) ? &m_maxValues["PadH"] : &m_minValues["PadH"],
                                                (m_flippedAxis["PadH"]) ? &m_minValues["PadH"] : &m_maxValues["PadH"],
                                                 NULL, ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat, &m_grabPad);
        ImRect tempGrab(m_grabPad);
        valuePadVChanged = ImGui::SliderBehavior(framePadBB, idPad, ImGuiDataType_Double, m_values["PadV"],
                                                (m_flippedAxis["PadV"]) ? &m_maxValues["PadV"] : &m_minValues["PadV"],
                                                (m_flippedAxis["PadV"]) ? &m_minValues["PadV"] : &m_maxValues["PadV"],
                                                 NULL, ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_Vertical, &m_grabPad);


        if (padHovered)
        {
            ImGui::SetKeyOwner(ImGuiKey_MouseWheelY, idPad);
            *m_values["Slider"] += ImGui::GetIO().MouseWheel;
            *m_values["Slider"] = std::max(m_minValues["Slider"], std::min(*m_values["Slider"], m_maxValues["Slider"]));
            valueSliderChanged = true;
        }

        m_grabPad.Min.x = tempGrab.Min.x;
        m_grabPad.Max.x = tempGrab.Max.x;
        if (valuePadHChanged || valuePadVChanged)
            ImGui::MarkItemEdited(idPad);
    }

    ImDrawList* pDrawList = window->DrawList;

    { // Draw Pad
        ImU32 padBorderColor = ImGui::GetColorU32(ImGuiCol_FrameBgActive);
        ImU32 padInColor = ImGui::GetColorU32(ImGuiCol_HeaderHovered);
        float deltaPadH = m_maxValues["PadH"] - m_minValues["PadH"];
        float deltaPadV = m_maxValues["PadV"] - m_minValues["PadV"];
        float fScaleX = (*m_values["PadH"] - m_minValues["PadH"]) / deltaPadH;
        float fScaleY = 1.0f - ((*m_values["PadV"] - m_minValues["PadV"]) / deltaPadV);
        ImVec2 vCursorPos(m_grabBBPadH.GetCenter().x, m_grabBBPadV.GetCenter().y);

        // Cursor
        window->DrawList->AddCircleFilled(vCursorPos, grabRadius * 1.2f, GetColorU32(g.ActiveId == idPad ? ImGuiCol_SliderGrabActive : ImGuiCol_Button));
        window->DrawList->AddCircleFilled(vCursorPos, grabRadius, GetColorU32(ImGuiCol_SliderGrab));

        // Vertical Line
        if (fScaleY > 2.0f * fYLimit)
            pDrawList->AddLine(ImVec2(vCursorPos.x, framePadBB.Min.y + fCursorOff), ImVec2(vCursorPos.x, vCursorPos.y - fCursorOff), padInColor, lineThickness);
        if (fScaleY < 1.0f - 2.0f * fYLimit)
            pDrawList->AddLine(ImVec2(vCursorPos.x, framePadBB.Max.y - fCursorOff), ImVec2(vCursorPos.x, vCursorPos.y + fCursorOff), padInColor, lineThickness);

        // Horizontal Line
        if (fScaleX > 2.0f * fXLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Min.x + fCursorOff, vCursorPos.y), ImVec2(vCursorPos.x - fCursorOff, vCursorPos.y), padInColor, lineThickness);
        if (fScaleX < 1.0f - 2.0f * fYLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Max.x - fCursorOff, vCursorPos.y), ImVec2(vCursorPos.x + fCursorOff, vCursorPos.y), padInColor, lineThickness);

        // Borders::Right
        pDrawList->AddCircleFilled(ImVec2(framePadBB.Max.x, vCursorPos.y), 2.0f, padInColor, 3);
        // Handle Right::Y
        if (fScaleY > fYLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Max.x, framePadBB.Min.y), ImVec2(framePadBB.Max.x, vCursorPos.y - fCursorOff), padBorderColor, borderThickness);
        if (fScaleY < 1.0f - fYLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Max.x, framePadBB.Max.y), ImVec2(framePadBB.Max.x, vCursorPos.y + fCursorOff), padBorderColor, borderThickness);
        // Borders::Top
        pDrawList->AddCircleFilled(ImVec2(vCursorPos.x, framePadBB.Min.y), 2.0f, padInColor, 3);
        if (fScaleX > fXLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Min.x, framePadBB.Min.y), ImVec2(vCursorPos.x - fCursorOff, framePadBB.Min.y), padBorderColor, borderThickness);
        if (fScaleX < 1.0f - fXLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Max.x, framePadBB.Min.y), ImVec2(vCursorPos.x + fCursorOff, framePadBB.Min.y), padBorderColor, borderThickness);
        // Borders::Left
        pDrawList->AddCircleFilled(ImVec2(framePadBB.Min.x, vCursorPos.y), 2.0f, padInColor, 3);
        if (fScaleY > fYLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Min.x, framePadBB.Min.y), ImVec2(framePadBB.Min.x, vCursorPos.y - fCursorOff), padBorderColor, borderThickness);
        if (fScaleY < 1.0f - fYLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Min.x, framePadBB.Max.y), ImVec2(framePadBB.Min.x, vCursorPos.y + fCursorOff), padBorderColor, borderThickness);
        // Borders::Bottom
        pDrawList->AddCircleFilled(ImVec2(vCursorPos.x, framePadBB.Max.y), 2.0f, padInColor, 3);
        // Handle Bottom::X
        if (fScaleX > fXLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Min.x, framePadBB.Max.y), ImVec2(vCursorPos.x - fCursorOff, framePadBB.Max.y), padBorderColor, borderThickness);
        if (fScaleX < 1.0f - fXLimit)
            pDrawList->AddLine(ImVec2(framePadBB.Max.x, framePadBB.Max.y), ImVec2(vCursorPos.x + fCursorOff, framePadBB.Max.y), padBorderColor, borderThickness);
    }

    window->DC.CursorPos = framePadHBB.GetBL() + ImVec2(0., GetFrameHeight() + style.FramePadding.y*2);
    PushStyleColor(ImGuiCol_Text, GetColorU32(ImGuiCol_TextDisabled));
    Text("Press Ctrl or scroll to move the third dimension");
    PopStyleColor();

    window->DC.CursorPosPrevLine = totalBB.Max;
    window->DC.CursorPos = totalBB.Max;

    NewLine();

    return valuePadHChanged || valuePadVChanged || valueSliderChanged;
}


bool MovePad::show1DPadSlider(char const* label,
                              double* p_value, const double* p_min, const double* p_max,
                              const ImRect& bb, const ImRect& containerBB, ImRect& grabBB,
                              const ImGuiID& id, ImGuiWindow* window, ImGuiSliderFlags flags)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    float grabThickness = style.ScrollbarSize / 5.f;
    float grabRadius = grabThickness * 2.5f;

    if (!ImGui::ItemAdd(containerBB, id, &bb, 0))
        return false;

    bool valueChanged = false;

    ImRect expandedBB(bb.Min-ImVec2(grabRadius, grabRadius), bb.Max+ ImVec2(grabRadius, grabRadius));
    bool hovered = ImGui::ItemHoverable( expandedBB, id, g.LastItemData.ItemFlags);

    bool clicked = hovered && ImGui::IsMouseClicked(0, ImGuiInputFlags_None, id);
    bool makeActive = (clicked || g.NavActivateId == id);
    if (makeActive && clicked)
        ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);

    if (makeActive)
    {
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }
    ImU32 frameColor = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);

    ImGui::RenderNavCursor(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, frameColor, true, g.Style.FrameRounding);

    // Render grab
    valueChanged = ImGui::SliderBehavior(bb, id, ImGuiDataType_Double, p_value, p_min, p_max, NULL,
                                         ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat | flags, &grabBB);
    window->DrawList->AddCircleFilled(grabBB.GetCenter(), grabRadius * 1.2f, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_Button));
    window->DrawList->AddCircleFilled(grabBB.GetCenter(), grabRadius, GetColorU32(ImGuiCol_SliderGrab));

    bool showOtherAxis = false;
    { // Add Button
        window->DC.CursorPos = ((flags & ImGuiSliderFlags_Vertical) == ImGuiSliderFlags_Vertical) ? ImVec2(grabBB.Max.x + 2*style.FramePadding.x, grabBB.GetCenter().y - GetFrameHeight() / 2.0f) : ImVec2(grabBB.GetCenter().x-GetFrameHeight()/2.0f, grabBB.Max.y + 2*style.FramePadding.y);

        PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        PushStyleColor(ImGuiCol_ButtonText, GetColorU32(ImGuiCol_Text));
        ImGui::AlignTextToFramePadding();
        const ImVec4& color = ImVec4(strcmp(label, "X")==0? 1.0f : 0.0f,
                                     strcmp(label, "Y")==0? 1.0f : 0.0f,
                                     strcmp(label, "Z")==0? 1.0f : 0.0f,
                                     1.0f);
        ImVec2 size(1.0f, ImGui::GetFrameHeight	()/2.);
        ImGui::GetWindowDrawList()->AddRectFilled(window->DC.CursorPos + ImVec2(0.0f, size.y / 2.),
            window->DC.CursorPos + ImVec2(0.0f, size.y / 2.) +size,
            ImGui::GetColorU32(color), ImGuiStyleVar_FrameRounding); // draw colored axis line in before button

        if (ImGui::Button((std::string(label)+ " " + ICON_FA_CARET_DOWN).c_str(), ImVec2(GetFrameHeight(), GetFrameHeight())))
        {
            showOtherAxis = true;
        }
        PopStyleColor(4);
    }

    SameLine();

    { // Slider value
        window->DC.CursorPos -= ImVec2(style.FramePadding.x * 2, 0.);
        ImGui::LocalInputDouble(("##Value"+std::string(label)).c_str(), p_value);
    }

    // Add popup
    auto idPopup = "##ChangeAxis" + std::string(label);
    if (showOtherAxis)
    {
        ImGui::OpenPopup(idPopup.c_str());
    }

    if (ImGui::BeginPopup(idPopup.c_str()))
    {
        for (int i = 0; i < 3; i++)
        {
            if (m_axis[i] != label)
                if (Selectable(m_axis[i]))
                    swapAxis(label, i);
        }
        ImGui::EndPopup();
    }

    return valueChanged;
}


void MovePad::swapAxis(const char* axisLabel, int axisIndexToSwap)
{
    auto axis = getMappedAxis(axisLabel);
    auto swappingAxis = getMappedAxis(m_axis[axisIndexToSwap]);

    // Swap the values
    auto tmpValue = m_minValues[swappingAxis];
    m_minValues[swappingAxis] = m_minValues[axis];
    m_minValues[axis] = tmpValue;

    tmpValue = m_maxValues[swappingAxis];
    m_maxValues[swappingAxis] = m_maxValues[axis];
    m_maxValues[axis] = tmpValue;

    auto tmp = m_values[swappingAxis];
    m_values[swappingAxis] = m_values[axis];
    m_values[axis] = tmp;

    // Swap the axisLabel and axisIndex axis
    auto tmpAxis = m_mappedAxis[swappingAxis];
    m_mappedAxis[swappingAxis] = m_mappedAxis[axis];
    m_mappedAxis[axis] = tmpAxis;
}


const char* MovePad::getMappedAxis(const char* axis)
{
    for (auto [key, value] : m_mappedAxis)
    {
        if (value == axis)
            return key;
    }
    return nullptr;
}

}
