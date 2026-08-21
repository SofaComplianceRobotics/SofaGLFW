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
#define IMGUI_DEFINE_MATH_OPERATORS // import math operators

#include <sofa/core/visual/VisualParams.h>
#include <sofa/component/visual/BaseCamera.h>

#include <imgui_internal.h>

#include <IconsFontAwesome6.h>
#include <Style.h>
#include <GUIColors.h>

#include <SofaGLFW/SofaGLFWWindow.h>
#include <GLFW/glfw3.h>

#include <SofaImGui/Workbench.h>
#include <SofaImGui/FooterStatusBar.h>
#include <SofaImGui/DrivingWindow.h>
#include <SofaImGui/windows/ViewportWindow.h>
#include <SofaImGui/windows/WindowsSettingsName.h>
#include <SofaImGui/widgets/Widgets.h>
#include <SofaImGui/widgets/Gizmos.h>

namespace sofaimgui::windows {

ViewportWindow::ViewportWindow(const std::string& name)
    : BaseWindow(name)
{
    m_windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
}

std::string ViewportWindow::getDescription()
{
    return "Main viewport rendering window.";
}

void ViewportWindow::registerAndLoadWindowSettings()
{
    registerAndLoadSetting(WS_VIEWPORT_ORIENTATIONGIZMOENABLED, m_ws_orientationGizmoEnabled, WindowsSettings::SettingType::BOOL);
    registerAndLoadSetting(WS_VIEWPORT_CAMERABUTTONCOLLAPSE, m_ws_cameraButtonsCollapsed, WindowsSettings::SettingType::BOOL);
    registerAndLoadSetting(WS_VIEWPORT_DRIVINGWINDOW, m_ws_drivingWindow, WindowsSettings::SettingType::LONG);
}

void ViewportWindow::internalShowWindow()
{
    ImGui::BeginChild("Render", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        ImVec2 viewportPos = ImGui::GetWindowPos();
        m_baseGUI->updateViewportPosition(viewportPos.x, viewportPos.y);

        ImVec2 wsize = ImGui::GetWindowSize();
        m_windowSize = {wsize.x, wsize.y};
        m_maxPanelItemWidth = ImGui::CalcTextSize("Input/Output").x + ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetTextLineHeightWithSpacing();

        m_isFocusOnViewport = ImGui::IsWindowFocused();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p_min = ImGui::GetCursorScreenPos();
        ImVec2 p_max = ImVec2(p_min.x + wsize.x, p_min.y + wsize.y);
        ImGui::ItemAdd(ImRect(p_min, p_max), ImGui::GetID("ImageRender"));
        dl->AddImageRounded(m_textureID, p_min, p_max,
                            ImVec2(0, 1), ImVec2(1, 0), COLOR_WHITE,
                            ImGui::GetStyle().FrameRounding);

        m_isMouseOnViewport = ImGui::IsWindowHovered();

        if (workbench != Workbench::SCENE_EDITOR)
        {
            addSimulationTimeAndFPS();
        }

        addCameraButtons();
        if(m_baseGUI->isVideoRecording())
            addRecordingStatus(ImColor(COLOR_RED));
        addContextMenu(m_textureID);
    }
    ImGui::EndChild();
}

bool ViewportWindow::checkCamera()
{
    auto groot = m_baseGUI->getRootNode().get();
    if (groot) // Check the groot
    {
        sofa::component::visual::BaseCamera::SPtr camera;
        groot->get(camera);
        if (camera) // Check if there is a camera in the scene
        {
            sofa::type::BoundingBox bb(camera->d_minBBox.getValue(), camera->d_maxBBox.getValue());
            if (bb.isValid() && !bb.isFlat()) // Check that the bounding box is correctly initialized
                return true;
        }
    }

    return false;
}

void ViewportWindow::addCameraButtons()
{
    auto groot = m_baseGUI->getRootNode().get();

    // If the camera is not correctly initialized don't draw anything
    if (!checkCamera())
        return;

    // Positions and sizes
    const auto& wpos = ImGui::GetMainViewport()->Pos;
    auto position = ImGui::GetWindowPos();
    ImGui::GetCurrentWindow()->DC.CursorPos = position;

    // Camera
    sofa::component::visual::BaseCamera::SPtr camera;
    groot->get(camera);

    // Gizmos
    double frameGizmoSize = ImGui::GetFrameHeight() * 4;
    double orientationGizmoSize = frameGizmoSize;
    bool axisClicked[3]{false};
    ImGui::PushStyleColor(ImGuiCol_ChildBg, COLOR_TRANSPARENT);
    if (ImGui::Begin("ViewportChildGizmos", &isOpen(), ImGuiWindowFlags_ChildWindow| ImGuiWindowFlags_AlwaysAutoResize |
                                                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
    {
        ImRect wosize = ImRect(wpos, ImVec2(wpos.x + frameGizmoSize + m_ws_orientationGizmoEnabled * orientationGizmoSize, wpos.y + frameGizmoSize));
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
                sofaimgui::widgets::SetRect(position.x, position.y, frameGizmoSize);
                sofaimgui::widgets::DrawFrameGizmo(mview, proj, axisClicked);
                if (axisClicked[0])
                    sofaglfw::SofaGLFWWindow::alignCamera(m_baseGUI, sofaglfw::SofaGLFWWindow::CameraAlignement::LEFT);
                else if (axisClicked[1])
                    sofaglfw::SofaGLFWWindow::alignCamera(m_baseGUI, sofaglfw::SofaGLFWWindow::CameraAlignement::TOP);
                else if (axisClicked[2])
                    sofaglfw::SofaGLFWWindow::alignCamera(m_baseGUI, sofaglfw::SofaGLFWWindow::CameraAlignement::FRONT);
                else if (axisClicked[3])
                    sofaglfw::SofaGLFWWindow::alignCamera(m_baseGUI, sofaglfw::SofaGLFWWindow::CameraAlignement::RIGHT);
                else if (axisClicked[4])
                    sofaglfw::SofaGLFWWindow::alignCamera(m_baseGUI, sofaglfw::SofaGLFWWindow::CameraAlignement::BOTTOM);
                else if (axisClicked[5])
                    sofaglfw::SofaGLFWWindow::alignCamera(m_baseGUI, sofaglfw::SofaGLFWWindow::CameraAlignement::BACK);
            }

            { // Orientation gizmo
                if (m_ws_orientationGizmoEnabled)
                {
                    // Center of the viewport (look at position)
                    sofaimgui::widgets::SetRect(position.x + frameGizmoSize,
                                               position.y,
                                               orientationGizmoSize);
                    sofaimgui::widgets::DrawOrientationGizmo(mview, proj, axisClicked);
                }
            }
        }
    }
    ImGui::PopStyleColor();
    ImGui::EndChild();

    position = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + frameGizmoSize);

    position.x += ImGui::GetStyle().FramePadding.x;
    position.y += ImGui::GetStyle().FramePadding.y;
    ImGui::SetNextWindowPos(position);  // attach the button window to left of the viewport window, after the gizmos
    ImGui::GetCurrentWindow()->DC.CursorPos = position;

    // Left buttons background
    // Clip down
    auto color = ImGui::GetStyle().Colors[ImGuiCol_TabActive];
    color.w = 0.6f;
    ImGui::PushClipRect(ImVec2(ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y),
                        ImVec2(ImGui::GetWindowContentRegionMax().x + ImGui::GetWindowPos().x,
                               ImGui::GetWindowContentRegionMax().y + ImGui::GetWindowPos().y - ImGui::GetStyle().FramePadding.y),
                        true);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1); // Work around to add padding
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(color));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetColorU32(color));

    // When clicking these buttons, the mouse only moves into the current window area
    // We allow the mouse to cross walls and reapear on the other side
    auto dpos = ImGui::GetIO().MouseDelta;
    // Scale from camera pixel to world displacement
    dpos *= 1e-3;

    // Buttons
    bool translate = false;
    if (ImGui::Begin("ViewportChildLeftButtons", &isOpen(), ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_AlwaysAutoResize |
                                                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
    {
        ImGui::TextDisabled("  " ICON_FA_VIDEO);

        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_TRANSPARENT);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
        std::string title = (m_ws_cameraButtonsCollapsed) ? ICON_FA_CHEVRON_DOWN : ICON_FA_CHEVRON_UP;
        title+="##viewoptions";

        if(sofaimgui::widgets::Button(title.c_str()))
        {
            m_ws_cameraButtonsCollapsed = !m_ws_cameraButtonsCollapsed;
        }
        
        ImGui::SetItemTooltip(m_ws_cameraButtonsCollapsed? "Expand view options": "Collapse view options");
        ImGui::PopStyleColor(3);

        if (!m_ws_cameraButtonsCollapsed)
        {
            const auto& bbox = groot->f_bbox.getValue();

            { // 3D view display options
                if (ImGui::BeginPopup("##DisplayOptions"))
                {
                    menus::ViewMenu(m_baseGUI).addShowIn3DViewMenuItems();
                    ImGui::EndPopup();
                }

                if (sofaimgui::widgets::Button(ICON_FA_EYE))
                {
                    ImGui::OpenPopup("##DisplayOptions");
                }
                ImGui::SetItemTooltip("Show...");
            }

            ImGui::PushStyleColor(ImGuiCol_Separator, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
            ImGui::PopStyleColor();

            { // Fit all
                if (sofaimgui::widgets::Button(ICON_FA_ARROWS_TO_DOT))
                {
                    camera->fitBoundingBox(bbox.minBBox(), bbox.maxBBox());
                    auto bbCenter = (bbox.maxBBox() + bbox.minBBox()) * 0.5f;
                    camera->d_lookAt.setValue(bbCenter);
                }
                ImGui::SetItemTooltip("Fit all");
            }

            { // Center view
                if (sofaimgui::widgets::Button(ICON_FA_BULLSEYE))
                {
                    auto bbCenter = (bbox.maxBBox() + bbox.minBBox()) * 0.5f;
                    camera->d_lookAt.setValue(bbCenter);
                }
                ImGui::SetItemTooltip("Center view");
            }

            { // Othographic / perspective view
                bool ortho = (camera->getCameraType() == sofa::core::visual::VisualParams::ORTHOGRAPHIC_TYPE);
                if (sofaimgui::widgets::Button((!ortho)? ICON_FA_SQUARE: ICON_FA_CUBE))
                {
                    camera->setCameraType((!ortho)? sofa::core::visual::VisualParams::ORTHOGRAPHIC_TYPE: sofa::core::visual::VisualParams::PERSPECTIVE_TYPE);
                    sofaglfw::SofaGLFWWindow::userSelectedOrthographic = !ortho;
                }
                ImGui::SetItemTooltip("Orthographic/Perspective");
            }

            { // Orientation gizmo button
                if (sofaimgui::widgets::Button(ICON_FA_ROTATE))
                {
                    m_ws_orientationGizmoEnabled = !m_ws_orientationGizmoEnabled;
                }
                std::string text = (m_ws_orientationGizmoEnabled)? "Disable ": "Enable ";
                text += "orientation gizmo \n(Rotation Center: Look At)";
                ImGui::SetItemTooltip("%s", text.c_str());
            }

            ImGui::PushStyleColor(ImGuiCol_Separator, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
            ImGui::PopStyleColor();

            { // Axis related
                { // Translate Left/Right
                    sofaimgui::widgets::Button(ICON_FA_ARROWS_LEFT_RIGHT"##TranslateLR");
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Vec3 t = sofa::type::Vec3(1., 0., 0.);
                        t = camera->cameraToWorldTransform(t);
                        t.normalize();
                        t *= - dpos.x * camera->getDistance(); // Compute scale based on distance. The further the camera, the faster the translation.
                        camera->translate(t);
                        camera->translateLookAt(t);
                        translate = true;
                    }
                    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    ImGui::SetItemTooltip("Translate left/right");
                }

                { // Translate Up/Down
                    sofaimgui::widgets::Button(ICON_FA_ARROWS_UP_DOWN"##TranslateUD");
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Vec3 t = sofa::type::Vec3(0., 1., 0.);
                        t = camera->cameraToWorldTransform(t);
                        t.normalize();
                        t *= dpos.y * camera->getDistance(); // Compute scale based on distance. The further the camera, the faster the translation.
                        camera->translate(t);
                        camera->translateLookAt(t);
                        translate = true;
                    }
                    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                    ImGui::SetItemTooltip("Translate up/down");
                }

                { // Zoom
                    sofaimgui::widgets::Button(ICON_FA_MAGNIFYING_GLASS_PLUS"##Zoom");
                    if (ImGui::IsItemActive())
                    {
                        sofa::type::Vec3 t = sofa::type::Vec3(0., 0., 1.);
                        t = camera->cameraToWorldTransform(t);
                        t.normalize();
                        const auto& mousedelta = dpos.x * camera->getDistance();
                        t *= mousedelta;
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

    bool rotate = false;
    { // Orientation gizmo clicked
        const double &distance = camera->getDistance();
        const sofa::type::Vec3 &lookAt = camera->getLookAt();

        auto getRotationCoef = [dpos, camera](sofa::type::Vec3 axis) -> float
        {
            auto sorigin = camera->worldToScreenPoint(sofa::type::Vec3(0, 0, 0));
            auto saxis = camera->worldToScreenPoint(axis);

            ImVec2 spos(-(saxis[1]-sorigin[1]), (saxis[0]-sorigin[0])); // orthogonal axis in image coord


            if(sqrt(spos.x*spos.x + spos.y*spos.y) < 1e-10)
            {
                spos.x = abs(dpos.y)>abs(dpos.x)? 1: 0;
                spos.y = abs(dpos.y)>abs(dpos.x)? 0: 1;
            }
            return (spos.x*dpos.x + spos.y*dpos.y) / sqrt(spos.x*spos.x + spos.y*spos.y);
        };

        // Rotate X
        if (axisClicked[0])
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>(getRotationCoef(sofa::type::Vec3(1., 0., 0.)), 0., 0., 1.);
            q.normalize();
            camera->rotateCameraAroundPoint(q, lookAt);
            rotate = true;
        }
        // Rotate Y
        else if (axisClicked[1])
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>(0., getRotationCoef(sofa::type::Vec3(0., 1., 0.)), 0., 1.);
            q.normalize();
            camera->rotateCameraAroundPoint(q, lookAt);
            rotate = true;
        }
        // Rotate Z
        else if (axisClicked[2])
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            sofa::type::Quat<SReal> q = sofa::type::Quat<SReal>(0., 0., getRotationCoef(sofa::type::Vec3(0., 0., 1.)), 1.);
            q.normalize();
            camera->rotateCameraAroundPoint(q, lookAt);
            rotate = true;
        }

        if (rotate)
        {
            // TODO: This should be done in rotateCameraAroundPoint()
            auto orientation = camera->getOrientation();
            orientation.normalize();
            camera->setView(lookAt - orientation.rotate(sofa::type::Vec3(0., 0., -distance)), orientation);
        }
    }

    // Hides and grabs the cursor, providing virtual and unlimited cursor movement.
    m_baseGUI->setDisabledMouse(rotate || translate);

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    ImGui::PopClipRect();
    ImGui::EndChild();
}

void ViewportWindow::addContextMenu(const ImTextureID& texture)
{
    if (ImGui::BeginPopup("##ViewportContextMenu"))
    {
        menus::ViewMenu viewMenu(m_baseGUI);
        viewMenu.addSaveCameraMenuItem();
        viewMenu.addRestoreCameraMenuItem();

        ImGui::Separator();

        viewMenu.addRecordVideoMenuItem();
        viewMenu.addSaveScreenShotMenuItem(std::pair<unsigned int, unsigned int>(m_windowSize.first, m_windowSize.second), texture);
        ImGui::EndPopup();
    }

    // Right click and drag: translates the view
    // Simple right click (same position) opens the context menu
    const auto& io = ImGui::GetIO();
    if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)
        && io.MouseClickedPos[ImGuiMouseButton_Right].x == ImGui::GetMousePos().x
        && io.MouseClickedPos[ImGuiMouseButton_Right].y == ImGui::GetMousePos().y)
    {
        ImGui::OpenPopup("##ViewportContextMenu");
    }
}

bool ViewportWindow::addAnimateButton(bool *animate, const float &shift_x)
{
    bool isItemClicked = false;

    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &isOpen()))
        {
            if (ImGui::BeginChild("Render"))
            {
                auto position = ImGui::GetWindowPos();
                position.x += ImGui::GetWindowWidth() * 0.5f - shift_x;
                position.y += ImGui::GetStyle().FramePadding.y;
                ImGui::SetNextWindowPos(position);  // attach the button window to top middle of the viewport window

                // Middle buttons background
                // Clip down
                auto color = ImGui::GetStyle().Colors[ImGuiCol_TabActive];
                color.w = 0.6f;
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1); // Work around to add padding
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(color));
                ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetColorU32(color));

                if (ImGui::Begin("ViewportChildMiddleButtons", &isOpen(), ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_AlwaysAutoResize |
                                                                          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
                {
                    sofaimgui::widgets::Button(*animate ? ICON_FA_PAUSE : ICON_FA_PLAY);
                    ImGui::SetItemTooltip(*animate ? "Stop simulation" : "Start simulation");

                    if (ImGui::IsItemClicked())
                    {
                        *animate = !*animate;
                        isItemClicked = true;
                    }
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    return isItemClicked;
}

bool ViewportWindow::addStepButton()
{
    bool isItemClicked = false;
    
    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &isOpen()))
        {
            if (ImGui::BeginChild("Render"))
            {
                if (ImGui::Begin("ViewportChildMiddleButtons"))
                {
                    ImGui::SameLine();
                    ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
                    if (sofaimgui::widgets::Button(ICON_FA_FORWARD_STEP))
                        isItemClicked = true;
                    ImGui::PopItemFlag();
                    ImGui::SetItemTooltip("One step of simulation");
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    return isItemClicked;
}

bool ViewportWindow::addReloadButton()
{
    bool isItemClicked = false;

    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &isOpen()))
        {
            if (ImGui::BeginChild("Render"))
            {
                if (ImGui::Begin("ViewportChildMiddleButtons"))
                {
                    ImGui::SameLine();
                    if (sofaimgui::widgets::Button(ICON_FA_ROTATE_LEFT))
                        isItemClicked = true;
                    ImGui::SetItemTooltip("Reload the simulation");
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    return isItemClicked;
}

void ViewportWindow::addDrivingTabCombo()
{
    int dw = m_ws_drivingWindow;
    drivingWindow = DrivingWindow(dw);

    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &isOpen()))
        {
            if (ImGui::BeginChild("Render"))
            {
                if (ImGui::Begin("ViewportChildMiddleButtons"))
                {
                    ImGui::SameLine();
                    ImGui::PushItemWidth(m_maxPanelItemWidth);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.53f, 0.54f, 0.55f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.53f, 0.54f, 0.55f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.53f, 0.54f, 0.55f, 1.00f));

                    const char* listTabs[getDrivingWindowCount()];
                    for (sofa::Index i=0; i<getDrivingWindowCount(); i++)
                        listTabs[i] = getDrivingWindowName(DrivingWindow(i));

                    if(ImGui::Combo("##DrivingWindowViewport", &dw, listTabs, IM_ARRAYSIZE(listTabs)))
                        m_ws_drivingWindow = dw;
                    ImGui::PopStyleColor(3);
                    ImGui::PopItemWidth();
                    ImGui::SetItemTooltip("Choose a window to drive the TCP target");
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}

void ViewportWindow::addSimulationTimeAndFPS()
{
    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &isOpen()))
        {
            if(ImGui::BeginChild("Render"))
            {
                const ImGuiIO& io = ImGui::GetIO();

                // Time
                auto position = ImGui::GetWindowWidth() - ImGui::CalcTextSize("Time: 000.000").x - ImGui::GetStyle().ItemSpacing.x;
                ImGui::SetCursorPosX(position);
                ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing());
                ImGui::PushStyleColor(ImGuiCol_Text, COLOR_WHITE);
                auto groot = m_baseGUI->getRootNode();
                ImGui::Text("Time: %.3f", groot->getTime());
                ImGui::PopStyleColor();
                ImGui::SetItemTooltip("Total time simulated");

                // FPS
                if (groot->animate_.getValue())
                    m_fps = io.Framerate;

                if (m_fps > 0)
                {
                    position -= ImGui::CalcTextSize("100.0 FPS ").x;
                    ImGui::SetCursorPosX(position);
                    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing());
                    ImGui::PushStyleColor(ImGuiCol_Text, COLOR_WHITE);
                    ImGui::Text("%.1f FPS", m_fps);
                    ImGui::PopStyleColor();
                    ImGui::SetItemTooltip("FPS: Frame Per Second \n Average %.2f ms per frame (%.1f FPS) \n GUI Average %.2f ms per frame (%.1f FPS)",
                                          1000.0f / m_fps, m_fps, 1000.0f / io.Framerate, io.Framerate);
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}

void ViewportWindow::addRecordingStatus(const ImVec4& red)
{
    if (isOpen())
    {
        if (ImGui::Begin(getLabel().c_str(), &isOpen()))
        {
            if(ImGui::BeginChild("Render"))
            {
                // Recording
                std::string icon = ICON_FA_CIRCLE_DOT;
                std::string text = " Recording";
                auto position = ImGui::GetWindowWidth() - ImGui::CalcTextSize((icon+text).c_str()).x - ImGui::GetFrameHeight() * 0.5f;
                ImGui::SetCursorPosX(position);
                ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y);
                ImGui::PushStyleColor(ImGuiCol_Text, red);
                ImGui::Text("%s", icon.c_str());
                ImGui::PopStyleColor();
                ImGui::SameLine(0.f, 0.f);
                ImGui::PushStyleColor(ImGuiCol_Text, COLOR_WHITE);
                ImGui::Text("%s", text.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}

}

