#pragma once
#include <imgui.h>
#include <imgui_internal.h>

namespace sofaimgui::widgets
{

/// Redifinition of ImGui::Combo. Adds a border to the combo box.
bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);

/// Redifinition of ImGui::InputDouble. Adds a border to the input box, adjusts its width, and automatically selects the display format based on the value.
bool InputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);

/// Redifinition of ImGui::InputFloat. Adds a border to the input box, adjusts its width, and automatically selects the display format based on the value.
bool InputFloat(const char* label, float* v, float step = 0.0, float step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);

/// Adds a toggle button widget, which is not present in standard ImGui.
bool ToggleButton(const char* str_id, bool* v);

/// Redifinition of ImGui::Button. Adds square size.
bool Button(const char* str_id, const ImVec2 &buttonSize = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));

/// Redifinition of ImGui::PushButton. Adds a push button that maintains its state.
void PushButton(const char* str_id, bool *v, const ImVec2 &buttonSize = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));

/// Redifinition of ImGui::Checkbox. Adjusts padding and border size for a more compact appearance.
bool CheckBox(const char* label, bool* v);

/// Redifinition of ImGui::RadioButton. Adjusts padding and border size for a more compact appearance.
bool RadioButton(const char* label, int* v, int v_button);
bool RadioButton(const char* label, bool active);

/// Redifinition of ImGui::Checkbox. Core implementation with style adjustments.
bool CheckBoxEx(const char* label, bool* v);

/// Redifinition of ImGui::CollapsingHeader. Adds indentation when expanded.
bool BeginCollapsingHeader(const char* label, ImGuiTreeNodeFlags flags);

/// Ends a local collapsing header, removing the indentation added when expanded.
void EndCollapsingHeader();

/// Format: ICON_FA_GLOBE Open label
void TextLinkOpenURL(const char* label, const char* url);

}
