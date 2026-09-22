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

#include "GUIColors.h"
#include <SofaImGui/windows/ProgramWindow.h>
#include <SofaImGui/Utils.h>
#include <SofaImGui/widgets/Widgets.h>

#include <SofaImGui/models/actions/Action.h>
#include <SofaImGui/models/actions/Custom.h>
#include <SofaImGui/models/actions/Move.h>
#include <SofaImGui/models/actions/Pick.h>
#include <SofaImGui/models/actions/Wait.h>

#include <sofa/helper/system/FileSystem.h>
#include <sofa/helper/Utils.h>

#include <imgui_internal.h>
#include <GLFW/glfw3.h>

#include <nfd.h>
#include <filesystem>
#include <IconsFontAwesome6.h>

#include <ProgramStyle.h>
#include <SofaImGui/FooterStatusBar.h>
#include <SofaImGui/windows/WindowsSettingsName.h>


namespace sofaimgui::windows {

using sofa::type::Vec3;
using sofa::type::Quat;

ProgramWindow::ProgramWindow(const std::string& name,
                             models::guidata::KinematicsGUIDataManager::SPtr kinematicsGUIDataManager)
    : BaseWindow(name)
{
    m_enabledWorkbenches = Workbench::LIVE_CONTROL | Workbench::SIMULATION_MODE;
    m_windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    m_kinematicsGUIDataManager = kinematicsGUIDataManager;
}

std::string ProgramWindow::getDescription()
{
    return "Create programs with simulation data.";
}

void ProgramWindow::clearWindow()
{
    if (m_program.isValid())
        m_program.clearTracks();
}

void ProgramWindow::onEndSimulationLoad()
{   
    if (m_program.isEmpty())
    {
        m_program = models::Program(m_kinematicsGUIDataManager);

        if (m_program.isValid())
        {
            if (!m_ws_programDirPath.empty() && !m_ws_programFilename.empty())
                importProgram(sofa::helper::system::FileSystem::append(m_ws_programDirPath, m_ws_programFilename));
        }
    }
}

void ProgramWindow::registerAndLoadWindowSettings()
{
    registerAndLoadWindowSetting(WS_PROGRAM_PROGRAMDIRPATH, m_ws_programDirPath, WindowsSettings::STRING);
    registerAndLoadWindowSetting(WS_PROGRAM_PROGRAMFILENAME, m_ws_programFilename, WindowsSettings::STRING);
    registerAndLoadWindowSetting(WS_PROGRAM_REPEAT, m_ws_repeat, WindowsSettings::BOOL);
    registerAndLoadWindowSetting(WS_PROGRAM_REVERSE, m_ws_reverse, WindowsSettings::BOOL);
    registerAndLoadWindowSetting(WS_PROGRAM_DRAWTRAJECTORY, m_ws_drawTrajectory, WindowsSettings::BOOL);
    registerAndLoadWindowSetting(WS_PROGRAM_TIMEBASEDDISPLAY, m_ws_timeBasedDisplay, WindowsSettings::BOOL);

    // Import program file if any
    if (!m_ws_programFilename.empty() && !m_ws_programDirPath.empty())
        importProgram(sofa::helper::system::FileSystem::append(m_ws_programDirPath, m_ws_programFilename));
}

void ProgramWindow::internalShowWindow()
{
    if (isEnabledByState())
    {
        ProgramSizes().BlockHeaderHeight = ImGui::GetStyle().FramePadding.y + ImGui::GetFrameHeight();
        ProgramSizes().TrackMaxHeight = ImGui::GetFrameHeightWithSpacing() * 3 + ProgramSizes().BlockHeaderHeight * 2;
        ProgramSizes().TrackMinHeight = ProgramSizes().BlockHeaderHeight * 2;
        static bool firstTime = true;
        if (firstTime)
        {
            firstTime = false;
            ProgramSizes().TrackHeight = ProgramSizes().TrackMaxHeight;
        }
        ProgramSizes().InputWidth = ImGui::CalcTextSize("10000").x;
        ProgramSizes().AlignWidth = ImGui::CalcTextSize("iterations    ").x;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y * 0.5));
        showProgramButtons();

        float width = ImGui::GetWindowWidth();
        float height = ImGui::GetContentRegionAvail().y;
        static const float defaultZoomCoef = 6.5;
        static float zoomCoef = defaultZoomCoef;
        static float minSize = ImGui::GetFrameHeight() * 1.5;
        ProgramSizes().TimelineOneSecondSize = zoomCoef * minSize;
        ProgramSizes().StartMoveBlockSize = defaultZoomCoef * minSize;

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_WindowBg));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        if (ImGui::BeginChild("Timeline", ImVec2(width, height), ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
        {
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.));
            if (m_ws_timeBasedDisplay)
                showTimeline();
            else // Keep the space the timeline would have taken, empty
            {
                ImGui::NewLine();
                ImGui::NewLine();
            }
            ImGui::PopStyleVar();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));
            int nbCollaspedTracks = showTracks();
            ImGui::PopStyleVar();

            if (m_ws_timeBasedDisplay)
                showCursorMarker(nbCollaspedTracks);
        }
        else
        {
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);
        }
        ImGui::EndChild();

        if (m_ws_timeBasedDisplay)
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
    else
    {
        showInfoMessage("This window is designed for programming a robot using action blocks arranged on time-based tracks. "
                        "The scene is missing elements for this window to work properly.");
    }
}

void ProgramWindow::showProgramButtons()
{
    auto positionRight = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - ImGui::GetFrameHeight() * 4 - ImGui::GetStyle().ItemSpacing.x * 5.5; // Get position for right buttons
    auto positionMiddle = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x / 2.f; // Get position for middle button

            // Left buttons
    if (sofaimgui::widgets::Button(ICON_FA_FILE_IMPORT))
    {
        importProgram();
    }
    ImGui::SetItemTooltip("Import program");

    ImGui::SameLine();

    if (sofaimgui::widgets::Button(ICON_FA_FILE_EXPORT))
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
        setTime(0);
    }
    ImGui::SetItemTooltip("Restart the program");

    if (!isDrivingSimulation())
        ImGui::EndDisabled();

            // Right pushed buttons
    ImGui::SameLine();
    ImGui::SetCursorPosX(positionRight); // Set position to right of the header

    sofaimgui::widgets::PushButton(ICON_FA_CLOCK"##TimeBasedDisplay", &m_ws_timeBasedDisplay);
    ImGui::SetItemTooltip("Display blocks based on simulation time");

    ImGui::SameLine();

    sofaimgui::widgets::PushButton(ICON_FA_DRAW_POLYGON"##Draw", &m_ws_drawTrajectory);
    ImGui::SetItemTooltip("Draw trajectory");

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    sofaimgui::widgets::PushButton(ICON_FA_REPEAT"##Repeat", &m_ws_repeat);
    ImGui::SetItemTooltip("Repeat program");
    if (m_ws_repeat)
        m_ws_reverse = false;

    ImGui::SameLine();

    sofaimgui::widgets::PushButton(ICON_FA_ARROWS_LEFT_RIGHT"##Reverse", &m_ws_reverse);
    ImGui::SetItemTooltip("Reverse and repeat program");
    if (m_ws_reverse)
        m_ws_repeat = false;
}

void ProgramWindow::showCursorMarker(const int& nbCollaspedTracks)
{
    SOFA_UNUSED(nbCollaspedTracks); // todo: handle mutli tracks
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    float thicknessRect = 1.0f;
    const float borderSize = ImGui::GetWindowWidth() * 1.f / 8.f;
    float widthTri = ImGui::GetStyle().ItemInnerSpacing.x * 2;

    m_cursorPos = m_time * ProgramSizes().TimelineOneSecondSize;

    // On animate, follow the cursor marker
    if (m_baseGUI->getRootNode()->getAnimate() && isDrivingSimulation())
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

    ImVec2 p0Rect(grab_bb.Min.x + widthTri / 2., grab_bb.Min.y);
    ImVec2 p1Rect(p0Rect.x + thicknessRect,
                  m_trackBeginPos.y + ProgramSizes().TrackHeight * m_program.getNbTracks() + ImGui::GetStyle().ItemSpacing.y * (m_program.getNbTracks() - 1)); // TODO: case multiple tracks

    ImVec2 p0Tri(p0Rect.x + thicknessRect / 2.f, grab_bb.Max.y);
    ImVec2 p1Tri(p0Tri.x - widthTri / 2.f, p0Tri.y - widthTri);
    ImVec2 p2Tri(p0Tri.x + widthTri / 2.f, p0Tri.y - widthTri);

    ImRect frame_bb(ImVec2(m_trackBeginPos.x - widthTri / 2., p0Rect.y),
                    ImVec2(m_trackBeginPos.x + max, p1Rect.y));

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
    frame_bb.Max.y = m_trackBeginPos.y;
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
        setTime(m_cursorPos / ProgramSizes().TimelineOneSecondSize);
        stepProgram();
    }

    window->DrawList->AddTriangleFilled(p0Tri, p1Tri, p2Tri, COLOR_RED);
    window->DrawList->AddRectFilled(p0Rect, p1Rect, COLOR_RED, 1.0f);
}

void ProgramWindow::showTimeline()
{
    float width = ImGui::GetWindowWidth() + ImGui::GetScrollX();
    int nbSteps = width / ProgramSizes().TimelineOneSecondSize + 1;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    window->DC.CursorPos.x = m_trackBeginPos.x;

    ImGui::BeginGroup(); // Timeline's number (seconds)
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
    ImGui::SetItemTooltip("Simulation time");

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
}

int ProgramWindow::showTracks()
{
    int nbCollapsedTrack = 0;

    { // Tracks
        const auto& tracks = m_program.getTracks();

        int trackIndex = 0;

        for (auto track: tracks)
        {
            ImGui::PushID(trackIndex);

            // Track options menu
            std::string menuLabel = "##TrackMenu" + std::to_string(trackIndex);

            bool collapsed = showTrackButtons(trackIndex, menuLabel.c_str());
            if (collapsed)
            {
                nbCollapsedTrack++;
            }

            ImGui::SameLine();

            if (trackIndex == 0)
            {
                m_trackBeginPos = ImGui::GetCurrentWindow()->DC.CursorPos;
                if (m_kinematicsGUIDataManager->hasInverseProblemSolverAndTCP())
                    m_trackBeginPos.x += ProgramSizes().StartMoveBlockSize;
            }
            else
            {
                ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.x += ProgramSizes().StartMoveBlockSize + ImGui::GetStyle().ItemSpacing.x;
            }

            float blockHeight = ProgramSizes().TrackHeight - ProgramSizes().BlockHeaderHeight;

            { // Show blocks
                showBlocks(track, trackIndex);
            }

            // Store position
            float x = ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.x ;
            float y = ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.y ;

            { // Empty track background
                ImGui::SameLine();
                std::string trackLabel = "##Track" + std::to_string(trackIndex) + "Empty";
                ImVec2 size(ImGui::GetWindowWidth() + ImGui::GetScrollX(), blockHeight);

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

            { // Show between blocks button
                const std::vector<models::actions::Action::SPtr> &actions = track->getActions();
                showBetweenBlocksButtons(ImVec2(x + ImGui::GetStyle().ItemSpacing.x, y + blockHeight / 2.f), actions.size(), track, trackIndex);
            }

            trackIndex++;

            ImGui::PopID();
        }
    }

    { // Tracks menu
        const auto& tracks = m_program.getTracks();
        int trackIndex = 0;
        for (auto track: tracks)
        {
            ImGui::PushID(trackIndex);

            // Track options menu
            std::string menuLabel = "##TrackMenu" + std::to_string(trackIndex);
            addTrackMenu(menuLabel, trackIndex, track);

            trackIndex++;

            ImGui::PopID();
        }
    }

    return nbCollapsedTrack;
}

bool ProgramWindow::showTrackButtons(const int &trackIndex, const char* const menuLabel)
{
    static bool collapsed = false;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 size(ImGui::GetFrameHeight() * 2, ProgramSizes().TrackHeight);

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

    { // Option button
        std::string optionlabel = ICON_FA_BARS"##TrackOption" + std::to_string(trackIndex);
        if(ImGui::Button(optionlabel.c_str(), ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
        {
            ImGui::OpenPopup(menuLabel);
        }
    }

    window->DC.CursorPos.x = x;
    window->DC.CursorPos.y = y + ProgramSizes().TrackHeight - ImGui::GetFrameHeightWithSpacing();
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5, 1)); // Align icon down middle

    std::string collapselabel = "##TrackCollapse" + std::to_string(trackIndex);
    std::vector<std::string> icons{ICON_FA_COMPRESS, ICON_FA_EXPAND};
    static std::string icon = icons[collapsed];

    { // Collapse button
        ImGui::Button((icon + collapselabel).c_str(), ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
        ImGui::SetItemTooltip(collapsed? "Expend track": "Collapse track");

        if (ImGui::IsItemClicked())
            collapsed = !collapsed;
    }

    { // Track Name
        ImGui::SameLine(0, 0);
        showTrackName(trackIndex);
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

    ImGui::PopStyleVar(); // End align icon down middle
    ImGui::PopStyleColor(3); // Color of track button

    window->DC.CursorPosPrevLine.x = topRight.x;
    window->DC.CursorPosPrevLine.y = topRight.y;

    return collapsed;
}

void ProgramWindow::showBlocks(models::Track::SPtr track, const int& trackIndex)
{
    float blockHeight = ProgramSizes().TrackHeight - ProgramSizes().BlockHeaderHeight;

    if (m_kinematicsGUIDataManager->hasInverseProblemSolverAndTCP() && trackIndex == 0)
        showStartMoveBlock(blockHeight, trackIndex, track);

    showActionBlocks(blockHeight, trackIndex, track);
}

void ProgramWindow::showStartMoveBlock(const float& blockHeight, const sofa::Index& trackIndex, models::Track::SPtr track)
{
    auto startmove = track->getStartMove();

    { // Start Move
        std::string blockLabel = "##StartMove" + std::to_string(trackIndex);
        std::string menuLabel = std::string("##OptionsMenu" + blockLabel);

        addStartMoveBlockMenu(menuLabel, trackIndex, track, startmove);

        ImGui::SameLine();
        float blockWidth = ProgramSizes().StartMoveBlockSize;
        ImVec2 blockSize(blockWidth, blockHeight);
        if (startmove->getView()->showBlock(blockLabel, blockSize))
        {
            track->updateNextMoveInitialPoint(-1, startmove->getWaypoint());
        }
        showBlockOptionButton(menuLabel, blockLabel);
    }
}

void ProgramWindow::showActionBlocks(const float& blockHeight, const sofa::Index& trackIndex, models::Track::SPtr track)
{
    const auto &actions = track->getActions();
    sofa::Index actionIndex = 0;

    bool actionSelected = false;
    static sofa::Index actionIndexContextMenu = 0;
    const std::string actionContextMenuLabel = "##ActionContextMenuLabel";

    while(actionIndex < actions.size())
    {
        ImGui::PushID(actionIndex);

        models::actions::Action::SPtr action = actions[actionIndex];
        bool isSelected = track->isActionSelected(actionIndex);
        bool openPopUp = false;

        float blockWidth = (m_ws_timeBasedDisplay? action->getDuration(): 1.f) * ProgramSizes().TimelineOneSecondSize - ImGui::GetStyle().ItemSpacing.x;
        std::string blockLabel = "##Action" + std::to_string(trackIndex) + std::to_string(actionIndex);
        std::string menuLabel = std::string("##OptionsMenu" + blockLabel);

        ImGui::SameLine();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        float x = window->DC.CursorPos.x ;
        float y = window->DC.CursorPos.y ;
        ImVec2 blockSize(blockWidth, blockHeight);

        { // Show group if any
            if (const int length = track->getGroupLength(action) > 0)
            {
            }
        }

        { // Show action block
            models::actions::Move::SPtr move = std::dynamic_pointer_cast<models::actions::Move>(action);
            if (move)
            {
                move->setDrawTrajectory(m_ws_drawTrajectory);
                if(move->getView()->showBlock(blockLabel, blockSize, isSelected))
                {
                    track->updateNextMoveInitialPoint(actionIndex, move->getWaypoint());
                }
            }
            else
            {
                action->getView()->showBlock(blockLabel, blockSize, isSelected);
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                openPopUp = true;

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                if (!m_program.isTrackSelected(trackIndex))
                {
                    m_program.clearTrackSelected();
                    m_program.setTrackSelected(trackIndex);
                }
                track->setActionSelected(actionIndex);
                actionSelected = true;
            }
        }

        { // Menu
            actionIndex = addActionBlockMenu(menuLabel, actionIndex, trackIndex, track, action);
            if (blockWidth > ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 2.0f)
                showBlockOptionButton(menuLabel, blockLabel);
        }

        { // Between blocks buttons
            showBetweenBlocksButtons(ImVec2(x, y + blockHeight / 2.f), actionIndex - 1, track, trackIndex);
        }

        ImGui::PopID();

        if (openPopUp)
        {
            ImGui::OpenPopup(actionContextMenuLabel.c_str());
            actionIndexContextMenu = actionIndex - 1;
        }
    }

    addActionsContextMenu(actionContextMenuLabel, actionIndexContextMenu);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !actionSelected)
        track->clearSelectedActions();
}

void ProgramWindow::showBetweenBlocksButtons(const ImVec2 &position,
                                            const unsigned int &actionIndex,
                                            models::Track::SPtr track,
                                            const int& trackIndex)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    auto backuppos = window->DC.CursorPosPrevLine;
    size_t nbActions = track->getActions().size();

    const float buttonSize = ImGui::GetFrameHeight();
    const float x = position.x - ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x / 2.f;
    const float y = position.y - ImGui::GetFrameHeight();

    window->DC.CursorPos.x = x;
    window->DC.CursorPos.y = y;

    ImRect bb{ImVec2(x, position.y - buttonSize),
              ImVec2(x + buttonSize * 2.f, position.y + buttonSize)};

    bool enableSwapButton = (actionIndex!=0 && actionIndex!=track->getActions().size());

    // Add button
    {
        ImGui::PushID(actionIndex);
        const ImGuiID idAddButton = window->GetID("##InvisibleActionBlockAddButtons");
        ImGui::ItemAdd(bb, idAddButton);

        window->DC.CursorPos.x = position.x - ImGui::GetFrameHeight() / 2.f - ImGui::GetStyle().ItemSpacing.x / 2.f;
        window->DC.CursorPos.y = position.y - ImGui::GetFrameHeight();
        if (!enableSwapButton)
            window->DC.CursorPos.y = position.y - ImGui::GetFrameHeight() / 2.f;

        const std::string menulabel = "##ActionBlockAddButtonsMenu";
        const std::string buttonlabel = ICON_FA_PLUS"##ActionBlockAddButtons";

        if (ImGui::BeginPopup(menulabel.c_str()))
        {
            addAddActionMenu(track, trackIndex, actionIndex);
            ImGui::EndPopup();
        }

        if (ImGui::IsItemHovered() || ImGui::IsPopupOpen(menulabel.c_str()) || actionIndex == nbActions)
        {
            ImGui::Button(buttonlabel.c_str(), ImVec2(buttonSize, buttonSize));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                ImGui::OpenPopup(menulabel.c_str());
            }
        }

        ImGui::PopID();
    }

    // Swap button
    if (enableSwapButton)
    {
        ImGui::PushID(actionIndex + nbActions);
        const ImGuiID idSwapButton = window->GetID("##InvisibleActionBlockSwapButtons");
        ImGui::ItemAdd(bb, idSwapButton);

        window->DC.CursorPos.x = position.x - ImGui::GetFrameHeight() / 2.f - ImGui::GetStyle().ItemSpacing.x / 2.f ;
        window->DC.CursorPos.y = position.y + ImGui::GetStyle().ItemSpacing.x / 2.f;

        const std::string menulabel = "##ActionBlockSwapButtonsMenu";
        const std::string buttonlabel = ICON_FA_RIGHT_LEFT"##ActionSwapBlockButtons";

        if (ImGui::IsItemHovered() || ImGui::IsPopupOpen(menulabel.c_str()) || actionIndex == nbActions)
        {
            ImGui::Button(buttonlabel.c_str(), ImVec2(buttonSize, buttonSize));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                track->swapActions(actionIndex, actionIndex-1);
            }
        }

        ImGui::PopID();
    }

    window->DC.CursorPosPrevLine = backuppos;
    window->DC.CursorPos.y = backuppos.y + ProgramSizes().TrackHeight + ImGui::GetStyle().ItemSpacing.x;
}

void ProgramWindow::showBlockOptionButton(const std::string &menulabel, const std::string &label)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    auto backuppos = window->DC.CursorPosPrevLine;

    float ysize = ImGui::CalcTextSize(ICON_FA_BARS).y + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImVec2 buttonSize(ysize, ysize);
    window->DC.CursorPos = window->DC.CursorPosPrevLine;
    window->DC.CursorPos.x -= buttonSize.x + ImGui::GetStyle().FramePadding.x;
    window->DC.CursorPos.y += ImGui::GetStyle().FramePadding.y * 0.5;
    ImGui::PushStyleColor(ImGuiCol_Button, COLOR_TRANSPARENT);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.05f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_TRANSPARENT);
    std::string buttonlabel = ICON_FA_BARS;
    buttonlabel += "##" + label;
    if (ImGui::Button(buttonlabel.c_str(), buttonSize))
    {
        ImGui::OpenPopup(menulabel.c_str());
    }
    ImGui::PopStyleColor(3);

    window->DC.CursorPosPrevLine = backuppos;
}

void ProgramWindow::showTrackName(const int& trackIndex)
{
    ImGui::BeginDisabled();
    ImGui::PushStyleColor(ImGuiCol_ButtonText, COLOR_BLACK);
    ImGui::PushStyleColor(ImGuiCol_Button, COLOR_WHITE);
    ImGui::Button(("T"+std::to_string(trackIndex + 1)).c_str());
    ImGui::PopStyleColor(2);
    ImGui::EndDisabled();
}

void ProgramWindow::initFilePath(const std::string& filename)
{
    const std::string extension = m_program.getExtension();

    std::filesystem::path absFilename;
    if (!filename.empty())
        absFilename = std::filesystem::absolute(filename);

    if (m_ws_programDirPath.empty())
    {
        if (!absFilename.empty() && sofa::helper::system::FileSystem::exists(absFilename.parent_path().string()))
        {
            m_ws_programDirPath = absFilename.parent_path().string();
        }
        else
        {
            m_ws_programDirPath = sofa::helper::Utils::getSofaUserLocalDirectory();
        }
    }

    if (m_ws_programFilename.empty())
    {
        if (!absFilename.empty())
        {
            std::filesystem::path path(absFilename);
            path = path.replace_extension(extension);
            m_ws_programFilename = path.filename().string();
        }
        else
        {
            m_ws_programFilename = "output" + extension;
        }
    }
}

bool ProgramWindow::importProgram()
{
    bool successfulImport = false;
    nfdchar_t *outPath;
    std::vector<nfdfilteritem_t> nfd_filters;
    nfd_filters.push_back({"program file", "crprog"});
    std::filesystem::path path;
    initFilePath(m_baseGUI->getFilename());

    nfdresult_t result = NFD_OpenDialog(&outPath, nfd_filters.data(), nfd_filters.size(), (m_ws_programDirPath.empty()) ? nullptr : m_ws_programDirPath.c_str());
    if (result == NFD_OKAY)
    {
        if (sofa::helper::system::FileSystem::exists(outPath))
        {
            successfulImport = importProgram(outPath);
            path = outPath;
        }
        NFD_FreePath(outPath);
    }
    else if (result == NFD_ERROR) {
        FooterStatusBar::getInstance().setTempMessage("Import failed to proceed.", FooterStatusBar::MERROR);
    }

    if (successfulImport)
    {
        FooterStatusBar::getInstance().setTempMessage("Imported program [" + path.string() + "]");
    }

    return successfulImport;
}

bool ProgramWindow::importProgram(const std::string &filename)
{
    bool successfulImport = false;
    if (sofa::helper::system::FileSystem::exists(filename))
    {
        successfulImport = m_program.importProgram(filename, m_baseGUI->getRootNode());
        if (successfulImport)
            saveProgramDirAndFilename(filename);
    }

    return successfulImport;
}

void ProgramWindow::exportProgram(const bool &exportAs)
{
    nfdchar_t *outPath;
    std::vector<nfdfilteritem_t> nfd_filters;
    nfd_filters.push_back({"program file", "crprog"});
    const std::string extension = m_program.getExtension();
    initFilePath(m_baseGUI->getFilename());

    std::filesystem::path path;
    path = m_ws_programDirPath;
    path.append(m_ws_programFilename);
    bool doExport = true;

    if (exportAs)
    {
        nfdresult_t result = NFD_SaveDialog(&outPath, nfd_filters.data(), nfd_filters.size(), m_ws_programDirPath.c_str(), m_ws_programFilename.c_str());
        if (result == NFD_OKAY)
        {
            path = outPath;
            path = (!path.has_extension())? outPath + extension: outPath;
            saveProgramDirAndFilename(path.string());
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

void ProgramWindow::saveProgramDirAndFilename(const std::string& filepath)
{
    if (sofa::helper::system::FileSystem::exists(filepath))
    {
        std::filesystem::path path = filepath;

        if (path.extension() == m_program.getExtension() && path.has_parent_path() && path.has_filename())
        {
            m_ws_programDirPath = path.parent_path().string(); // store chosen dir path
            m_ws_programFilename = path.filename().string(); // store chosen filename
        }
    }
}

void ProgramWindow::stepProgram(const double &dt, const bool &reverse)
{
    if (isDrivingSimulation())
    {
        double eps = 1e-5;
        const auto& tracks = m_program.getTracks();
        for (auto track: tracks)
        {
            double blockEnd = 0;
            double blockStart = 0;
            const auto& actions = track->getActions();
            for (auto action: actions)
            {
                blockEnd += action->getDuration();
                if ((!reverse && (blockEnd - m_time) > eps) || (reverse && (blockEnd - m_time - dt) > eps))
                {
                    RigidCoord position = (m_kinematicsGUIDataManager->hasTCP())? m_kinematicsGUIDataManager->getTCPGUIData()->getTCPPosition() : RigidCoord();
                    if (action->apply(position, m_time + dt - blockStart)) // apply the time corresponding to the end of the time step
                    {
                        if (m_kinematicsGUIDataManager->hasTCP())
                            m_kinematicsGUIDataManager->getTCPGUIData()->setTCPTargetPosition(position);
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
    if (isDrivingSimulation())
    {
        if (m_program.isEmpty())
            return;

        // Always start from the cursor's position and clamp time to the program's duration
        double time = m_cursorPos / ProgramSizes().TimelineOneSecondSize;
        time = std::clamp(time, 0., m_program.getDuration());
        setTime(time);

        double eps = 1e-5;
        static bool reverse = false;
        double dt = reverse? -groot->getDt(): groot->getDt();
        double programDuration = m_program.getDuration();

        if (groot->getTime() >= programDuration - eps) // if we've reached the end of the program
        {
            if (m_ws_repeat) // start from beginning
            {
                setTime(0.);
            }
            else if (m_ws_reverse)
            {
                reverse = true;
                dt = -groot->getDt();
            }
            else // nothing to do, exit
            {
                setTime(programDuration);
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

        stepProgram(dt, reverse);
        m_time += dt; // for cursor display
        m_cursorPos = m_time * ProgramSizes().TimelineOneSecondSize; // update the cursor position

    } // isDrivingSimulation
}

void ProgramWindow::animateEndEvent(sofa::simulation::Node *groot)
{
    SOFA_UNUSED(groot);
    if (isDrivingSimulation())
        groot->setTime(m_time);
}

void ProgramWindow::setTime(const double &time)
{
    m_time = time;
    m_baseGUI->getRootNode().get()->setTime(m_time);
}

void ProgramWindow::addStartMoveBlockMenu(const std::string& menuLabel,
                                        const sofa::Index& trackIndex,
                                        models::Track::SPtr track,
                                        models::actions::StartMove::SPtr startmove)
{
    if (ImGui::BeginPopup(menuLabel.c_str()))
    {
        if (ImGui::BeginMenu("Add after"))
        {
            addAddActionMenu(track, trackIndex, 0);
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Overwrite waypoint"))
        {
            startmove->setWaypoint(m_kinematicsGUIDataManager->getTCPGUIData()->getTCPTargetPosition());
            track->updateNextMoveInitialPoint(-1, startmove->getWaypoint());
        }
        ImGui::EndPopup();
    }
}

sofa::Index ProgramWindow::addActionBlockMenu(const std::string& menuLabel,
                                              const sofa::Index& actionIndex,
                                              const sofa::Index& trackIndex,
                                              models::Track::SPtr track,
                                              models::actions::Action::SPtr action)
{
    sofa::Index index = actionIndex;

    if (ImGui::BeginPopup(menuLabel.c_str()))
    {
        if (ImGui::BeginMenu("Add before"))
        {
            addAddActionMenu(track, trackIndex, index);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add after"))
        {
            addAddActionMenu(track, trackIndex, index+1);
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate"))
        {
            action->duplicate()->insertInTrack(track, index + 1);
        }
        if (ImGui::BeginMenu("Replace"))
        {
            if (addAddActionMenu(track, trackIndex, index+1))
            {
                action->deleteFromTrack(track, index);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Swap"))
        {
            sofa::Size nbActions = track->getActions().size();

            bool disableLeft = (index == 0);
            if (disableLeft)
                ImGui::BeginDisabled();
            if (ImGui::MenuItem("Left"))
                track->swapActions(index-1, index);
            if (disableLeft)
                ImGui::EndDisabled();

            bool disableRight = (index == nbActions-1);
            if (disableRight)
                ImGui::BeginDisabled();
            if (ImGui::MenuItem("Right"))
                track->swapActions(index, index+1);
            if (disableRight)
                ImGui::EndDisabled();

            ImGui::EndMenu();
        }

        ImGui::Separator();
        ImGui::EndPopup();
    }

    models::actions::Move::SPtr move = std::dynamic_pointer_cast<models::actions::Move>(action);
    if (move)
    {
        if (ImGui::BeginPopup(menuLabel.c_str()))
        {
            if (ImGui::MenuItem("Overwrite waypoint"))
            {
                move->setWaypoint(m_kinematicsGUIDataManager->getTCPGUIData()->getTCPTargetPosition());
                track->updateNextMoveInitialPoint(actionIndex, move->getWaypoint());
            }
            ImGui::Separator();
            ImGui::EndPopup();
        }
    }

    if (ImGui::BeginPopup(menuLabel.c_str()))
    {
        if (ImGui::MenuItem("Delete"))
        {
            action->deleteFromTrack(track, actionIndex);
        }
        else
            index++;
        ImGui::EndPopup();
    } else {
        index++;
    }

    return index;
}

bool ProgramWindow::addAddActionMenu(models::Track::SPtr track, const int &trackIndex, const int &actionIndex)
{
    if (m_kinematicsGUIDataManager->hasTCP() && ImGui::MenuItem(("Move##" + std::to_string(trackIndex)).c_str()))
    {
        auto move = std::make_shared<models::actions::Move>(RigidCoord(),
                                                            m_kinematicsGUIDataManager->getTCPGUIData()->getTCPTargetPosition(),
                                                            models::actions::Action::DEFAULTDURATION,
                                                            m_kinematicsGUIDataManager,
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

    if (ImGui::MenuItem(("Custom##" + std::to_string(trackIndex)).c_str()))
    {
        auto custom = std::make_shared<models::actions::Custom>();
        custom->insertInTrack(track, actionIndex);
        return true;
    }

    return false;
}

sofa::Index ProgramWindow::addTrackMenu(const std::string& menuLabel, const sofa::Index& trackIndex, models::Track::SPtr track)
{
    auto index = trackIndex;
    if (ImGui::BeginPopup(menuLabel.c_str()))
    {
        if (ImGui::MenuItem(("Clear track##" + std::to_string(index)).c_str()))
        {
            track->clear();
        }
        if (ImGui::MenuItem(("Add track after##" + std::to_string(index)).c_str(), nullptr, false))
        {
            m_program.addTrack(std::make_shared<models::Track>(m_kinematicsGUIDataManager), index+1);
        }
        if (ImGui::MenuItem(("Remove track##" + std::to_string(index)).c_str(), nullptr, false, (index>0)? true : false))
        {
            m_program.removeTrack(index--);
        }

        ImGui::Separator();

        if (ImGui::BeginMenu(("Add action##" + std::to_string(index)).c_str()))
        {
            addAddActionMenu(track, index, track->getActions().size());
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    return index;
}

void ProgramWindow::addActionsContextMenu(const std::string& label, const int& actionIndex)
{
    if (ImGui::BeginPopup(label.c_str()))
    {
        auto tracks = m_program.getTracks();
        int trackIndex = m_program.getTrackSelected();

        models::Track::SPtr track = nullptr;
        if (trackIndex>=0 && trackIndex<(int)tracks.size())
            track = tracks[trackIndex];

        { // Group
            bool disabled = (track == nullptr) || !track->canGroup(actionIndex);

            if (disabled)
                ImGui::BeginDisabled();

            ImGui::MenuItem("Group");
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) // These two steps are required to capture the click before clearing the selected actions
            {
                if (track)
                    track->group();
            }

            if (disabled)
                ImGui::EndDisabled();
        }

        { // Ungroup
            bool disabled = (track == nullptr) || !track->canUngroup(actionIndex) || track->hasSelectedActions();

            if (disabled)
                ImGui::BeginDisabled();

            ImGui::MenuItem("Ungroup");
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                if (track)
                    track->ungroup(actionIndex);
            }

            if (disabled)
                ImGui::EndDisabled();
        }

        ImGui::EndPopup();
    }
}

} // namespace


























