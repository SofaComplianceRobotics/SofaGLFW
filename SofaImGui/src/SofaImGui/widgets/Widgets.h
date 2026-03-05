#pragma once
#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui
{

/// Local redefinition of ImGui::Combo. Adds a border to the combo box.
bool LocalCombo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);

/// Local redefinition of ImGui::InputDouble. Adds a border to the input box, adjusts its width, and automatically selects the display format based on the value.
bool LocalInputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);

/// Local redefinition of ImGui::InputFloat. Adds a border to the input box, adjusts its width, and automatically selects the display format based on the value.
bool LocalInputFloat(const char* label, float* v, float step = 0.0, float step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);

/// Adds a toggle button widget, which is not present in standard ImGui.
bool LocalToggleButton(const char* str_id, bool* v);

/// Local redefinition of ImGui::PushButton. Adds a push button that maintains its state.
void LocalPushButton(const char* str_id, bool *v, const ImVec2 &buttonSize = ImVec2(0, 0));

/// Local redefinition of ImGui::Checkbox. Adjusts padding and border size for a more compact appearance.
bool LocalCheckBox(const char* label, bool* v);

/// Local redefinition of ImGui::RadioButton. Adjusts padding and border size for a more compact appearance.
bool LocalRadioButton(const char* label, int* v, int v_button);
bool LocalRadioButton(const char* label, bool active);

/// Local redefinition of ImGui::Checkbox. Core implementation with style adjustments.
bool LocalCheckBoxEx(const char* label, bool* v);

/// Local redefinition of ImGui::CollapsingHeader. Adds indentation when expanded.
bool LocalBeginCollapsingHeader(const char* label, ImGuiTreeNodeFlags flags);

/// Ends a local collapsing header, removing the indentation added when expanded.
void LocalEndCollapsingHeader();

/// Format: ICON_FA_GLOBE Open label
void LocalTextLinkOpenURL(const char* label, const char* url);

// ProgramWindow widgets

/// Draws a colored block with a title area.
void Block(const char* label, const ImRect &bb, const ImVec4 &color, const float &offset);

/// Draws an action block.
void ActionBlock(const char* label, const ImRect &bb, const ImVec4 &color);

/// Draws a modifier block with draggable left and right handles.
void ModifierBlock(const char* label, const ImRect &bb, double *dragleft, double *dragright, const ImVec4 &color);

/// Draws a draggable area within the specified bounding box.
void Drag(const char* label, const ImRect &bb, double *value);

}
