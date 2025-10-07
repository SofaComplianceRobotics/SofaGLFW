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
#include <SofaImGui/windows/IOWindow.h>
#include <SofaImGui/windows/LogWindow.h>
#include <SofaImGui/windows/MyRobotWindow.h>
#include <SofaImGui/windows/MoveWindow.h>
#include <SofaImGui/windows/PlottingWindow.h>
#include <SofaImGui/windows/ProgramWindow.h>

#include <SofaImGui/menus/ViewMenu.h>

#include <SofaImGui/models/IPController.h>
#include <SofaImGui/models/SimulationState.h>

#include <SoftRobots.Inverse/component/solver/QPInverseProblemSolver.h>
#include <SoftRobots.Inverse/component/constraint/PositionEffector.h>
#include <SofaImGui/FooterStatusBar.h>
#include <SofaImGui/Robot.h>

#define OFFSET "       "

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

    void animateBeginEvent(sofa::simulation::Node* groot) override;
    void animateEndEvent(sofa::simulation::Node* groot) override;

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) override;

    void saveProject();

    void setIPController(sofa::simulation::Node::SPtr groot,
                         softrobotsinverse::solver::QPInverseProblemSolver::SPtr solver,
                         sofa::core::behavior::BaseMechanicalState::SPtr TCPTargetMechanical,
                         sofa::core::behavior::BaseMechanicalState::SPtr TCPMechanical,
                         softrobotsinverse::constraint::PositionEffector<sofa::defaulttype::Rigid3Types>::SPtr rotationEffector);

    void setRobotConnection(const bool& robotConnectionToggle) { Robot::getInstance().setConnection(robotConnectionToggle); }
    bool getRobotConnection() { return Robot::getInstance().getConnection(); }

    models::SimulationState& getSimulationState() {return m_simulationState;}

    std::shared_ptr<windows::StateWindow> m_stateWindow = std::make_shared<windows::StateWindow>("State", false);

    windows::ViewportWindow     m_viewportWindow     = windows::ViewportWindow(OFFSET"Viewport", true, m_stateWindow);
    windows::SceneGraphWindow   m_sceneGraphWindow   = windows::SceneGraphWindow(OFFSET"Scene Graph", false);
    windows::LogWindow          m_logWindow          = windows::LogWindow(OFFSET"Log", false);
    windows::IOWindow           m_IOWindow           = windows::IOWindow(OFFSET"Input/Output", false);
    windows::ProgramWindow      m_programWindow      = windows::ProgramWindow(OFFSET"Program", true);
    windows::PlottingWindow     m_plottingWindow     = windows::PlottingWindow(OFFSET"Plotting", true);
    windows::MyRobotWindow      m_myRobotWindow      = windows::MyRobotWindow(OFFSET"My Robot", true);
    windows::MoveWindow         m_moveWindow         = windows::MoveWindow(OFFSET"Move", true);



protected:
    std::unique_ptr<sofa::gl::FrameBufferObject> m_fbo;
    std::pair<unsigned int, unsigned int> m_currentFBOSize;

    std::vector<std::reference_wrapper<windows::BaseWindow>> m_windows{
                                                                        m_IOWindow,
                                                                        m_programWindow,
                                                                        m_myRobotWindow,
                                                                        m_moveWindow,
                                                                        m_plottingWindow,
                                                                        m_viewportWindow,
                                                                        m_sceneGraphWindow,
                                                                        m_logWindow,
    };

    CSimpleIniA iniGUISettings;
    CSimpleIniA iniProject;

    void showFrameOnViewport(sofaglfw::SofaGLFWBaseGUI *baseGUI);
    void initDockSpace(const bool& firstTime);
    void showViewportWindow(sofaglfw::SofaGLFWBaseGUI* baseGUI);
    void showOptionWindows(sofaglfw::SofaGLFWBaseGUI* baseGUI);
    void showMainMenuBar(sofaglfw::SofaGLFWBaseGUI* baseGUI);
    void showStatusBar();
    void applyDarkMode(const bool &darkMode, sofaglfw::SofaGLFWBaseGUI* baseGUI=nullptr);

    void saveSettings();
    void loadSimulation(const bool& reload, const std::string &filename);
    void enableWindows();
    void clearGUI();
    void setDockSizeFromFile(const ImGuiID& id);

    models::IPController::SPtr m_IPController;
    models::SimulationState m_simulationState;
    bool m_animate{false};
    int m_mode{0};
    bool m_darkMode{false};
    sofaglfw::SofaGLFWBaseGUI* m_baseGUI{nullptr};
    std::vector<ImGuiID> m_dockIDs;
};

} // namespace sofaimgui
