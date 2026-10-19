#include "GUIColors.h"
#include "IconsFontAwesome6.h"
#include <sofa/helper/logging/Messaging.h>
#include <SofaImGui/widgets/Widgets.h>
#include <string>


namespace sofaimgui::widgets
{

bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    const bool value_changed = ImGui::Combo(label, current_item, items, items_count, height_in_items);
    ImGui::PopStyleVar();
    return value_changed;
}

bool InputDouble(const char* label, double* v, double step, double step_fast, const char*, ImGuiInputTextFlags flags)
{
    float inputWidth = ImGui::CalcTextSize("-100000,00").x;
    if (step>0) // add step buttons width
        inputWidth += ImGui::GetFrameHeight() / 2 + ImGui::GetStyle().ItemSpacing.x * 2;

    ImGui::PushItemWidth(inputWidth);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    const char* format = (abs(*v)!=0. && (log10f(abs(*v))>3 || log10f(abs(*v))<-2))? "%0.2e": "%0.2f";
    bool value_changed =  ImGui::InputDouble(label, v, step, step_fast, format, flags);
    ImGui::PopStyleVar();
    ImGui::PopItemWidth();

    return value_changed;
}

bool InputFloat(const char* label, float* v, float step, float step_fast, const char*, ImGuiInputTextFlags flags)
{
    float inputWidth = ImGui::CalcTextSize("-100000,00").x;
    if (step>0) // add step buttons width
        inputWidth += ImGui::GetFrameHeight() / 2 + ImGui::GetStyle().ItemSpacing.x * 2;

    ImGui::PushItemWidth(inputWidth);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    const char* format = (abs(*v)!=0. && (log10f(abs(*v))>3 || log10f(abs(*v))<-2))? "%0.2e": "%0.2f";
    bool result =  ImGui::InputFloat(label, v, step, step_fast, format, flags);
    ImGui::PopStyleVar();
    ImGui::PopItemWidth();

    return result;
}

bool ToggleButton(const char* str_id, bool* v)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float radius = height * 0.40f;
    float innerRadius = radius * 0.88f;
    float width = innerRadius * 4.0f;
    bool clicked = false;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked())
    {
        *v = !*v;
        clicked = true;
    }

    float t = *v ? 1.0f : 0.0f;

    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg;
    if (ImGui::IsItemHovered())
        col_bg = ImGui::GetColorU32(ImLerp(ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered),
                                           ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered),
                                           t));
    else
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.72f, 0.70f, 0.65f, 1.00f),
                                           ImColor(COLOR_GREEN),
                                           t));

    draw_list->AddRectFilled(ImVec2(p.x, p.y + (height - 2 * radius) / 2.f),
                             ImVec2(p.x + width, p.y + height - (height - 2 * radius) / 2.f), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + height / 2), innerRadius,
                               IM_COL32(255, 255, 255, 255));

    return clicked;
}

bool Button(const char* str_id, const ImVec2 &buttonSize)
{
    return ImGui::Button(str_id, buttonSize);
}


void PushButton(const char* str_id, bool* v, const ImVec2 &buttonSize)
{
    ImVec4 colorActive = ImGui::GetStyle().Colors[ImGuiCol_Button];
    colorActive.x -= 0.25;
    colorActive.y -= 0.25;
    colorActive.z -= 0.25;
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, COLOR_TRANSPARENT);
    ImGui::PushStyleColor(ImGuiCol_Button, *v? colorActive : ImGui::GetStyle().Colors[ImGuiCol_Button]);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, *v? colorActive : ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);

    const bool active = *v;
    if (active)
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  ImGui::GetStyle().TabBarBorderSize * 3);

    if(ImGui::Button(str_id, buttonSize))
    {
        *v = !*v;
    }

    if (active)
        ImGui::PopStyleVar();

    ImGui::PopStyleColor(5);
}

bool CheckBox(const char* label, bool* v)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    bool pressed = CheckBoxEx(label, v);
    ImGui::PopStyleVar(2);
    return pressed;
}

bool RadioButton(const char* label, int* v, int v_button)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    bool pressed = ImGui::RadioButton(label, v, v_button);
    ImGui::PopStyleVar(2);
    return pressed;
}

bool RadioButton(const char* label, bool active)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    bool pressed = ImGui::RadioButton(label, active);
    ImGui::PopStyleVar(2);
    return pressed;
}

bool CheckBoxEx(const char* label, bool* v)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const float square_sz = ImGui::GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    ImVec2 pos2 = ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f);
    const ImRect total_bb(pos, ImVec2(pos.x + pos2.x, pos.y + pos2.y));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
    }

    pos2 = ImVec2(square_sz, square_sz);
    const ImRect check_bb(pos, ImVec2(pos.x + pos2.x, pos.y + pos2.y));
    ImGui::RenderNavHighlight(total_bb, id);
    ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding / 2);
    ImU32 check_col = ImGui::GetColorU32(ImGuiCol_CheckMark);
    bool mixed_value = (g.LastItemData.ItemFlags & ImGuiItemFlags_MixedValue) != 0;
    if (mixed_value || *v)
    {
        // Undocumented tristate/mixed/indeterminate checkbox (#2644)
        // This may seem awkwardly designed because the aim is to make ImGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
        ImVec2 pad(ImMax(1.0f, IM_TRUNC(square_sz / 4.6f)), ImMax(1.0f, IM_TRUNC(square_sz / 4.6f)));

        window->DrawList->AddRectFilled(ImVec2(check_bb.Min.x + pad.x, check_bb.Min.y + pad.y),
                                        ImVec2(check_bb.Max.x - pad.x, check_bb.Max.y - pad.y),
                                        check_col, style.FrameRounding / 4);
    }

    ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
    if (g.LogEnabled)
        ImGui::LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
    if (label_size.x > 0.0f)
        ImGui::RenderText(label_pos, label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

bool BeginCollapsingHeader(const char* label, ImGuiTreeNodeFlags flags)
{
    bool result = ImGui::CollapsingHeader(label, flags);

    if (result)
    {
        ImGui::Indent();
    }

    return result;
}

void EndCollapsingHeader()
{
    ImGui::Spacing();
    ImGui::Unindent();
}

void TextLinkOpenURL(const char* label, const char* url)
{
    std::string _label = ICON_FA_GLOBE" ";
    _label += label;
    ImGui::TextLinkOpenURL(_label.c_str(), url);
}

}
