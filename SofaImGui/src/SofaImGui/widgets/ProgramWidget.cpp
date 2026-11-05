#include <GUIColors.h>
#include <IconsFontAwesome6.h>
#include <string>

#include <sofa/helper/logging/Messaging.h>
#include <SofaImGui/widgets/ProgramWidget.h>
#include <SofaImGui/models/actions/Action.h>
#include <ProgramStyle.h>
#define IMGUI_DEFINE_MATH_OPERATORS // import math operators

#include <imgui_internal.h>

namespace sofaimgui::widgets
{

void BlockBackground(const char* label, const ImRect &bb, const ImVec4 &color, const bool& isSelected)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImRect blockbb = bb;
    const ImGuiID id = ImGui::GetID(label);
    if (!ImGui::ItemAdd(blockbb, id))
        return;

    { // Block background
        drawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y),
                                ImVec2(bb.Max.x, bb.Max.y),
                                ImGui::GetColorU32(color),
                                ImGui::GetStyle().FrameRounding,
                                ImDrawFlags_None);

        if (isSelected)
        {
            const float t = ProgramSizes().BlockSelectionSize * 0.5;
            drawList->AddRect(ImVec2(bb.Min.x - t, bb.Min.y - t),
                              ImVec2(bb.Max.x + t, bb.Max.y + t),
                              COLOR_LIGHT_BLUE,
                              ImGui::GetStyle().FrameRounding,
                              ImDrawFlags_None,
                              ProgramSizes().BlockSelectionSize);
        }
    }

    { // Header background
        ImVec2 padding(ImGui::GetStyle().FramePadding);
        padding.x*=0.5;
        padding.y*=0.5;
        if (bb.Min.x + padding.x < bb.Max.x - padding.x)
        {
            drawList->AddRectFilled(ImVec2(bb.Min.x + padding.x, bb.Min.y + padding.y),
                                    ImVec2(bb.Max.x - padding.x, bb.Min.y + padding.y + ImGui::GetFrameHeight()),
                                    ImGui::GetColorU32(color),
                                    ImGui::GetStyle().FrameRounding,
                                    ImDrawFlags_None);
        }
    }
}

void BeginBlock(const std::string &label, const ImVec2 &size, const ImVec4& color, const bool &isSelected)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    float x = window->DC.CursorPos.x ;
    float y = window->DC.CursorPos.y ;
    window->DC.CursorStartPos.x = x;
    window->DC.CursorStartPos.y = y;

    ImRect bb(ImVec2(x, y), ImVec2(x + size.x, y + size.y));
    sofaimgui::widgets::BlockBackground(label.c_str(), bb, color, isSelected);

    auto rectMin = ImGui::GetItemRectMin();
    auto rectMax = ImGui::GetItemRectMax();
    rectMax.x -= ImGui::GetStyle().FramePadding.x;
    ImGui::PushClipRect(rectMin, rectMax, true);
}

void EndBlock(const std::string &label, const ImVec2 &size)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    window->DC.CursorPosPrevLine.x = window->DC.CursorStartPos.x + size.x;
    window->DC.CursorPosPrevLine.y = window->DC.CursorStartPos.y;

    ImGui::PopClipRect();

    float x = window->DC.CursorStartPos.x ;
    float y = window->DC.CursorStartPos.y ;
    ImRect bb(ImVec2(x, y), ImVec2(x + size.x, y + size.y));

    const ImGuiID id = ImGui::GetID(&label);
    if (!ImGui::ItemAdd(bb, id))
        return;
}

void BlockHeader(const char* icon, char* label, bool& hasValuesChanged)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    float x = window->DC.CursorPos.x ;
    float y = window->DC.CursorPos.y ;

    ImVec2 padding(ImGui::GetStyle().FramePadding);

    ImGui::PushStyleColor(ImGuiCol_Text, ProgramColors().Text);
    { // Header
        x += padding.y * 3;
        y += padding.y * 0.5;

        window->DC.CursorPos.x = x;
        window->DC.CursorPos.y = y;

        auto rectMin = ImGui::GetItemRectMin();
        auto rectMax = ImGui::GetItemRectMax();
        rectMax.x -= padding.x * 2 + ImGui::GetFrameHeight(); // leave space for option button
        ImGui::PushClipRect(rectMin, rectMax, true);

        std::string id = "##comment" + std::to_string(window->DC.CursorPos.x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, COLOR_TRANSPARENT);

        ImGui::Text("%s", icon);
        ImGui::SameLine();
        window->DC.CursorPos.y = y;

        if (ImGui::InputText(id.c_str(), label, models::actions::Action::COMMENTSIZE))
        {
            hasValuesChanged = true;
        }
        ImGui::PopStyleColor();

        ImGui::PopClipRect();
    }
    ImGui::PopStyleColor();

    window->DC.CursorPosPrevLine.x = x;
    window->DC.CursorPosPrevLine.y = y + ImGui::GetStyle().FramePadding.y * 0.5;
}

void BeginBlockLine(const char* label)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    float x = window->DC.CursorPosPrevLine.x ;
    float y = window->DC.CursorPosPrevLine.y ;

    ImVec2 padding(ImGui::GetStyle().FramePadding);

    ImGui::PushStyleColor(ImGuiCol_Text, ProgramColors().Text);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddText(ImVec2(x + padding.x,
                             y + padding.y),
                      ImGui::GetColorU32(ImGuiCol_Text), label);
    ImGui::PopStyleColor();

    window->DC.CursorPos.x = x + ProgramSizes().AlignWidth;
    window->DC.CursorPos.y = y;

    window->DC.CursorPosPrevLine.x = window->DC.CursorPos.x;
    window->DC.CursorPosPrevLine.y = window->DC.CursorPos.y;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, padding);
    ImGui::PushItemWidth(ProgramSizes().InputWidth);
}

bool BeginBlockLockLine(const char* label)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    bool clicked = false;

    float x = window->DC.CursorPosPrevLine.x ;
    float y = window->DC.CursorPosPrevLine.y ;

    ImVec2 padding(ImGui::GetStyle().FramePadding);

    window->DC.CursorPos.x = x;
    window->DC.CursorPos.y = y;

    clicked = ImGui::Button(label);

    window->DC.CursorPos.x = x + ProgramSizes().AlignWidth;
    window->DC.CursorPos.y = y;

    window->DC.CursorPosPrevLine.x = window->DC.CursorPos.x;
    window->DC.CursorPosPrevLine.y = window->DC.CursorPos.y;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, padding);
    ImGui::PushItemWidth(ProgramSizes().InputWidth);

    return clicked;
}

void EndBlockLine()
{
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
}

void BlockNewLine()
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    window->DC.CursorPosPrevLine.x = window->DC.CursorStartPos.x + ImGui::GetStyle().FramePadding.x;
    window->DC.CursorPosPrevLine.y += ImGui::GetFrameHeightWithSpacing();
}

}
