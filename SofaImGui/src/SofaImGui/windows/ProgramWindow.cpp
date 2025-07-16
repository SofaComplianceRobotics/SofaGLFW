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

#include <SofaImGui/windows/ProgramWindow.h>
#include <SofaImGui/models/actions/Action.h>
#include <SofaImGui/Utils.h>
#include <SofaImGui/widgets/Buttons.h>

#include <SofaImGui/models/actions/Move.h>
#include <SofaImGui/models/actions/Pick.h>
#include <SofaImGui/models/actions/Wait.h>
#include <SofaImGui/models/modifiers/Repeat.h>

#include <sofa/helper/system/FileSystem.h>
#include <sofa/helper/Utils.h>

#include <imgui_internal.h>
#include <GLFW/glfw3.h>

#include <nfd.h>
#include <filesystem>
#include <IconsFontAwesome6.h>

#include <ProgramStyle.h>
#include <SofaImGui/FooterStatusBar.h>


namespace sofaimgui::windows {

using sofa::type::Vec3;
using sofa::type::Quat;

ProgramWindow::ProgramWindow(const std::string& name,
                             const bool& isWindowOpen)
{
    m_name = name;
    m_isOpen = isWindowOpen;
}

void ProgramWindow::showWindow(sofaglfw::SofaGLFWBaseGUI *baseGUI,
                               const ImGuiWindowFlags& windowFlags)
{
    if (enabled() && isOpen())
    {
        if (baseGUI)
            m_baseGUI = baseGUI;
        else
            return;

        ProgramSizes().TrackMaxHeight = ImGui::GetFrameHeightWithSpacing() * 4.55;
        ProgramSizes().TrackMinHeight = ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.y * 2.;
        static bool firstTime = true;
        if (firstTime)
        {
            firstTime = false;
            ProgramSizes().TrackHeight = ProgramSizes().TrackMaxHeight;
        }
        ProgramSizes().InputWidth = ImGui::CalcTextSize("10000").x;
        ProgramSizes().AlignWidth = ImGui::CalcTextSize("iterations    ").x;
        
        if (ImGui::Begin(m_name.c_str(), &m_isOpen,
                         windowFlags | ImGuiWindowFlags_AlwaysAutoResize
                         ))
        {
            showProgramButtons();

            float width = ImGui::GetWindowWidth();
            float height = ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() * 3.;
            static const float defaultZoomCoef = 6.5;
            static float zoomCoef = defaultZoomCoef;
            static float minSize = ImGui::GetFrameHeight() * 1.5;
            ProgramSizes().TimelineOneSecondSize = zoomCoef * minSize;
            ProgramSizes().StartMoveBlockSize = defaultZoomCoef * minSize;
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_WindowBg));

            if (ImGui::BeginChild(ImGui::GetID(m_name.c_str()), ImVec2(width, height), ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
            {
                ImGui::PopStyleColor();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));

                if (m_timeBasedDisplay)
                    showTimeline();
                else // Keep the space the timeline would have taken, empty
                {
                    ImGui::NewLine();
                    ImGui::NewLine();
                }

                int nbCollaspedTracks = showTracks();

                if (m_timeBasedDisplay)
                    showCursorMarker(nbCollaspedTracks);
                else // Keep the space the cursor marker would have taken, empty
                    ImGui::NewLine();

                ImGui::PopStyleVar();
            }
            else
            {
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();

            if (m_timeBasedDisplay)
            {
                if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
                    zoomCoef += ImGui::GetIO().MouseWheel * 0.4f;
                float coefMax = 20.f;
                zoomCoef = (zoomCoef < 1)? 1 : zoomCoef;
                zoomCoef = (zoomCoef > coefMax)? coefMax : zoomCoef;
            }
            else
                zoomCoef = defaultZoomCoef;
        }
        ImGui::End();
    }
}

void ProgramWindow::showProgramButtons()
{
    ImVec2 buttonSize(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
    auto positionRight = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - buttonSize.x * 4 - ImGui::GetStyle().ItemSpacing.y * 5.5; // Get position for right buttons
    auto positionMiddle = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x / 2.f; // Get position for middle button

            // Left buttons
    if (ImGui::Button(ICON_FA_FOLDER_OPEN, buttonSize))
    {
        importProgram();
    }
    ImGui::SetItemTooltip("Import program");

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_ARROW_UP_FROM_BRACKET, buttonSize))
    {
        exportProgram();
    }
    ImGui::SetItemTooltip("Export program");

            // Middle button
    ImGui::SameLine();
    ImGui::SetCursorPosX(positionMiddle); // Set position to middle of the header

    if (!isDrivingSimulation())
        ImGui::BeginDisabled();

    if (ImGui::Button("Restart"))
    {
        auto groot = m_baseGUI->getRootNode().get();
        groot->setTime(0.);
        m_time = 0.f;

        for (const auto& track: m_program.getTracks())
        {
            const auto& modifiers = track->getModifiers();
            for (const auto& modifier: modifiers)
                modifier->reset();
        }
    }
    ImGui::SetItemTooltip("Restart the program");

    if (!isDrivingSimulation())
        ImGui::EndDisabled();

            // Right pushed buttons
    ImGui::SameLine();
    ImGui::SetCursorPosX(positionRight); // Set position to right of the header

    ImGui::LocalPushButton(ICON_FA_CLOCK"##TimeBasedDisplay", &m_timeBasedDisplay, buttonSize);
    ImGui::SetItemTooltip("Display blocks based on simulation time");

    ImGui::SameLine();

    ImGui::LocalPushButton(ICON_FA_DRAW_POLYGON"##Draw", &m_drawTrajectory, buttonSize);
    ImGui::SetItemTooltip("Draw trajectory");

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    ImGui::LocalPushButton(ICON_FA_REPEAT"##Repeat", &m_repeat, buttonSize);
    ImGui::SetItemTooltip("Repeat program");
    if (m_repeat)
        m_reverse = false;

    ImGui::SameLine();

    ImGui::LocalPushButton(ICON_FA_ARROWS_LEFT_RIGHT"##Reverse", &m_reverse, buttonSize);
    ImGui::SetItemTooltip("Reverse and repeat program");
    if (m_reverse)
        m_repeat = false;
}

void ProgramWindow::showCursorMarker(const int& nbCollaspedTracks)
{
    SOFA_UNUSED(nbCollaspedTracks); // todo: handle mutli tracks
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImVec4 color(0.95f, 0.f, 0.f, 1.0f);

    float thicknessRect = 1.0f;
    const float borderSize = ImGui::GetWindowWidth() * 1.f / 8.f;
    float widthTri = ImGui::GetStyle().ItemInnerSpacing.x * 2;

    m_cursorPos = m_time * ProgramSizes().TimelineOneSecondSize;

    // On animate, follow the cursor marker
    if (m_baseGUI->getRootNode()->getAnimate() && m_isDrivingSimulation)
    {
        float step = m_cursorPos - (ImGui::GetWindowContentRegionMax().x + ImGui::GetScrollX() - borderSize - m_trackBeginPos.x);
        if (step > 0)
            ImGui::SetScrollX(ImGui::GetScrollX() + step * 0.1f);

        step = m_cursorPos + (ImGui::GetWindowContentRegionMin().x + borderSize);
        if (step < 0)
            ImGui::SetScrollX(ImGui::GetScrollX() + step * 0.1f);
    }

    double max = ImGui::GetWindowWidth() + ImGui::GetScrollX();
    ImRect grab_bb(ImVec2(m_trackBeginPos.x + m_cursorPos - widthTri / 2., m_trackBeginPos.y - widthTri),
                   ImVec2(m_trackBeginPos.x + m_cursorPos + widthTri / 2., m_trackBeginPos.y));
    const ImRect frame_bb(ImVec2(m_trackBeginPos.x - widthTri / 2., m_trackBeginPos.y - widthTri),
                          ImVec2(m_trackBeginPos.x + max, m_trackBeginPos.y));

    ImVec2 p0Rect(grab_bb.Min.x + widthTri / 2., grab_bb.Min.y);
    ImVec2 p1Rect(p0Rect.x + thicknessRect,
                  m_trackBeginPos.y + ProgramSizes().TrackHeight * m_program.getNbTracks() + ImGui::GetStyle().ItemSpacing.y * (m_program.getNbTracks() - 1)); // TODO: case multiple tracks

    ImVec2 p0Tri(p0Rect.x + thicknessRect / 2.f, grab_bb.Max.y);
    ImVec2 p1Tri(p0Tri.x - widthTri / 2.f, p0Tri.y - widthTri);
    ImVec2 p2Tri(p0Tri.x + widthTri / 2.f,  p0Tri.y - widthTri);

    ImGui::ItemSize(ImVec2(widthTri, widthTri));
    const ImGuiID id = ImGui::GetID("##cursormarker");
    if (!ImGui::ItemAdd(frame_bb, id))
        return;

    // On drag, follow the cursor marker
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        ImGuiIO& io = ImGui::GetIO();

        const float xMax = ImGui::GetWindowContentRegionMax().x + ImGui::GetScrollX() - borderSize;
        const float xMin = ImGui::GetWindowContentRegionMin().x + ImGui::GetScrollX() + borderSize;

        if (io.MousePos.x > xMax)
            ImGui::SetScrollX(ImGui::GetScrollX() + (io.MousePos.x - xMax) * 0.1f);

        if (io.MousePos.x < xMin)
            ImGui::SetScrollX(ImGui::GetScrollX() - (xMin - io.MousePos.x) * 0.1f);
    }

    ImGuiContext& g = *GImGui;
    const bool hovered = ImGui::ItemHoverable(frame_bb, id, g.LastItemData.ItemFlags);
    const bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left, ImGuiInputFlags_None, id);
    const bool make_active = (clicked || g.NavActivateId == id);
    if (clicked)
    {
        ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
    }

    if (make_active)
    {
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    double min = 0;
    const bool value_changed = ImGui::SliderBehavior(frame_bb, id, ImGuiDataType_Double,
                                                     &m_cursorPos, &min, &max, "%0.2f", ImGuiSliderFlags_NoInput, &grab_bb);
    if (value_changed)
    {
        ImGui::MarkItemEdited(id);
        const auto& groot = m_IPController->getRootNode().get();
        m_time = m_cursorPos / ProgramSizes().TimelineOneSecondSize;
        groot->setTime(m_time);
        stepProgram();
    }

    window->DrawList->AddTriangleFilled(p0Tri, p1Tri, p2Tri, ImGui::GetColorU32(color));
    window->DrawList->AddRectFilled(p0Rect, p1Rect, ImGui::GetColorU32(color), 1.0f);
}

void ProgramWindow::showTimeline()
{
    float width = ImGui::GetWindowWidth() + ImGui::GetScrollX();
    int nbSteps = width / ProgramSizes().TimelineOneSecondSize + 1;

    float indentSize = ImGui::GetFrameHeight();
    ImGui::Indent(indentSize);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImRect frame_bb(ImVec2(m_trackBeginPos.x, m_trackBeginPos.y - ImGui::GetFrameHeight() * 1.5),
                          ImVec2(m_trackBeginPos.x + width, m_trackBeginPos.y));
    const ImGuiID id = ImGui::GetID("##timeline");
    if (!ImGui::ItemAdd(frame_bb, id))
        return;
    ImGui::SetItemTooltip("Simulation time");

    ImGui::BeginGroup(); // Timeline's number (seconds)
    window->DC.CursorPos.x = m_trackBeginPos.x;
    for (int i=0 ; i<nbSteps; i++)
    {
        std::string text = std::to_string(i) + " s";
        float textSize = ImGui::CalcTextSize(text.c_str()).x;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ProgramSizes().TimelineOneSecondSize - textSize, 0.f));
        ImGui::Text("%s", text.c_str());
        ImGui::SameLine();
        ImGui::PopStyleVar();
    }
    ImGui::EndGroup();

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGui::BeginGroup(); // Timeline's lines
    ImGui::NewLine();
    ImGui::SameLine();
    window->DC.CursorPos.x = m_trackBeginPos.x;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ProgramSizes().TimelineOneSecondSize / 10, 0.f));
    float height = ImGui::GetFrameHeight() / 3.f;
    float heightSpace = height / 4.f;
    float y1 = window->DC.CursorPos.y;
    float y1Space = window->DC.CursorPos.y + heightSpace;
    float y2 = window->DC.CursorPos.y + height;

    for (int i=0 ; i<nbSteps; i++)
    {
        for (int j=0; j<10; j++)
        {
            ImVec4 color = (j==0) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : ImVec4(0.5f, 0.5f, 0.5f, 1.f);

            drawList->AddLine(ImVec2(window->DC.CursorPos.x, (j==0)? y1: y1Space), ImVec2(window->DC.CursorPos.x, y2), ImGui::GetColorU32(color));
            ImGui::Spacing();
            ImGui::SameLine();
        }
    }
    ImGui::PopStyleVar();
    ImGui::EndGroup();

    ImGui::Unindent(indentSize);
}

int ProgramWindow::showTracks()
{
    const auto& tracks = m_program.getTracks();

    int trackIndex = 0;
    int nbCollapsedTrack = 0;

    for (const auto& track: tracks)
    {
        // Track options menu
        std::string menuLabel = "##TrackMenu" + std::to_string(trackIndex);
        if (ImGui::BeginPopup(menuLabel.c_str()))
        {
            if (ImGui::MenuItem(("Clear track##" + std::to_string(trackIndex)).c_str()))
            {
                track->clear();
            }
            if (ImGui::MenuItem(("Add track##" + std::to_string(trackIndex)).c_str(), nullptr, false, false))
            {
                m_program.addTrack(std::make_shared<models::Track>(m_IPController));
            }
            if (ImGui::MenuItem(("Remove track##" + std::to_string(trackIndex)).c_str(), nullptr, false, (trackIndex>0)? true : false))
            {
                m_program.removeTrack(trackIndex--);
            }

            ImGui::Separator();

            if (ImGui::BeginMenu(("Add action##" + std::to_string(trackIndex)).c_str()))
            {
                showActionMenu(track, trackIndex, track->getActions().size());
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(("Add modifier##" + std::to_string(trackIndex)).c_str()))
            {
                if (track->getActions().empty() || !track->getModifiers().empty()) // TODO: handle showing multiple modifiers
                    ImGui::BeginDisabled();

                if (ImGui::MenuItem(("Repeat##" + std::to_string(trackIndex)).c_str()))
                {
                    std::shared_ptr<models::modifiers::Repeat> repeat = std::make_shared<models::modifiers::Repeat>(1, 0);
                    repeat->pushToTrack(track);
                }

                if (track->getActions().empty() || !track->getModifiers().empty())
                    ImGui::EndDisabled();

                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        bool collapsed = showTrackButtons(trackIndex, menuLabel.c_str());
        if (collapsed)
        {
            nbCollapsedTrack++;
        }

        ImGui::SameLine();
        m_trackBeginPos = ImGui::GetCurrentWindow()->DC.CursorPos;
        m_trackBeginPos.x += ProgramSizes().StartMoveBlockSize;
        showBlocks(track, trackIndex);

        float x = ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.x ;
        float y = ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.y ;

        { // Empty track background
            ImGui::SameLine();
            std::string trackLabel = "##Track" + std::to_string(trackIndex) + "Empty";
            ImVec2 size(ImGui::GetWindowWidth() + ImGui::GetScrollX(), ProgramSizes().TrackHeight);

            float x = ImGui::GetCurrentWindow()->DC.CursorPos.x ;
            float y = ImGui::GetCurrentWindow()->DC.CursorPos.y ;
            ImRect bb(ImVec2(x, y), ImVec2(x + size.x, y + size.y));

            ImGui::ItemSize(size);
            if (ImGui::ItemAdd(bb, ImGui::GetID(trackLabel.c_str())))
            { // Backgroung
                ImGui::GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max,
                                                          ImGui::GetColorU32(ProgramColors().EmptyTrackBg),
                                                          ImGui::GetStyle().FrameRounding,
                                                          ImDrawFlags_None);
            }
        }

        const std::vector<std::shared_ptr<models::actions::Action>> &actions = track->getActions();
        showAddActionButton(ImVec2(x + ImGui::GetStyle().ItemSpacing.x, y + ProgramSizes().TrackHeight / 2.f), actions.size(), track, trackIndex);

        trackIndex++;
    }

    return nbCollapsedTrack;
}

bool ProgramWindow::showTrackButtons(const int &trackIndex, const char* const menuLabel)
{
    static bool collapsed = false;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 size(ImGui::GetFrameHeight(), ProgramSizes().TrackHeight);

    float x = window->DC.CursorPos.x ;
    float y = window->DC.CursorPos.y ;

    ImRect bb(ImVec2(x, y), ImVec2(x + size.x, y + size.y));
    ImVec2 topRight = ImVec2(x + size.x, y);

    std::string label = "##TrackButtons" + std::to_string(trackIndex);
    ImGui::ItemSize(size);
    const ImGuiID id = ImGui::GetID(label.c_str());
    if (!ImGui::ItemAdd(bb, id))
        return collapsed;

    { // Block backgroung
        drawList->AddRectFilled(bb.Min, bb.Max,
                                ImGui::GetColorU32(ImGuiCol_Header),
                                ImGui::GetStyle().FrameRounding,
                                ImDrawFlags_None);
    }

    window->DC.CursorPos.x = x;
    window->DC.CursorPos.y = y;
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_Header)); // Color of track button
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_Header)); // Color of track button
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_Header)); // Color of track button

    if (!collapsed || ProgramSizes().TrackHeight > ProgramSizes().TrackMinHeight) // hide the button at the end of the collapsing animation
    {
        std::string optionlabel = ICON_FA_BARS"##TrackOption" + std::to_string(trackIndex);
        if(ImGui::Button(optionlabel.c_str(), ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
        {
            ImGui::OpenPopup(menuLabel);
        }
    }

    window->DC.CursorPos.x = x;
    window->DC.CursorPos.y = y + (collapsed? (ProgramSizes().TrackHeight - ImGui::GetFrameHeight()) / 2.f :
                                      ProgramSizes().TrackHeight - ImGui::GetFrameHeight()) ;
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5, 1)); // Align icon down middle

    std::string collapselabel = "##TrackCollapse" + std::to_string(trackIndex);
    std::vector<std::string> icons{ICON_FA_COMPRESS, ICON_FA_EXPAND};
    static std::string icon = icons[collapsed];
    ImGui::Button((icon + collapselabel).c_str(), ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
    if (ImGui::IsItemClicked())
    {
        collapsed = !collapsed;
    }

    // Animate collapse
    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID((icon + collapselabel).c_str()))
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        ProgramSizes().TrackHeight = collapsed ? ProgramSizes().TrackMinHeight * t_anim + ProgramSizes().TrackMaxHeight * (1 - t_anim) :
                                                 ProgramSizes().TrackMinHeight * (1 - t_anim) + ProgramSizes().TrackMaxHeight * t_anim;
        if (t_anim >= 1) // change the icon at the end of animation
            icon = icons[collapsed];
    }

    ImGui::SetItemTooltip(collapsed? "Expend track": "Collapse track");
    ImGui::PopStyleVar(); // End align icon down middle
    ImGui::PopStyleColor(3); // Color of track button

    window->DC.CursorPosPrevLine.x = topRight.x;
    window->DC.CursorPosPrevLine.y = topRight.y;

    return collapsed;
}

void ProgramWindow::showBlocks(std::shared_ptr<models::Track> track,
                               const int& trackIndex)
{
    float blockHeight = ProgramSizes().TrackHeight;

    // StartMove block
    {
        std::shared_ptr<models::actions::StartMove> startmove = track->getStartMove();
        std::string blockLabel = "##StartMove" + std::to_string(trackIndex);
        std::string menuLabel = std::string("##OptionsMenu" + blockLabel);

        ImGui::SameLine();

        float blockWidth = ProgramSizes().StartMoveBlockSize;
        ImVec2 blockSize(blockWidth, blockHeight);

        if (ImGui::BeginPopup(menuLabel.c_str()))
        {
            if (ImGui::BeginMenu("Add after"))
            {
                showActionMenu(track, trackIndex, 0);
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Overwrite waypoint"))
            {
                startmove->setWaypoint(m_IPController->getTCPTargetPosition());
                track->updateNextMoveInitialPoint(-1, startmove->getWaypoint());
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        if (startmove->getView()->showBlock(blockLabel, blockSize))
        {
            track->updateNextMoveInitialPoint(-1, startmove->getWaypoint());
        }
        showBlockOptionButton(menuLabel, blockLabel);
    }

    // Modifiers blocks
    float x = ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.x ;
    float y = ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.y ;
    const std::vector<std::shared_ptr<models::modifiers::Modifier>> &modifiers = track->getModifiers();

    sofa::Index modifierIndex = 0;
    while(modifierIndex < modifiers.size())
    {
        std::shared_ptr<models::modifiers::Modifier> modifier = modifiers[modifierIndex];
        float blockWidth = modifier->getDuration() * ProgramSizes().TimelineOneSecondSize - ImGui::GetStyle().ItemSpacing.x;
        std::string blockLabel = "##Modifier" + std::to_string(trackIndex) + std::to_string(modifierIndex);
        std::string menuLabel = std::string("##OptionsMenu" + blockLabel);
        ImGui::SameLine();

        modifier->getView()->showBlock(blockLabel, ImVec2(blockWidth, blockHeight), m_trackBeginPos);

        if (ImGui::BeginPopup(menuLabel.c_str()))
        {
            if (ImGui::MenuItem("Delete"))
            {
                modifier->deleteFromTrack(track, modifierIndex);
            }
            else
                modifierIndex++;
            ImGui::EndPopup();
        } else {
            modifierIndex++;
        }

        if (blockWidth > ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 2.0f)
            showBlockOptionButton(menuLabel, blockLabel);
    }

    ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.x = x;
    ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.y = y;

    // Action blocks
    const std::vector<std::shared_ptr<models::actions::Action>> &actions = track->getActions();
    sofa::Index actionIndex = 0;
    while(actionIndex < actions.size())
    {
        std::shared_ptr<models::actions::Action> action = actions[actionIndex];
        float blockWidth = (m_timeBasedDisplay? action->getDuration(): 1.f) * ProgramSizes().TimelineOneSecondSize - ImGui::GetStyle().ItemSpacing.x;
        std::string blockLabel = "##Action" + std::to_string(trackIndex) + std::to_string(actionIndex);
        std::string menuLabel = std::string("##OptionsMenu" + blockLabel);
        ImGui::SameLine();

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        float x = window->DC.CursorPos.x ;
        float y = window->DC.CursorPos.y ;
        ImVec2 blockSize(blockWidth, blockHeight);

        if (ImGui::BeginPopup(menuLabel.c_str()))
        {
            if (ImGui::MenuItem("Duplicate"))
            {
                action->duplicate()->insertInTrack(track, actionIndex + 1);
            }
            if (ImGui::BeginMenu("Replace"))
            {
                if (showActionMenu(track, trackIndex, actionIndex+1))
                {
                    action->deleteFromTrack(track, actionIndex);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Add before"))
            {
                showActionMenu(track, trackIndex, actionIndex);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Add after"))
            {
                showActionMenu(track, trackIndex, actionIndex+1);
                ImGui::EndMenu();
            }

            ImGui::Separator();
            ImGui::EndPopup();
        }

        std::shared_ptr<models::actions::Move> move = std::dynamic_pointer_cast<models::actions::Move>(action);
        if (move)
        {
            move->setDrawTrajectory(m_drawTrajectory);
            if(move->getView()->showBlock(blockLabel, blockSize))
            {
                track->updateNextMoveInitialPoint(actionIndex, move->getWaypoint());
            }
            // Options menu
            if (ImGui::BeginPopup(menuLabel.c_str()))
            {
                if (ImGui::MenuItem("Overwrite waypoint"))
                {
                    move->setWaypoint(m_IPController->getTCPTargetPosition());
                    track->updateNextMoveInitialPoint(actionIndex, move->getWaypoint());
                }
                ImGui::Separator();
                ImGui::EndPopup();
            }
        } else {
            action->getView()->showBlock(blockLabel, blockSize);
        }

        if (ImGui::BeginPopup(menuLabel.c_str()))
        {
            if (ImGui::MenuItem("Delete"))
            {
                action->deleteFromTrack(track, actionIndex);
            }
            else
                actionIndex++;
            ImGui::EndPopup();
        } else {
            actionIndex++;
        }

        if (blockWidth > ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 2.0f)
            showBlockOptionButton(menuLabel, blockLabel);

        showAddActionButton(ImVec2(x, y + blockHeight / 2.f), actionIndex - 1, track, trackIndex);
    }
}

void ProgramWindow::showAddActionButton(const ImVec2 &position,
                                        const unsigned int &actionIndex,
                                        std::shared_ptr<models::Track> track,
                                        const int& trackIndex)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    auto backuppos = window->DC.CursorPosPrevLine;

    const float buttonSize = ImGui::GetFrameHeight();
    const float x = position.x - ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x / 2.f;
    const float y = position.y - ImGui::GetFrameHeight();

    window->DC.CursorPos.x = x;
    window->DC.CursorPos.y = y;

    ImRect bb{ImVec2(x, position.y - buttonSize),
              ImVec2(x + buttonSize * 2.f, position.y + buttonSize)};

    ImGui::PushID(actionIndex);
    const ImGuiID id = window->GetID("##InvisibleActionBlockAddButtons");
    ImGui::ItemAdd(bb, id);

    window->DC.CursorPos.x = position.x - ImGui::GetFrameHeight() / 2.f - ImGui::GetStyle().ItemSpacing.x / 2.f;
    window->DC.CursorPos.y = position.y - ImGui::GetFrameHeight() / 2.f;

    const std::string menulabel = "##ActionBlockAddButtonsMenu";
    const std::string buttonlabel = ICON_FA_PLUS"##ActionBlockAddButtons";
    if (ImGui::BeginPopup(menulabel.c_str()))
    {
        showActionMenu(track, trackIndex, actionIndex);
        ImGui::EndPopup();
    }

    size_t nbActions = track->getActions().size();
    if (ImGui::IsItemHovered() || ImGui::IsPopupOpen(menulabel.c_str()) || actionIndex == nbActions)
    {
        ImGui::Button(buttonlabel.c_str(), ImVec2(buttonSize, buttonSize));
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            ImGui::OpenPopup(menulabel.c_str());
        }
    }

    ImGui::PopID();

    window->DC.CursorPosPrevLine = backuppos;
}

void ProgramWindow::showBlockOptionButton(const std::string &menulabel,
                                           const std::string &label)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    auto backuppos = window->DC.CursorPosPrevLine;

    float ysize = ImGui::CalcTextSize(ICON_FA_BARS).y + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImVec2 buttonSize(ysize, ysize);
    window->DC.CursorPos = window->DC.CursorPosPrevLine;
    window->DC.CursorPos.x -= buttonSize.x + ImGui::GetStyle().FramePadding.x;
    window->DC.CursorPos.y += ImGui::GetStyle().FramePadding.y;
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.05f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    std::string buttonlabel = ICON_FA_BARS;
    buttonlabel += "##" + label;
    if (ImGui::Button(buttonlabel.c_str(), buttonSize))
    {
        ImGui::OpenPopup(menulabel.c_str());
    }
    ImGui::PopStyleColor(3);

    window->DC.CursorPosPrevLine = backuppos;
}

bool ProgramWindow::showActionMenu(std::shared_ptr<models::Track> track, const int &trackIndex, const int &actionIndex)
{
    if (ImGui::MenuItem(("Move##" + std::to_string(trackIndex)).c_str()))
    {
        auto move = std::make_shared<models::actions::Move>(RigidCoord(),
                                                            m_IPController->getTCPTargetPosition(),
                                                            models::actions::Action::DEFAULTDURATION,
                                                            m_IPController,
                                                            true,
                                                            models::actions::Move::Type::LINE);
        move->insertInTrack(track, actionIndex);
        return true;
    }

    if (models::actions::Pick::gripperInstalled && ImGui::MenuItem(("Pick##" + std::to_string(trackIndex)).c_str()))
    {
        auto pick = std::make_shared<models::actions::Pick>();
        pick->insertInTrack(track, actionIndex);
        return true;
    }

    if (models::actions::Pick::gripperInstalled && ImGui::MenuItem(("Place##" + std::to_string(trackIndex)).c_str()))
    {
        auto pick = std::make_shared<models::actions::Pick>(models::actions::Action::DEFAULTDURATION, true);
        pick->setComment("Place");
        pick->insertInTrack(track, actionIndex);
        return true;
    }

    if (ImGui::MenuItem(("Wait##" + std::to_string(trackIndex)).c_str()))
    {
        auto wait = std::make_shared<models::actions::Wait>();
        wait->insertInTrack(track, actionIndex);
        return true;
    }

    return false;
}

void ProgramWindow::initFilePath(const std::string& filename)
{
    const std::string extension = m_program.getExtension();

    std::filesystem::path absFilename;
    if (!filename.empty())
        absFilename = std::filesystem::absolute(filename);

    if (m_programDirPath.empty())
    {
        if (!absFilename.empty() && sofa::helper::system::FileSystem::exists(absFilename.parent_path().string()))
        {
            m_programDirPath = absFilename.parent_path().string();
        }
        else
        {
            m_programDirPath = sofa::helper::Utils::getSofaUserLocalDirectory();
        }
    }

    if (m_programFilename.empty())
    {
        if (!absFilename.empty())
        {
            std::filesystem::path path(absFilename);
            path = path.replace_extension(extension);
            m_programFilename = path.filename().string();
        }
        else
        {
            m_programFilename = "output" + extension;
        }
    }
}

void ProgramWindow::importProgram()
{
    bool successfulImport = false;
    nfdchar_t *outPath;
    std::vector<nfdfilteritem_t> nfd_filters;
    nfd_filters.push_back({"program file", "crprog"});
    std::filesystem::path path;
    initFilePath(m_baseGUI->getFilename());

    nfdresult_t result = NFD_OpenDialog(&outPath, nfd_filters.data(), nfd_filters.size(), (m_programDirPath.empty()) ? nullptr : m_programDirPath.c_str());
    if (result == NFD_OKAY)
    {
        if (sofa::helper::system::FileSystem::exists(outPath))
        {
            successfulImport = m_program.importProgram(outPath);
            path = outPath;
        }
        NFD_FreePath(outPath);
    }
    else if (result == NFD_ERROR) {
        FooterStatusBar::getInstance().setTempMessage("Import failed to proceed.", FooterStatusBar::MERROR);
    }

    if (successfulImport)
    {
        m_programDirPath = path.parent_path().string(); // store chosen dir path
        m_programFilename = path.filename().string(); // store chosen filename

        FooterStatusBar::getInstance().setTempMessage("Imported program [" + path.string() + "]");
    }
}

void ProgramWindow::exportProgram(const bool &exportAs)
{
    nfdchar_t *outPath;
    std::vector<nfdfilteritem_t> nfd_filters;
    nfd_filters.push_back({"program file", "crprog"});
    const std::string extension = m_program.getExtension();
    initFilePath(m_baseGUI->getFilename());

    std::filesystem::path path;
    path = m_programDirPath;
    path.append(m_programFilename);
    bool doExport = true;

    if (exportAs)
    {
        nfdresult_t result = NFD_SaveDialog(&outPath, nfd_filters.data(), nfd_filters.size(), m_programDirPath.c_str(), m_programFilename.c_str());
        if (result == NFD_OKAY)
        {
            path = outPath;
            path = (!path.has_extension())? outPath + extension: outPath;

            m_programDirPath = path.parent_path().string(); // store chosen dir path
            m_programFilename = path.filename().string(); // store chosen filename

            NFD_FreePath(outPath);
        }
        else
        {
            doExport = false;
            if (result == NFD_ERROR) {
                FooterStatusBar::getInstance().setTempMessage("Export failed to proceed.", FooterStatusBar::MERROR);
            }
        }
    }

    if (doExport)
    {
        m_program.exportProgram(path.string());
        FooterStatusBar::getInstance().setTempMessage("Exported program [" + path.string() + "]");
    }
}

void ProgramWindow::stepProgram(const double &dt, const bool &reverse)
{
    if (m_isDrivingSimulation)
    {
        double eps = 1e-5;
        for (const auto& track: m_program.getTracks())
        {
            double blockEnd = 0;
            double blockStart = 0;
            const auto& actions = track->getActions();
            for (const auto& action: actions)
            {
                blockEnd += action->getDuration();
                if ((!reverse && (blockEnd - m_time) > eps) || (reverse && (blockEnd - m_time - dt) > eps))
                {
                    RigidCoord position = m_IPController->getTCPPosition();
                    if (action->apply(position, m_time + dt - blockStart)) // apply the time corresponding to the end of the time step
                    {
                        m_IPController->setTCPTargetPosition(position);
                    }
                    break;
                }
                blockStart = blockEnd;
            }
        }
    }
}

void ProgramWindow::animateBeginEvent(sofa::simulation::Node *groot)
{
    if (m_isDrivingSimulation)
    {
        if (m_program.isEmpty())
            return;

        double eps = 1e-5;
        static bool reverse = false;
        double dt = reverse? -groot->getDt(): groot->getDt();
        double programDuration = m_program.getDuration();

        for (const auto& track: m_program.getTracks()) // allow the mofifiers to do their jobs first
        {
            const auto& modifiers = track->getModifiers();
            for (const auto& modifier: modifiers)
            {
                modifier->modify(m_time);
                groot->setTime(m_time);
            }
        }

        if (groot->getTime() >= programDuration - eps) // if we've reached the end of the program
        {
            if (m_repeat) // start from beginning
            {
                groot->setTime(0.);

                for (const auto& track: m_program.getTracks())
                {
                    const auto& modifiers = track->getModifiers();
                    for (const auto& modifier: modifiers)
                        modifier->reset();
                }
            }
            else if (m_reverse)
            {
                reverse = true;
                dt = -groot->getDt();
            }
            else // nothing to do, exit
            {
                groot->setTime(programDuration);
                m_time = programDuration;
                return;
            }
        }

        if (reverse)
        {
            if (groot->getTime() <= eps)
            {
                reverse = false;
                dt = groot->getDt();
            }
        }

        m_time = groot->getTime(); // time at the beginning of the time step

        stepProgram(dt, reverse);

        m_time += dt; // for cursor display
    } // isDrivingSimulation
}

void ProgramWindow::animateEndEvent(sofa::simulation::Node *groot)
{
    SOFA_UNUSED(groot);
    if (m_isDrivingSimulation)
        groot->setTime(m_time);
}

void ProgramWindow::setIPController(models::IPController::SPtr IPController)
{
    m_IPController = IPController;
    if (m_IPController)
        m_program = models::Program(IPController);
}

void ProgramWindow::setDrivingTCPTarget(const bool &isDrivingSimulation)
{
    m_isDrivingSimulation=isDrivingSimulation;
}

} // namespace


























