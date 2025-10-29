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

#include <SofaGLFW/SofaGLFWWindow.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/component/visual/BaseCamera.h>
#include <SofaImGui/windows/ViewportWindow.h>
#include <imgui_internal.h>
#include <IconsFontAwesome6.h>
#include <SofaImGui/widgets/Gizmos.h>
#include <GLFW/glfw3.h>

namespace sofaimgui::windows {

ViewportWindow::ViewportWindow(const std::string& name, const bool& isWindowOpen, std::shared_ptr<StateWindow> stateWindow)
    : m_stateWindow(stateWindow)
{
    m_defaultIsOpen = true;
    m_name = name;
    m_isOpen = isWindowOpen;
}

void ViewportWindow::showWindow(sofaglfw::SofaGLFWBaseGUI* baseGUI,
                                sofa::simulation::Node* groot,
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

                addCameraButtons(baseGUI, groot);
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

bool ViewportWindow::checkCamera(sofa::simulation::Node* groot)
{
    if (groot) // Check the groot
    {
        sofa::component::visual::BaseCamera::SPtr camera;
        groot->get(camera);
        if (camera) // Check if there is a camera in the scene
        {
            sofa::type::BoundingBox bb(camera->d_minBBox.getValue(), camera->d_maxBBox.getValue());
            if (bb.isValid()) // Check that the bounding box is correctly initialized
                return true;
        }
    }

    return false;
}

void ViewportWindow::addCameraButtons(sofaglfw::SofaGLFWBaseGUI* baseGUI, sofa::simulation::Node* groot)
{
    // If the camera is not correctly initialized don't draw anything
    if (!checkCamera(groot))
        return;

    // Positions and sizes
    static bool collapsed = true;
    const auto& wpos = ImGui::GetMainViewport()->Pos;
    const auto& cwpos = ImGui::GetCurrentWindow()->Pos;
    auto position = ImGui::GetWindowPos();
    ImGui::GetCurrentWindow()->DC.CursorPos = position;

    // Camera
    sofa::component::visual::BaseCamera::SPtr camera;
    groot->get(camera);

    // Gizmos
    static bool orientationGizmo = false;
    double frameGizmoSize = ImGui::GetFrameHeight() * 4;
    double orientationGizmoSize = frameGizmoSize;
    bool axisClicked[3]{false};
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    if (ImGui::Begin("ViewportChildGizmos", &m_isOpen, ImGuiWindowFlags_ChildWindow |
                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
    {
        ImRect wosize = ImRect(wpos, ImVec2(wpos.x + frameGizmoSize + orientationGizmo * orientationGizmoSize, wpos.y + frameGizmoSize));
        ImGui::ItemSize(wosize);
        ImGui::ItemAdd(wosize, ImGui::GetID("ViewportGizmos"));

        {// Frame & orientation gizmo
            // Base camera matrices are in double
            double modelview[16];
            double projection[16];
            const auto& type = camera->getCameraType();
            camera->setCameraType(sofa::core::visual::VisualParams::PERSPECTIVE_TYPE);
            camera->getOpenGLModelViewMatrix(modelview);
            camera->getOpenGLProjectionMatrix(projection);
            camera->setCameraType(type);
            // ImGui matrices are in float, so we convert
            float mview[16];
            float proj[16];
            for (int i=0; i<16; i++)
            {
                mview[i] = modelview[i];
                proj[i] = projection[i];
            }

            { // Frame gizmo
                bool axisClicked[6]{false};
                sofaimgui::widget::SetRect(position.x, position.y, frameGizmoSize);
                sofaimgui::widget::DrawFrameGizmo(mview, proj, axisClicked);
                if (axisClicked[0])
                    sofaglfw::SofaGLFWWindow::alignCamera(groot, sofaglfw::SofaGLFWWindow::CameraAlignement::LEFT);
                else if (axisClicked[1])
                    sofaglfw::SofaGLFWWindow::alignCamera(groot, sofaglfw::SofaGLFWWindow::CameraAlignement::TOP);
                else if (axisClicked[2])
                    sofaglfw::SofaGLFWWindow::alignCamera(groot, sofaglfw::SofaGLFWWindow::CameraAlignement::FRONT);
                else if (axisClicked[3])
                    sofaglfw::SofaGLFWWindow::alignCamera(groot, sofaglfw::SofaGLFWWindow::CameraAlignement::RIGHT);
                else if (axisClicked[4])
                    sofaglfw::SofaGLFWWindow::alignCamera(groot, sofaglfw::SofaGLFWWindow::CameraAlignement::BOTTOM);
                else if (axisClicked[5])
                    sofaglfw::SofaGLFWWindow::alignCamera(groot, sofaglfw::SofaGLFWWindow::CameraAlignement::BACK);
            }

            { // Orientation gizmo
                if (orientationGizmo)
                {
                    // Center of the viewport (look at position)
                    sofaimgui::widget::SetRect(position.x + frameGizmoSize,
                                               position.y,
                                               orientationGizmoSize);
                    sofaimgui::widget::DrawOrientationGizmo(mview, proj, axisClicked);
                }
            }
        }
    }
    ImGui::PopStyleColor();
    ImGui::EndChild();

    position = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + frameGizmoSize);

    position.x += ImGui::GetStyle().FramePadding.x;
    position.y += ImGui::GetStyle().FramePadding.y;
    ImGui::SetNextWindowPos(position);  // attach the button window to top middle of the viewport window
    ImGui::GetCurrentWindow()->DC.CursorPos = position;

    // Left buttons background
    // Clip down
    auto color = ImGui::GetStyle().Colors[ImGuiCol_TabActive];
    color.w = 0.6f;
    ImGui::PushClipRect(ImVec2(ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y),
                        ImVec2(ImGui::GetWindowContentRegionMax().x + ImGui::GetWindowPos().x, ImGui::GetWindowContentRegionMax().y + ImGui::GetWindowPos().y - ImGui::GetStyle().FramePadding.y), true);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1); // Work around to add padding
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(color));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetColorU32(color));

    // When clicking these buttons, the mouse only moves into the current window area
    // We allow the mouse to cross walls and reapear on the other side
    auto dpos = ImGui::GetIO().MouseDelta;
    dpos.x = std::clamp(int(dpos.x), -20, 20); // Clamp the mouse delta, set the maximum speed
    dpos.y = std::clamp(int(dpos.y), -20, 20);

    // Buttons
    ImVec2 buttonSize = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
    bool translate = false;
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

        if (!collapsed)
        {
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

            { // Orientation gizmo button
                if (ImGui::Button(ICON_FA_ROTATE, buttonSize))
                {
                    orientationGizmo = !orientationGizmo;
                }
                std::string text = (orientationGizmo)? "Disable ": "Enable ";
                text += "orientation gizmo \n(Rotation Center: Look At)";
                ImGui::SetItemTooltip("%s", text.c_str());
            }

            ImGui::PushStyleColor(ImGuiCol_Separator, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
            ImGui::PopStyleColor();

            { // Axis related
                const float scale = powf(10.0f, floorf(log10f((bbox.maxBBox() - bbox.minBBox()).norm()* 0.01)));

                { // Translate Left/Right
                    ImGui::Button(ICON_FA_ARROWS_LEFT_RIGHT"##TranslateLR", buttonSize);
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Vec3 t = sofa::type::Vec3(1., 0., 0.);
                        t = camera->cameraToWorldTransform(t);
                        t.normalize();
                        t *= dpos.x * scale;
                        camera->translate(t);
                        camera->translateLookAt(t);
                        translate = true;
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
                        t *= dpos.y * scale;
                        camera->translate(t);
                        camera->translateLookAt(t);
                        translate = true;
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
                        const auto& mousedelta = dpos.x;
                        t *= mousedelta * scale;
                        camera->translate(t);
                        translate = true;

                        const sofa::type::Vec3 newLookAt = camera->cameraToWorldCoordinates((mousedelta>0)? -t: t);
                        if (dot(camera->getLookAt() - camera->getPosition(), newLookAt - camera->getPosition()) < 0)
                            camera->translateLookAt(newLookAt - camera->getLookAt());
                    }
                    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    ImGui::SetItemTooltip("Zoom");
                }
            }
        }
    }

    const auto& cpos = ImGui::GetIO().MousePos;
    // When setting the mouse position, the value is relative to the window top left corner
    // Thus we need to compute the shifts between this position and the top left corner of the current window area
    const float xshift = (cwpos.x - wpos.x);
    const float yshift = (cwpos.y - wpos.y);

    const double &distance = camera->getDistance();
    const sofa::type::Vec3 &lookAt = camera->getLookAtFromOrientation(camera->getPosition(), distance, camera->getOrientation()); // TODO: This should be initialize in BaseCamera
    bool rotate = false;

    { // Orientation gizmo clicked
        // Rotate X
            if (axisClicked[0])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>(0.001 * dpos.x, 0., 0., 1.);
                camera->rotateCameraAroundPoint(q, lookAt);
                rotate = true;
            }
        // Rotate Y
            else if (axisClicked[1])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>(0., 0.001 * dpos.x, 0., 1.);
                camera->rotateCameraAroundPoint(q, lookAt);
                rotate = true;
            }
        // Rotate Z
            else if (axisClicked[2])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>(0., 0., 0.001 * dpos.x, 1.);
                camera->rotateCameraAroundPoint(q, lookAt);
                rotate = true;
            }
    }

    if (rotate)
    {
        // TODO: This should be done in rotateCameraAroundPoint()
        auto orientation = camera->getOrientation();
        orientation.normalize();
        camera->setView(lookAt - orientation.rotate(sofa::type::Vec3(0,0,-distance)), orientation);
    }

    // Allow the mouse to cross walls, and reapear on the other side of the current window area
    if (rotate || translate)
    {
        if (cpos.x < cwpos.x)
            baseGUI->setMousePos(xshift + m_windowSize.first, cpos.y - wpos.y);
        if (cpos.x > cwpos.x + m_windowSize.first)
            baseGUI->setMousePos(xshift + buttonSize.x / 2., cpos.y - wpos.y);
        if (cpos.y < cwpos.y)
            baseGUI->setMousePos(cpos.x - wpos.x, yshift + m_windowSize.second);
        if (cpos.y > cwpos.y + m_windowSize.second)
            baseGUI->setMousePos(cpos.x - wpos.x, yshift);
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

