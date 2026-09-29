#include <GUIColors.h>
#include <IconsFontAwesome6.h>
#include <string>

#include <sofa/helper/logging/Messaging.h>
#include <SofaImGui/widgets/ProgramWidget.h>
#include <SofaImGui/models/actions/Action.h>
#include <ProgramStyle.h>


namespace sofaimgui::widgets
{

void BlockBackground(const char* label, const ImRect &bb, const ImVec4 &color, const float &offset)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImRect blockbb = bb;
    blockbb.Min.y -= offset;
    const ImGuiID id = ImGui::GetID(label);
    if (!ImGui::ItemAdd(blockbb, id))
        return;

    { // Block background
        drawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y - offset),
                                ImVec2(bb.Max.x, bb.Max.y),
                                ImGui::GetColorU32(color),
                                ImGui::GetStyle().FrameRounding,
                                ImDrawFlags_None);
    }

    { // Title background
        ImVec2 padding(ImGui::GetStyle().FramePadding);
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

void ActionBlock(const char* label, const ImRect &bb, const ImVec4 &color)
{
    BlockBackground(label, bb, color, 0.);
}

void ModifierBlock(const char* label, const ImRect &bb, double *dragleft, double *dragright, const ImVec4 &color)
{
    float x = bb.Min.x ;
    float y = bb.Min.y ;

    ImVec2 size = bb.GetSize();
    ImVec2 dragSize(2.f, size.y);
    ImRect bbLeft(ImVec2(x, y), ImVec2(x + dragSize.x, y + size.y));
    ImRect bbRight(ImVec2(x + size.x - dragSize.x, y), ImVec2(x + size.x, y + size.y));

    std::string labelLeft = label;
    labelLeft += "dragLeft";
    Drag(labelLeft.c_str(), bbLeft, dragleft);

    std::string labelRight = label;
    labelRight += "dragRight";
    Drag(labelRight.c_str(), bbRight, dragright);

    BlockBackground(label, bb, color, size.y + ImGui::GetStyle().FramePadding.y);
}

void Drag(const char* label, const ImRect &bb, double *value)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    const ImGuiID id = ImGui::GetID(label);
    if (!ImGui::ItemAdd(bb, id))
        return;

    ImGuiContext& g = *GImGui;
    const bool hovered = ImGui::ItemHoverable(bb, id, g.LastItemData.ItemFlags);
    const bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left, ImGuiInputFlags_None, id);
    const bool makeActive = (clicked || g.NavActivateId == id);

    if (hovered || ImGui::IsMouseDown(0, id))
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

    if (clicked)
        ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);

    if (makeActive)
    {
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    double min = -500;
    double max = 500;
    const bool valueChanged = ImGui::DragBehavior(id, ImGuiDataType_Double,
                                                   value, 1., &min, &max, "%0.2f",
                                                   ImGuiSliderFlags_NoInput);
    if (valueChanged)
        ImGui::MarkItemEdited(id);
}

void BeginBlock(const std::string &label, const ImVec2 &size, const ImVec4& color)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    float x = window->DC.CursorPos.x ;
    float y = window->DC.CursorPos.y ;
    window->DC.CursorStartPos.x = x;
    window->DC.CursorStartPos.y = y;

    ImRect bb(ImVec2(x, y), ImVec2(x + size.x, y + size.y));
    sofaimgui::widgets::ActionBlock(label.c_str(), bb, color);

    auto rectMin = ImGui::GetItemRectMin();
    auto rectMax = ImGui::GetItemRectMax();
    rectMax.x -= ImGui::GetStyle().FramePadding.x;
    ImGui::PushClipRect(rectMin, rectMax, true);
}

void EndBlock(const ImVec2 &size)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    window->DC.CursorPosPrevLine.x = window->DC.CursorStartPos.x + size.x;
    window->DC.CursorPosPrevLine.y = window->DC.CursorStartPos.y;

    ImGui::PopClipRect();
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
        y += padding.y;

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
    window->DC.CursorPosPrevLine.y = y;
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
