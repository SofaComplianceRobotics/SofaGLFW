#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <string>

namespace sofaimgui::widgets
{

/// Draws a colored block with a header area.
void BlockBackground(const char* label, const ImRect &bb, const ImVec4 &color, const bool& isSelected);

/// Begin drawing a program block
void BeginBlock(const std::string &label, const ImVec2 &size, const ImVec4 &color, const bool & isSelected = false);
void EndBlock(const std::string &label, const ImVec2& size);

void BlockHeader(const char *icon, char* label, bool& hasValuesChanged);

void BeginBlockLine(const char* label);
bool BeginBlockLockLine(const char* label);
void EndBlockLine();

void BlockNewLine();
}
