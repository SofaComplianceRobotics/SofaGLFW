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

#include <sofa/core/visual/VisualParams.h>
#include <sofa/component/visual/BaseCamera.h>
#include <SofaImGui/windows/ViewportWindow.h>
#include <imgui_internal.h>
#include <IconsFontAwesome6.h>

namespace sofaimgui::windows {

ViewportWindow::ViewportWindow(const std::string& name, const bool& isWindowOpen, std::shared_ptr<StateWindow> stateWindow)
    : m_stateWindow(stateWindow)
{
    m_defaultIsOpen = true;
    m_name = name;
    m_isOpen = isWindowOpen;
}

void ViewportWindow::showWindow(sofa::simulation::Node* groot,
                                const ImTextureID& texture,
                                const ImGuiWindowFlags& windowFlags)
{
    if (enabled() && isOpen())
    {
        if (ImGui::Begin(m_name.c_str(), &m_isOpen, windowFlags))
        {
            ImGui::BeginChild("Render", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
            {
                ImVec2 wsize = ImGui::GetWindowSize();
                m_windowSize = {wsize.x, wsize.y};
                m_maxPanelItemWidth = ImGui::CalcTextSize("Input/Output").x + ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetTextLineHeightWithSpacing();

                m_isFocusOnViewport = ImGui::IsWindowFocused();

                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p_min = ImGui::GetCursorScreenPos();
                ImVec2 p_max = ImVec2(p_min.x + wsize.x, p_min.y + wsize.y);
                ImGui::ItemAdd(ImRect(p_min, p_max), ImGui::GetID("ImageRender"));
                dl->AddImageRounded(texture, p_min, p_max,
                                    ImVec2(0, 1), ImVec2(1, 0), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
                                    ImGui::GetStyle().FrameRounding);

                m_isMouseOnViewport = ImGui::IsItemHovered();

                addStateWindow();
                addSimulationTimeAndFPS(groot);

                // Panel backgroung
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 size(ImGui::GetFrameHeight() * 2 + ImGui::GetStyle().ItemSpacing.x * 4 + m_maxPanelItemWidth, ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.y * 2);

                float x = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() / 2.f - ImGui::GetFrameHeight() * 4.f + ImGui::GetStyle().FramePadding.x;
                float y = ImGui::GetWindowPos().y + ImGui::GetStyle().FramePadding.y;

                ImRect bb(ImVec2(x, y), ImVec2(x + size.x, y + size.y));

                { // Draw
                    auto color = ImGui::GetStyle().Colors[ImGuiCol_TabActive];
                    color.w = 0.6f;
                    drawList->AddRectFilled(bb.Min, bb.Max,
                                            ImGui::GetColorU32(color),
                                            ImGui::GetStyle().FrameRounding,
                                            ImDrawFlags_None);
                }

                addCameraButtons(groot);
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}

void ViewportWindow::addStateWindow()
{
    ImGui::SetNextWindowPos(ImGui::GetWindowPos());  // attach the state window to top left of the viewport window
    m_stateWindow->showWindow();
}

void ViewportWindow::addCameraButtons(sofa::simulation::Node* groot)
{
    static bool collapsed = true;
    auto position = ImGui::GetWindowPos();
    ImVec2 buttonSize = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());

    position.x += ImGui::GetStyle().FramePadding.x;
    position.y += ImGui::GetStyle().FramePadding.y;
    ImGui::SetNextWindowPos(position);  // attach the button window to top middle of the viewport window
    ImGui::GetCurrentWindow()->DC.CursorPos = position;

    auto color = ImGui::GetStyle().Colors[ImGuiCol_TabActive];
    color.w = 0.6f;
    ImGui::PushClipRect(ImVec2(ImGui::GetWindowContentRegionMin().x,
                                ImGui::GetWindowContentRegionMin().y),
                        ImVec2(ImGui::GetWindowContentRegionMax().x + ImGui::GetWindowPos().x,
                                ImGui::GetWindowContentRegionMax().y + ImGui::GetWindowPos().y - ImGui::GetFrameHeight() - ImGui::GetStyle().FramePadding.y), true); // Clip down to avoid hidding time
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1); // Work around to add padding
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(color));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetColorU32(color));

    if (ImGui::Begin("ViewportChildLeftButtons", &m_isOpen, ImGuiWindowFlags_ChildWindow |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
    {
        ImGui::TextDisabled("  " ICON_FA_EYE);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
        std::string title = (collapsed) ? ICON_FA_CHEVRON_DOWN : ICON_FA_CHEVRON_UP;
        title+="##viewoptions";

        if(ImGui::Button(title.c_str(), ImVec2(buttonSize.x, buttonSize.y)))
        {
            collapsed = !collapsed;
        }
        
        ImGui::SetItemTooltip(collapsed? "Expand view options": "Collapse view options");
        ImGui::PopStyleColor(3);

        if (groot && !collapsed)
        {
            sofa::component::visual::BaseCamera::SPtr camera;
            groot->get(camera);
            const auto& bbox = groot->f_bbox.getValue();

            { // Fit all
                if (ImGui::Button(ICON_FA_ARROWS_TO_DOT, buttonSize))
                {
                    camera->fitBoundingBox(bbox.minBBox(), bbox.maxBBox());
                    auto bbCenter = (bbox.maxBBox() + bbox.minBBox()) * 0.5f;
                    camera->d_lookAt.setValue(bbCenter);
                }
                ImGui::SetItemTooltip("Fit all");
            }

            { // Center view
                if (ImGui::Button(ICON_FA_BULLSEYE, buttonSize))
                {
                    auto bbCenter = (bbox.maxBBox() + bbox.minBBox()) * 0.5f;
                    camera->d_lookAt.setValue(bbCenter);
                }
                ImGui::SetItemTooltip("Center view");
            }

            { // Othographic / perspective view
                bool ortho = (camera->getCameraType() == sofa::core::visual::VisualParams::ORTHOGRAPHIC_TYPE);
                if (ImGui::Button((!ortho)? ICON_FA_SQUARE: ICON_FA_CUBE, buttonSize))
                {
                    camera->setCameraType((!ortho)? sofa::core::visual::VisualParams::ORTHOGRAPHIC_TYPE: sofa::core::visual::VisualParams::PERSPECTIVE_TYPE);
                }
                ImGui::SetItemTooltip("Orthographic/Perspective");
            }

            { // Axis related
                const float scale = powf(10.0f, floorf(log10f((bbox.maxBBox() - bbox.minBBox()).norm()* 0.01)));

                { // Translate Left/Right
                    ImGui::Button(ICON_FA_ARROWS_LEFT_RIGHT"##TranslateLR", buttonSize);
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Vec3 t = sofa::type::Vec3(1., 0., 0.);
                        t = camera->cameraToWorldTransform(t);
                        t.normalize();
                        t *= ImGui::GetIO().MouseDelta.x * scale;
                        camera->translate(t);
                        camera->translateLookAt(t);
                    }
                    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    ImGui::SetItemTooltip("Translate left/right");
                }

                { // Translate Up/Down
                    ImGui::Button(ICON_FA_ARROWS_UP_DOWN"##TranslateUD", buttonSize);
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Vec3 t = sofa::type::Vec3(0., 1., 0.);
                        t = camera->cameraToWorldTransform(t);
                        t.normalize();
                        t *= ImGui::GetIO().MouseDelta.y * scale;
                        camera->translate(t);
                        camera->translateLookAt(t);
                    }
                    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                    ImGui::SetItemTooltip("Translate up/down");
                }

                { // Zoom
                    ImGui::Button(ICON_FA_MAGNIFYING_GLASS_PLUS"##Zoom", buttonSize);
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Vec3 t = sofa::type::Vec3(0., 0., 1.);
                        t = camera->cameraToWorldTransform(t);
                        t.normalize();
                        const auto& mousedelta = ImGui::GetIO().MouseDelta.x;
                        t *= mousedelta * scale;
                        camera->translate(t);

                        const sofa::type::Vec3 newLookAt = camera->cameraToWorldCoordinates((mousedelta>0)? -t: t);
                        if (dot(camera->getLookAt() - camera->getPosition(), newLookAt - camera->getPosition()) < 0)
                            camera->translateLookAt(newLookAt - camera->getLookAt());
                    }
                    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    ImGui::SetItemTooltip("Zoom");
                }

                const auto &distance = camera->getDistance();
                const auto &lookAt = camera->getLookAtFromOrientation(camera->getPosition(), distance, camera->getOrientation()); // TODO: This should be initialize in BaseCamera
                bool rotate = false;

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 5);
                ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
                { // Rotate X
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 0, 0, 0.5));
                    ImGui::Button(ICON_FA_ROTATE_LEFT"##RotateX", buttonSize);
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>(0.001 * ImGui::GetIO().MouseDelta.x, 0., 0., 1.);
                        camera->rotateCameraAroundPoint(q, lookAt);
                        rotate = true;
                    }
                    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    ImGui::SetItemTooltip("Rotate around X");
                    ImGui::PopStyleColor();
                }

                { // Rotate Y
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 1, 0, 0.5));
                    ImGui::Button(ICON_FA_ROTATE_LEFT"##RotateY", buttonSize);
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>(0., 0.001 * ImGui::GetIO().MouseDelta.x, 0., 1.);
                        camera->rotateCameraAroundPoint(q, lookAt);
                        rotate = true;
                    }
                    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    ImGui::SetItemTooltip("Rotate around Y");
                    ImGui::PopStyleColor();
                }

                { // Rotate Z
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 1, 0.5));
                    ImGui::Button(ICON_FA_ROTATE_LEFT"##RotateZ", buttonSize);
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>(0., 0., 0.001 * ImGui::GetIO().MouseDelta.x, 1.);
                        camera->rotateCameraAroundPoint(q, lookAt);
                        rotate = true;
                    }
                    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    ImGui::SetItemTooltip("Rotate around Z");
                    ImGui::PopStyleColor();
                }
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                if (rotate)
                {
                    // TODO: This should be done in rotateCameraAroundPoint()
                    auto orientation = camera->getOrientation();
                    orientation.normalize();
                    camera->setView(lookAt - orientation.rotate(sofa::type::Vec3(0,0,-distance)), orientation);
                }
            }
        }
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    ImGui::PopClipRect();
    ImGui::EndChild();

    
}

bool ViewportWindow::addStepButton()
{
    ImVec2 buttonSize = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
    bool isItemClicked = false;
    
    if (m_isOpen)
    {
        if (ImGui::Begin(m_name.c_str(), &m_isOpen))
        {
            if(ImGui::BeginChild("Render"))
            {
                auto position = ImGui::GetWindowPos();
                position.x += ImGui::GetWindowWidth() / 2 - ImGui::GetFrameHeight() * 4.f;
                position.y += ImGui::GetStyle().FramePadding.y;
                ImGui::SetNextWindowPos(position);  // attach the button window to top middle of the viewport window

                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                if (ImGui::Begin("ViewportChildButtons", &m_isOpen,
                                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
                {
                    ImGui::SameLine();
                    ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
                    if (ImGui::Button(ICON_FA_FORWARD_STEP, buttonSize))
                        isItemClicked = true;
                    ImGui::PopItemFlag();
                    ImGui::SetItemTooltip("One step of simulation");
                }
                ImGui::End();
                ImGui::PopStyleColor();
                ImGui::EndChild();
            }
        }
        ImGui::End();
    }

    return isItemClicked;
}

bool ViewportWindow::addAnimateButton(bool *animate)
{
    ImVec2 buttonSize = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
    bool isItemClicked = false;
    
    if (m_isOpen)
    {
        if (ImGui::Begin(m_name.c_str(), &m_isOpen))
        {
            if(ImGui::BeginChild("Render"))
            {
                auto position = ImGui::GetWindowPos();
                position.x += ImGui::GetWindowWidth() / 2.f - ImGui::GetFrameHeight() * 4.f;
                position.y += ImGui::GetStyle().FramePadding.y;
                ImGui::SetNextWindowPos(position);  // attach the button window to top middle of the viewport window

                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                if (ImGui::Begin("ViewportChildButtons", &m_isOpen,
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
                {
                    ImGui::SameLine();
                    ImGui::Button(*animate ? ICON_FA_PAUSE : ICON_FA_PLAY, buttonSize);
                    ImGui::SetItemTooltip(*animate ? "Stop simulation" : "Start simulation");

                    if (ImGui::IsItemClicked())
                    {
                        *animate = !*animate;
                        isItemClicked = true;
                    }

                }
                ImGui::End();
                ImGui::PopStyleColor();

                ImGui::EndChild();
            }
        }
        ImGui::End();
    }

    return isItemClicked;
}

bool ViewportWindow::addDrivingTabCombo(int *mode, const char *listModes[], const int &sizeListModes)
{
    bool hasValueChanged = false;
    
    if (m_isOpen)
    {
        if (ImGui::Begin(m_name.c_str(), &m_isOpen))
        {
            if(ImGui::BeginChild("Render"))
            {
                auto position = ImGui::GetWindowPos();
                position.x += ImGui::GetWindowWidth() / 2.f - ImGui::GetFrameHeight() * 4.f;
                position.y += ImGui::GetStyle().FramePadding.y;
                ImGui::SetNextWindowPos(position);  // attach the button window to top middle of the viewport window

                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                if (ImGui::Begin("ViewportChildButtons", &m_isOpen,
                                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
                {
                    ImGui::SameLine();
                    ImGui::PushItemWidth(m_maxPanelItemWidth);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.53f, 0.54f, 0.55f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.53f, 0.54f, 0.55f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.53f, 0.54f, 0.55f, 1.00f));
                    hasValueChanged = ImGui::Combo("##DrivingWindowViewport", mode, listModes, sizeListModes);
                    ImGui::PopStyleColor(3);
                    ImGui::PopItemWidth();
                    ImGui::SetItemTooltip("Choose a window to drive the TCP target");

                }
                ImGui::End();
                ImGui::PopStyleColor();

                ImGui::EndChild();
            }
        }
        ImGui::End();
    }

    return hasValueChanged;
}

void ViewportWindow::addSimulationTimeAndFPS(sofa::simulation::Node* groot)
{
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));

    // Time
    auto position = ImGui::GetWindowWidth() - ImGui::CalcTextSize("Time: 000.000").x - ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetCursorPosX(position);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing());
    ImGui::Text("Time: %.3f", groot->getTime());

    // FPS
    if (groot->animate_.getValue())
    {
        position -= ImGui::CalcTextSize("100.0 FPS ").x;
        ImGui::SetCursorPosX(position);
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing());
        ImGui::Text("%.1f FPS", io.Framerate);
    }

    ImGui::PopStyleColor();
}

}

