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

#include <SofaImGui/ObjectColor.h>
#include <SofaImGui/ImGuiDataWidget.h>
#include <SofaImGui/ImGuiGUIEngine.h>
#include <SofaImGui/windows/Performances.h>
#include <SofaImGui/windows/Log.h>
#include <SofaImGui/windows/Profiler.h>
#include <SofaImGui/windows/SceneGraph.h>
#include <SofaImGui/windows/DisplayFlags.h>
#include <SofaImGui/windows/Plugins.h>
#include <SofaImGui/windows/Components.h>
#include <SofaImGui/windows/Settings.h>
#include <SofaImGui/AppIniFile.h>
#include <SofaImGui/windows/ViewPort.h>

#include <SofaGLFW/SofaGLFWBaseGUI.h>

#include <sofa/gl/component/rendering3d/OglSceneFrame.h>
#include <sofa/gui/common/BaseGUI.h>
#include <sofa/type/vector.h>

#include <sofa/core/CategoryLibrary.h>
#include <sofa/core/loader/SceneLoader.h>
#include <sofa/core/ComponentLibrary.h>
#include <sofa/core/visual/VisualParams.h>

#include <sofa/simulation/Simulation.h>
#include <sofa/simulation/Node.h>

#include <sofa/helper/Utils.h>
#include <sofa/helper/AdvancedTimer.h>
#include <sofa/helper/logging/LoggingMessageHandler.h>
#include <sofa/helper/io/File.h>
#include <sofa/helper/io/STBImage.h>
#include <sofa/helper/system/PluginManager.h>
#include <sofa/helper/system/FileSystem.h>

#include <sofa/component/visual/VisualStyle.h>
#include <sofa/component/visual/VisualGrid.h>
#include <sofa/component/visual/LineAxis.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h> //imgui_internal.h is included in order to use the DockspaceBuilder API (which is still in development)
#include <implot.h>
#include <nfd.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_opengl2.h>
#include <IconsFontAwesome6.h>

#include <fa-regular-400.h>
#include <fa-solid-900.h>
#include <OpenSans-Regular.h>
#include <Style.h>

#include <SofaImGui/menus/FileMenu.h>
#include <SofaImGui/menus/ViewMenu.h>
#include <SofaImGui/Utils.h>
#include <SofaImGui/widgets/Buttons.h>

#include <sofa/helper/Utils.h>
#include <sofa/type/vector.h>
#include <sofa/simulation/Node.h>
#include <sofa/component/visual/VisualStyle.h>
#include <sofa/core/ComponentLibrary.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/helper/system/PluginManager.h>
#include <SofaImGui/ObjectColor.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/helper/io/File.h>
#include <sofa/component/visual/VisualGrid.h>
#include <sofa/component/visual/LineAxis.h>
#include <sofa/gl/component/rendering3d/OglSceneFrame.h>
#include <sofa/gui/common/BaseGUI.h>
#include <sofa/helper/io/STBImage.h>
#include <sofa/simulation/graph/DAGNode.h>
#include <sofa/version.h>

using namespace sofa;

namespace sofaimgui
{

void ImGuiGUIEngine::saveSettings()
{
    const std::string settingsFile = sofaimgui::AppIniFile::getSettingsIniFile();
    msg_info("") << "Saving application settings in " << settingsFile;
    ini.SaveFile(settingsFile.c_str());
}

void ImGuiGUIEngine::setIPController(sofa::simulation::Node::SPtr groot,
                                     softrobotsinverse::solver::QPInverseProblemSolver::SPtr solver,
                                     sofa::core::behavior::BaseMechanicalState::SPtr TCPTargetMechanical,
                                     core::behavior::BaseMechanicalState::SPtr TCPMechanical,
                                     softrobotsinverse::constraint::PositionEffector<defaulttype::Rigid3Types>::SPtr rotationEffector)
{
    if (m_IPController)
        groot->removeObject(m_IPController.get());

    m_IPController = sofa::core::objectmodel::New<models::IPController>(groot, solver, TCPTargetMechanical, TCPMechanical, rotationEffector);
    m_IPController->setName(groot->getNameHelper().resolveName(m_IPController->getClassName(), sofa::core::ComponentNameHelper::Convention::python));

    groot->addObject(m_IPController.get());
    m_programWindow.setIPController(m_IPController);
    m_moveWindow.setIPController(m_IPController);
    m_IOWindow.setIPController(m_IPController);
}

void ImGuiGUIEngine::setRobotConnectionToggle(const bool& robotConnectionToggle)
{
    m_robotConnectionToggle = robotConnectionToggle;
    if (m_robotConnectionToggle)
        FooterStatusBar::getInstance().setTempMessage("Robot connected. See Windows>Log for more information.");
    else
        FooterStatusBar::getInstance().setTempMessage("Robot disconnected. See Windows>Log for more information.");
}

void ImGuiGUIEngine::init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    NFD_Init();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    static const std::string windowsIniFile(sofaimgui::AppIniFile::getWindowsIniFile());
    io.IniFilename = windowsIniFile.c_str();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ini.SetUnicode();
    if (sofa::helper::system::FileSystem::exists(sofaimgui::AppIniFile::getSettingsIniFile()))
    {
        SI_Error rc = ini.LoadFile(sofaimgui::AppIniFile::getSettingsIniFile().c_str());
        SOFA_UNUSED(rc);
        assert(rc == SI_OK);
    }

    const char* darkMode = ini.GetValue("Style", "darkMode");
    if (darkMode)
    {
        std::string m = darkMode;
        m_darkMode = (m=="on");
    }
    else
    {
        ini.SetValue("Style", "darkMode", (m_darkMode)? "on": "off");
    }
    applyDarkMode(m_darkMode);

    sofa::helper::system::PluginManager::getInstance().readFromIniFile(sofa::gui::common::BaseGUI::getConfigDirectoryPath() + "/loadedPlugins.ini");
}

void ImGuiGUIEngine::initBackend(GLFWwindow* glfwWindow)
{
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);

#if SOFAIMGUI_FORCE_OPENGL2 == 1
    ImGui_ImplOpenGL2_Init();
#else
    ImGui_ImplOpenGL3_Init(nullptr);
#endif // SOFAIMGUI_FORCE_OPENGL2 == 1

    GLFWmonitor* monitor = glfwGetWindowMonitor(glfwWindow);
    if (!monitor)
    {
        monitor = glfwGetPrimaryMonitor();
    }
    if (monitor)
    {
        float xscale, yscale;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->ClearFonts();
        io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_Regular_compressed_data, OpenSans_Regular_compressed_size, 18 * yscale);

        ImFontConfig config;
        config.MergeMode = true;
        config.GlyphMinAdvanceX = .0f; // Use if you want to make the icon monospaced
        static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        io.Fonts->AddFontFromMemoryCompressedTTF(FA_REGULAR_400_compressed_data, FA_REGULAR_400_compressed_size, 12 * yscale, &config, icon_ranges);
        io.Fonts->AddFontFromMemoryCompressedTTF(FA_SOLID_900_compressed_data, FA_SOLID_900_compressed_size, 12 * yscale, &config, icon_ranges);
    }
}

void ImGuiGUIEngine::startFrame(sofaglfw::SofaGLFWBaseGUI* baseGUI)
{
    // Start the Dear ImGui frame
#if SOFAIMGUI_FORCE_OPENGL2 == 1
    ImGui_ImplOpenGL2_NewFrame();
#else
    ImGui_ImplOpenGL3_NewFrame();
#endif // SOFAIMGUI_FORCE_OPENGL2 == 1

    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    initDockSpace();
    showMainMenuBar(baseGUI);
    FooterStatusBar::getInstance().showFooterStatusBar();
    FooterStatusBar::getInstance().showTempMessageOnStatusBar();

    static bool firstTime = true;
    if (firstTime)
    {
        firstTime = false;

        m_baseGUI = baseGUI;
        m_IOWindow.setSimulationState(m_simulationState);
        m_stateWindow->setSimulationState(m_simulationState);

        if(!m_programWindow.enabled())
            m_programWindow.setOpen(false);

        if(!m_plottingWindow.enabled())
            m_plottingWindow.setOpen(false);

        if(!m_myRobotWindow.enabled())
            m_myRobotWindow.setOpen(false);

        if(!m_moveWindow.enabled())
            m_moveWindow.setOpen(false);
    }

    showViewportWindow(baseGUI);
    showOptionWindows(baseGUI);

    ImGui::Render();

#if SOFAIMGUI_FORCE_OPENGL2 == 1
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
#else
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif // SOFAIMGUI_FORCE_OPENGL2 == 1

    const ImGuiIO& io = ImGui::GetIO();
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void ImGuiGUIEngine::beforeDraw(GLFWwindow*)
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_fbo)
    {
        m_fbo = std::make_unique<sofa::gl::FrameBufferObject>();
        m_currentFBOSize = {500, 500};
        m_fbo->init(m_currentFBOSize.first, m_currentFBOSize.second);
    }
    else
    {
        if (m_currentFBOSize.first != static_cast<unsigned int>(m_viewportWindow.m_windowSize.first)
            || m_currentFBOSize.second != static_cast<unsigned int>(m_viewportWindow.m_windowSize.second))
        {
            m_fbo->setSize(static_cast<unsigned int>(m_viewportWindow.m_windowSize.first), static_cast<unsigned int>(m_viewportWindow.m_windowSize.second));
            m_currentFBOSize = {static_cast<unsigned int>(m_viewportWindow.m_windowSize.first), static_cast<unsigned int>(m_viewportWindow.m_windowSize.second)};
        }
    }
    sofa::core::visual::VisualParams::defaultInstance()->viewport() = {0,0,m_currentFBOSize.first, m_currentFBOSize.second};

    m_fbo->start();
}

void ImGuiGUIEngine::afterDraw()
{
    m_fbo->stop();
}

void ImGuiGUIEngine::terminate()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ini.SetDoubleValue("Window", "width", viewport->Size.x);
    ini.SetDoubleValue("Window", "height", viewport->Size.y);
    saveSettings();

    NFD_Quit();

#if SOFAIMGUI_FORCE_OPENGL2 == 1
    ImGui_ImplOpenGL2_Shutdown();
#else
    ImGui_ImplOpenGL3_Shutdown();
#endif // SOFAIMGUI_FORCE_OPENGL2 == 1

    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

bool ImGuiGUIEngine::dispatchMouseEvents()
{
    return !ImGui::GetIO().WantCaptureMouse || m_viewportWindow.isMouseOnViewport();
}

void ImGuiGUIEngine::initDockSpace()
{
    static auto firstTime = true;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                       ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoMove | ImGuiDockNodeFlags_PassthruCentralNode
                 );

    ImGuiID dockspaceID = ImGui::GetID("WorkSpaceDockSpace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    if (firstTime)
    {
        firstTime = false;
        ImGui::DockBuilderRemoveNode(dockspaceID); // clear any previous layout
        ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

        auto dock_id_right = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.25f, nullptr, &dockspaceID);
        ImGui::DockBuilderDockWindow(m_IOWindow.getName().c_str(), dock_id_right);
        ImGui::DockBuilderDockWindow(m_myRobotWindow.getName().c_str(), dock_id_right);

        auto dock_id_right_up = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Up, 0.55f, nullptr, &dock_id_right);
        ImGui::DockBuilderDockWindow(m_moveWindow.getName().c_str(), dock_id_right_up);
        ImGui::DockBuilderDockWindow(m_sceneGraphWindow.getName().c_str(), dock_id_right_up);

        ImGui::DockBuilderDockWindow(m_viewportWindow.getName().c_str(), dockspaceID);

        auto dock_id_down = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Down, 0.32f, nullptr, &dockspaceID);
        ImGui::DockBuilderDockWindow(m_programWindow.getName().c_str(), dock_id_down);
        ImGui::DockBuilderDockWindow(m_plottingWindow.getName().c_str(), dock_id_down);
        ImGui::DockBuilderDockWindow(m_logWindow.getName().c_str(), dock_id_down);

        ImGui::DockBuilderGetNode(dockspaceID)->WantHiddenTabBarToggle = true;

        ImGui::DockBuilderFinish(dockspaceID);
    }

    ImGui::End();
}

void ImGuiGUIEngine::showViewportWindow(sofaglfw::SofaGLFWBaseGUI* baseGUI)
{
    showFrameOnViewport(baseGUI);
    auto groot = baseGUI->getRootNode();
    m_animate = groot->animate_.getValue();

    static bool firstTime = true;
    if (firstTime)
    {
        firstTime = false;
        Utils::resetSimulationView(baseGUI);
    }

    m_viewportWindow.showWindow(groot.get(), (ImTextureID)m_fbo->getColorTexture(),
                                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize
                                );

    // Animate button
    if (m_viewportWindow.addAnimateButton(&m_animate))
        sofa::helper::getWriteOnlyAccessor(groot->animate_).wref() = m_animate;

    // Step button
    if (m_viewportWindow.addStepButton())
    {
        if (!m_animate)
        {
            sofa::helper::AdvancedTimer::begin("Animate");
            animateBeginEvent(groot.get());
            sofa::simulation::node::animate(groot.get(), groot->getDt());
            animateEndEvent(groot.get());
            sofa::simulation::node::updateVisual(groot.get());
            sofa::helper::AdvancedTimer::end("Animate");
        }
    }

    // Driving Tab combo
    static const char* listTabs[]{"Move", "Program", "Input/Output"};

    if(!m_IPController)
        ImGui::BeginDisabled();

    if (m_viewportWindow.addDrivingTabCombo(&m_mode, listTabs, IM_ARRAYSIZE(listTabs)))
    {
        const auto filename = baseGUI->getFilename();

        m_moveWindow.setDrivingTCPTarget(false);
        m_programWindow.setDrivingTCPTarget(false);
        m_IOWindow.setDrivingTCPTarget(false);
        switch (m_mode) {
            case 1:
            {
                m_programWindow.setTime(groot->getTime());
                m_programWindow.setDrivingTCPTarget(true);
                break;
            }
            case 2:
            {
                m_IOWindow.setDrivingTCPTarget(true);
                break;
            }
            default:
            {
                m_moveWindow.setDrivingTCPTarget(true);
                break;
            }
        }
    }

    if(!m_IPController)
        ImGui::EndDisabled();
}

void ImGuiGUIEngine::showOptionWindows(sofaglfw::SofaGLFWBaseGUI* baseGUI)
{
    auto groot = baseGUI->getRootNode().get();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ;

    m_programWindow.showWindow(baseGUI, windowFlags);
    m_plottingWindow.showWindow(groot, windowFlags);

    m_logWindow.showWindow(windowFlags);
    m_IOWindow.showWindow(groot, windowFlags);
    m_myRobotWindow.showWindow(windowFlags);
    m_moveWindow.showWindow(windowFlags);
    m_sceneGraphWindow.showWindow(groot, windowFlags);

}

void ImGuiGUIEngine::showMainMenuBar(sofaglfw::SofaGLFWBaseGUI* baseGUI)
{
    if (ImGui::BeginMainMenuBar())
    {
        menus::FileMenu fileMenu(baseGUI);
        if(fileMenu.addMenu())
        {
            reloadSimulation();
        }

        menus::ViewMenu(baseGUI).addMenu(m_currentFBOSize, m_fbo->getColorTexture());

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::PopStyleColor();
            if (!m_IOWindow.enabled())
                ImGui::BeginDisabled();
            ImGui::LocalCheckBox(m_IOWindow.getName().c_str(), &m_IOWindow.isOpen());
            if (!m_IOWindow.enabled())
                ImGui::EndDisabled();

            if (!m_programWindow.enabled())
                ImGui::BeginDisabled();
            ImGui::LocalCheckBox(m_programWindow.getName().c_str(), &m_programWindow.isOpen());
            if (!m_programWindow.enabled())
                ImGui::EndDisabled();

            if (!m_plottingWindow.enabled())
                ImGui::BeginDisabled();
            ImGui::LocalCheckBox(m_plottingWindow.getName().c_str(), &m_plottingWindow.isOpen());
            if (!m_plottingWindow.enabled())
                ImGui::EndDisabled();

            if (!m_myRobotWindow.enabled())
                ImGui::BeginDisabled();
            ImGui::LocalCheckBox(m_myRobotWindow.getName().c_str(), &m_myRobotWindow.isOpen());
            if (!m_myRobotWindow.enabled())
                ImGui::EndDisabled();

            if (!m_moveWindow.enabled())
                ImGui::BeginDisabled();
            ImGui::LocalCheckBox(m_moveWindow.getName().c_str(), &m_moveWindow.isOpen());
            if (!m_moveWindow.enabled())
                ImGui::EndDisabled();

            ImGui::Separator();

            if (!m_viewportWindow.enabled())
                ImGui::BeginDisabled();
            ImGui::LocalCheckBox(m_viewportWindow.getName().c_str(), &m_viewportWindow.isOpen());
            if (!m_viewportWindow.enabled())
                ImGui::EndDisabled();

            ImGui::Separator();

            if (!m_sceneGraphWindow.enabled())
                ImGui::BeginDisabled();
            ImGui::LocalCheckBox(m_sceneGraphWindow.getName().c_str(), &m_sceneGraphWindow.isOpen());
            if (!m_sceneGraphWindow.enabled())
                ImGui::EndDisabled();

            if (!m_logWindow.enabled())
                ImGui::BeginDisabled();
            ImGui::LocalCheckBox(m_logWindow.getName().c_str(), &m_logWindow.isOpen());
            if (!m_logWindow.enabled())
                ImGui::EndDisabled();

            ImGui::EndMenu();
        }
        else
        {
            ImGui::PopStyleColor();
        }

        static bool isAboutOpen = false;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::PopStyleColor();
            if (ImGui::MenuItem("About", nullptr, false, true))
                isAboutOpen = true;
            ImGui::EndMenu();
        }
        else
        {
            ImGui::PopStyleColor();
        }

        if (isAboutOpen)
        {
            ImGui::Begin("About##SofaComplianceRobotics", &isAboutOpen, ImGuiWindowFlags_NoDocking);

            auto windowWidth = ImGui::GetWindowSize().x;
            std::string version = "v" + std::string(SOFA_VERSION_STR);
            std::vector<std::string> texts = {"\n", "SOFA, Simulation Open-Framework Architecture \n (c) 2006 INRIA, USTL, UJF, CNRS, MGH",
                                              "&", "(c) Compliance Robotics", "\n",
                                              version,
                                              "SOFA is an open-source framework for interactive physics simulation, \n"
                                              "with an emphasis on soft body dynamics. After years of research and \n"
                                              "development, the project remains open-source under the LGPL v2.1 license, \n"
                                              "fostering both research and development."};
            for (const auto& text : texts)
            {
                auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;
                ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
                ImGui::Text("%s", text.c_str());
            }
            ImGui::End();
        }

        const auto posX = ImGui::GetCursorPosX();

        // Dark / light mode
        static bool firstTime = true;
        if (firstTime)
        {
            firstTime = false;
            applyDarkMode(m_darkMode, baseGUI);
        }
        auto position = ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(ICON_FA_SUN).x
                        - 2 * ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(position);
        ImVec2 buttonSize(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonText, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        if (ImGui::ButtonEx(m_darkMode? ICON_FA_SUN: ICON_FA_MOON, buttonSize))
        {
            ImGui::PopStyleColor(4);
            m_darkMode = !m_darkMode;
            applyDarkMode(m_darkMode, baseGUI);
            ini.SetValue("Style", "darkMode", (m_darkMode)? "on": "off");
        }
        else
        {
            ImGui::PopStyleColor(4);
        }
        ImGui::SetItemTooltip((m_darkMode)? "Light mode": "Dark mode");
        ImGui::SetCursorPosX(posX);

        ImGui::EndMainMenuBar();
    }

    ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();

    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.14f, 0.25f, 0.42f, 1.00f));
    if (ImGui::BeginViewportSideBar("##MySecondaryMenuBar", viewport, ImGuiDir_Up, height, window_flags)) {
        if (ImGui::BeginMenuBar()) {
            ImGui::SetCursorPosX(ImGui::GetColumnWidth() / 2.f - ImGui::GetFrameHeight() * 2.f); //approximatively the center of the menu bar

            { // Simulation / Robot button
                if (ImGui::LocalToggleButton("Connection", &m_robotConnectionToggle))
                {
                    if (m_robotConnectionToggle)
                        FooterStatusBar::getInstance().setTempMessage("Connecting the robot.");
                    else
                        FooterStatusBar::getInstance().setTempMessage("Disconnecting the robot.");
                }
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
                ImGui::Text(m_robotConnectionToggle? "Robot" : "Simulation");
                ImGui::PopStyleColor();
            }

            ImGui::EndMenuBar();
        }
        ImGui::End();
    }
    ImGui::PopStyleColor();
}

void ImGuiGUIEngine::applyDarkMode(const bool &darkMode, sofaglfw::SofaGLFWBaseGUI* baseGUI)
{
    if (darkMode)
    {
        sofaimgui::setStyle("deep_dark");
        if (baseGUI)
            baseGUI->setBackgroundColor(type::RGBAColor(0.16, 0.18, 0.20, 1.0));
    }
    else
    {
        sofaimgui::setStyle("light");
        if (baseGUI)
            baseGUI->setBackgroundColor(type::RGBAColor(0.76, 0.78, 0.80, 1.0));
    }
}

void ImGuiGUIEngine::showFrameOnViewport(sofaglfw::SofaGLFWBaseGUI* baseGUI)
{
    auto groot = baseGUI->getRootNode();
    auto sceneFrame = groot->get<sofa::gl::component::rendering3d::OglSceneFrame>();
    if (!sceneFrame)
    {
        auto newSceneFrame = sofa::core::objectmodel::New<sofa::gl::component::rendering3d::OglSceneFrame>();
        newSceneFrame->d_style.setValue(sofa::gl::component::rendering3d::OglSceneFrame::Style("CubeCones"));
        newSceneFrame->d_alignment.setValue(sofa::gl::component::rendering3d::OglSceneFrame::Alignment("TopRight"));

        groot->addObject(newSceneFrame);
        newSceneFrame->setName("viewportFrame");
        newSceneFrame->addTag(core::objectmodel::Tag("createdByGUI"));
        newSceneFrame->d_drawFrame.setValue(true);
        newSceneFrame->init();
    }
}

void ImGuiGUIEngine::animateBeginEvent(sofa::simulation::Node* groot)
{
    m_IOWindow.animateBeginEvent(groot);
    m_programWindow.animateBeginEvent(groot);
}

void ImGuiGUIEngine::animateEndEvent(sofa::simulation::Node* groot)
{
    m_IOWindow.animateEndEvent(groot);
    m_programWindow.animateEndEvent(groot);
}

void ImGuiGUIEngine::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    SOFA_UNUSED(mods);

    const bool isCtrlKeyPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
    const bool isShiftKeyPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    // Handle specific keyName for additional functionality for layout dependent keys
    const char* keyName = glfwGetKeyName(key, scancode);
    if (keyName)
    {
        if(strcmp(keyName, "i") == 0)
        {
            if (action == GLFW_PRESS && isCtrlKeyPressed && isShiftKeyPressed)
            {
                m_programWindow.importProgram();
            }
        }
        else if (strcmp(keyName, "r") == 0)
        {
            if (action == GLFW_PRESS && isCtrlKeyPressed)
            {
                reloadSimulation();
            }
        }
        else if (strcmp(keyName, "e") == 0)
        {
            if (action == GLFW_PRESS && isCtrlKeyPressed && isShiftKeyPressed)
            {
                m_programWindow.exportProgram(false);
            }
        }
    }

    if(m_viewportWindow.isFocusOnViewport())
    {
        switch (key)
        {
        case GLFW_KEY_1:
            sofaimgui::Utils::alignCamera(m_baseGUI, sofaimgui::Utils::CameraAlignement::TOP);
            break;
        case GLFW_KEY_2:
            sofaimgui::Utils::alignCamera(m_baseGUI, sofaimgui::Utils::CameraAlignement::BOTTOM);
            break;
        case GLFW_KEY_3:
            sofaimgui::Utils::alignCamera(m_baseGUI, sofaimgui::Utils::CameraAlignement::FRONT);
            break;
        case GLFW_KEY_4:
            sofaimgui::Utils::alignCamera(m_baseGUI, sofaimgui::Utils::CameraAlignement::BACK);
            break;
        case GLFW_KEY_5:
            sofaimgui::Utils::alignCamera(m_baseGUI, sofaimgui::Utils::CameraAlignement::RIGHT);
            break;
        case GLFW_KEY_6:
            sofaimgui::Utils::alignCamera(m_baseGUI, sofaimgui::Utils::CameraAlignement::LEFT);
            break;
        }
    }
}

void ImGuiGUIEngine::reloadSimulation()
{
    m_simulationState.clearStateData();
    m_myRobotWindow.clearData();
    m_moveWindow.clearData();
    m_plottingWindow.clearData();

    Utils::reloadSimulation(m_baseGUI, m_baseGUI->getFilename());

    auto groot = m_baseGUI->getRootNode().get();
    m_programWindow.addTrajectoryComponents(groot);
    m_IOWindow.setSimulationState(m_simulationState);
    m_stateWindow->setSimulationState(m_simulationState);
}

} //namespace sofaimgui
