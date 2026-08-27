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
#pragma once
#include <SofaImGui/config.h>

#include <memory>
#include <SofaGLFW/BaseGUIEngine.h>
#include <sofa/gl/FrameBufferObject.h>

#include <imgui.h>
#include <sofa/simulation/Node.h>
#include <SimpleIni.h>

#include <SofaImGui/windows/ViewportWindow.h>
#include <SofaImGui/windows/SceneGraphWindow.h>
#include <SofaImGui/windows/ComponentsWindow.h>
#include <SofaImGui/windows/IOWindow.h>
#include <SofaImGui/windows/LogWindow.h>
#include <SofaImGui/windows/MyRobotWindow.h>
#include <SofaImGui/windows/MoveWindow.h>
#include <SofaImGui/windows/PlottingWindow.h>
#include <SofaImGui/windows/ProgramWindow.h>
#include <SofaImGui/windows/ProfilerWindow.h>
#include <SofaImGui/windows/DashboardWindow.h>
#include <SofaImGui/windows/RecordVideoWindow.h>
#include <SofaImGui/windows/PluginsWindow.h>
#include <SofaImGui/windows/MouseManagerWindow.h>

#include <SofaImGui/menus/ViewMenu.h>
#include <SofaImGui/models/guidata/KinematicsGUIDataManager.h>
#include <SofaImGui/models/KinematicsController.h>

#include <SoftRobots.Inverse/component/solver/QPInverseProblemSolver.h>
#include <SoftRobots.Inverse/component/constraint/PositionEffector.h>
#include <SofaImGui/FooterStatusBar.h>
#include <SofaImGui/Robot.h>

struct GLFWwindow;
namespace sofa::glfw
{
    class SofaGLFWBaseGUI;
}

namespace sofaimgui
{

class SOFAIMGUI_API ImGuiGUIEngine : public sofaglfw::BaseGUIEngine
{
public:
    ImGuiGUIEngine() = default;
    ~ImGuiGUIEngine() = default;
    
    void init() override;
    void initBackend(GLFWwindow*) override;
    void startFrame(sofaglfw::SofaGLFWBaseGUI*) override;
    void endFrame() override {}
    void beforeDraw(GLFWwindow* window) override;
    void afterDraw() override;
    void terminate() override;
    bool dispatchMouseEvents() override;
    sofa::type::Vec2i getFrameBufferPixels(std::vector<uint8_t>& pixels) override;

    void animateBeginEvent(sofa::simulation::Node* groot) override;
    void animateEndEvent(sofa::simulation::Node* groot) override;

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) override;

    void loadSimulation(const bool& reload, const std::string &filename) override;

    void saveProject(const bool& saveAs=false);
    bool loadProject();

    void setRobotConnection(const bool& robotConnectionToggle) { Robot::getInstance().setConnection(robotConnectionToggle); }
    bool getRobotConnection() { return Robot::getInstance().getConnection(); }

    models::guidata::KinematicsGUIDataManager::SPtr m_kinematicsGUIDataManager = std::make_shared<models::guidata::KinematicsGUIDataManager>();
    models::KinematicsController::SPtr m_kinematicsController = sofa::core::objectmodel::New<models::KinematicsController>(m_kinematicsGUIDataManager); // TODO: remove when QPInverseProblemSolver can solve direct problem

    windows::ViewportWindow     m_viewportWindow     = windows::ViewportWindow("Viewport");
    windows::SceneGraphWindow   m_sceneGraphWindow   = windows::SceneGraphWindow("Scene Graph");
    windows::ComponentsWindow   m_componentsWindow   = windows::ComponentsWindow("Components");
    windows::LogWindow          m_logWindow          = windows::LogWindow("Log");
    windows::IOWindow           m_IOWindow           = windows::IOWindow("Input/Output", m_kinematicsGUIDataManager);
    windows::ProgramWindow      m_programWindow      = windows::ProgramWindow("Program", m_kinematicsGUIDataManager);
    windows::PlottingWindow     m_plottingWindow     = windows::PlottingWindow("Plotting");
    windows::ProfilerWindow     m_profilerWindow     = windows::ProfilerWindow("Profiler");
    windows::MyRobotWindow      m_myRobotWindow      = windows::MyRobotWindow("My Robot");
    windows::MoveWindow         m_moveWindow         = windows::MoveWindow("Move", m_kinematicsGUIDataManager);
    windows::DashboardWindow    m_dashboardWindow    = windows::DashboardWindow("Dashboard");

    windows::PluginsWindow      m_pluginsWindow      = windows::PluginsWindow("Plugins Manager");
    windows::MouseManagerWindow m_mouseManagerWindow = windows::MouseManagerWindow("Mouse Manager");
    windows::RecordVideoWindow  m_recordVideoWindow  = windows::RecordVideoWindow("Record Video");


protected:

    std::unique_ptr<sofa::gl::FrameBufferObject> m_fbo;
    std::pair<unsigned int, unsigned int> m_currentFBOSize;

    std::vector<std::reference_wrapper<windows::BaseWindow>> m_windows{ // Menu bar > Windows
                                                                        m_IOWindow,
                                                                        m_programWindow,
                                                                        m_myRobotWindow,
                                                                        m_moveWindow,
                                                                        m_plottingWindow,
                                                                        m_viewportWindow,
                                                                        m_sceneGraphWindow,
                                                                        m_componentsWindow,
                                                                        m_logWindow,
                                                                        m_profilerWindow,
                                                                        m_dashboardWindow
                                                                       };

    std::vector<std::reference_wrapper<windows::BaseWindow>> m_modalWindows{
                                                                            m_pluginsWindow,
                                                                            m_mouseManagerWindow,
                                                                            m_recordVideoWindow
                                                                            };

    CSimpleIniA iniGUISettings;

    void initDockSpace(const bool& firstTime);
    void setupIOConfig();
    void changeWorkbench(Workbench wb);

    void showViewportWindow(sofaglfw::SofaGLFWBaseGUI* baseGUI);
    void showOptionWindows();
    void showMainMenuBar(sofaglfw::SofaGLFWBaseGUI* baseGUI);
    void showSecondaryMenuBar();
    void showStatusBar();
    void applyDarkMode(const bool &darkMode, sofaglfw::SofaGLFWBaseGUI* baseGUI=nullptr);

    void saveSettings();
    void enableWindows();
    void createGUINode(Node::SPtr guiNode = nullptr);
    void clearWindows();
    void clearWindowsGUIData();
    void applyDockSizeFromWindowsSettings(const ImGuiID& id);
    void setWindowsBaseGUI(sofaglfw::SofaGLFWBaseGUI*);
    void notifyWindowsEndSimulationLoad();

    bool m_animate{false};
    bool m_darkMode{false};
    sofaglfw::SofaGLFWBaseGUI* m_baseGUI{nullptr};

    std::vector<ImGuiID> m_dockIDs;

    std::size_t m_frameCount{0};
    static inline constexpr int s_NB_PBOS = 2;
    GLuint m_pbos[s_NB_PBOS];
    sofa::type::Vec2i m_pboSize;
};

} // namespace sofaimgui
