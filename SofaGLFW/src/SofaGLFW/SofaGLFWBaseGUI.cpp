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

#include <filesystem>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <sofa/gui/common/BaseGUI.h>
#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <SofaGLFW/SofaGLFWWindow.h>

#include <sofa/gui/common/PickHandler.h>

#include <sofa/simulation/SimulationLoop.h>
#include <sofa/simulation/Node.h>
#include <sofa/simulation/Simulation.h>

#include <sofa/component/visual/InteractiveCamera.h>
#include <sofa/component/visual/VisualStyle.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/core/objectmodel/BaseClassNameHelper.h>
#include <sofa/core/objectmodel/KeypressedEvent.h>
#include <sofa/core/objectmodel/KeyreleasedEvent.h>

#include <sofa/helper/io/STBImage.h>
#include <sofa/helper/logging/Messaging.h>
#include <sofa/helper/AdvancedTimer.h>
#include <sofa/helper/system/FileRepository.h>
#include <sofa/helper/system/SetDirectory.h>
#include <sofa/helper/system/FileSystem.h>
#include <sofa/helper/Utils.h>

#include <algorithm>
#include <map>


using namespace sofa;

namespace sofaglfw
{

SofaGLFWBaseGUI::SofaGLFWBaseGUI()
{
    m_guiEngine = std::make_shared<NullGUIEngine>();
    m_showSelectedNodeBoundingBox = true;
    m_showSelectedObjectBoundingBox = false;
    m_showSelectedObjectPositions = true;
    m_showSelectedObjectSurfaces = true;
    m_selectionColor = type::RGBAColor(0.439, 0.588, 0.702, 1.);
}

SofaGLFWBaseGUI::~SofaGLFWBaseGUI()
{
    terminate();
}

sofa::core::sptr<sofa::simulation::Node> SofaGLFWBaseGUI::getRootNode() const
{
    return m_groot;
}

bool SofaGLFWBaseGUI::init(int nbMSAASamples)
{
    if (m_bGlfwIsInitialized)
        return true;

    setErrorCallback();

    // on macOS, glfwInit change the current working directory...
    // giving this hint avoids doing the change
#if defined(__APPLE__)
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

    if (glfwInit() == GLFW_TRUE)
    {
        // defined samples for MSAA
        // min = 0  (no MSAA Anti-aliasing)
        // max = 32 (MSAA with 32 samples)
        glfwWindowHint(GLFW_SAMPLES, std::clamp(nbMSAASamples, 0, 32) );
        
        m_glDrawTool = std::make_unique<sofa::gl::DrawToolGL>();
        m_bGlfwIsInitialized = true;
        return true;
    }
    else
    {
        msg_error("SofaGLFWBaseGUI") << "Cannot initialize GLFW";
        return false;
    }
}

void SofaGLFWBaseGUI::setErrorCallback() const
{
    glfwSetErrorCallback(error_callback);
}

void SofaGLFWBaseGUI::setSimulation(sofa::simulation::NodeSPtr groot, const std::string& filename)
{
    m_groot = groot;
    m_filename = filename;

    sofa::core::visual::VisualParams::defaultInstance()->drawTool() = m_glDrawTool.get();
    sofa::core::visual::VisualParams::defaultInstance()->setSupported(sofa::core::visual::API_OpenGL);
    setScene(groot, filename.c_str());
    load();

    if (this->groot)
    {
        // Initialize the pick handler
        this->pick->init(this->groot.get());
        m_sofaGLFWMouseManager.setPickHandler(getPickHandler());
    }
}

void SofaGLFWBaseGUI::setSimulationCanRun(bool canRun)
{
    m_simulationCanRun = canRun;
}

void SofaGLFWBaseGUI::setSimulationIsRunning(bool running)
{
    if (m_simulationCanRun && m_groot)
    {
        m_groot->setAnimate(running);
    }
}

bool SofaGLFWBaseGUI::simulationIsRunning() const
{
    if (m_groot)
    {
        return m_simulationCanRun && m_groot->getAnimate();
    }

    return false;
}

void SofaGLFWBaseGUI::setSizeW(int width)
{
    m_windowWidth = width;
}

void SofaGLFWBaseGUI::setSizeH(int height)
{
    m_windowHeight = height;
}

int SofaGLFWBaseGUI::getWidth()
{
    return m_windowWidth;
}

int SofaGLFWBaseGUI::getHeight()
{
    return m_windowHeight;
}

void SofaGLFWBaseGUI::redraw()
{
}

void SofaGLFWBaseGUI::drawScene()
{
}

void SofaGLFWBaseGUI::viewAll()
{
}

void SofaGLFWBaseGUI::saveView()
{
}

sofa::component::visual::BaseCamera::SPtr SofaGLFWBaseGUI::findCamera(sofa::simulation::NodeSPtr groot)
{
    sofa::component::visual::BaseCamera::SPtr camera;
    groot->get(camera);
    if (!camera)
    {
        camera = sofa::core::objectmodel::New<component::visual::InteractiveCamera>();
        camera->setName(core::objectmodel::Base::shortName(camera.get()));
        m_groot->addObject(camera);
        camera->bwdInit();
    }

    camera->setBoundingBox(m_groot->f_bbox.getValue().minBBox(), m_groot->f_bbox.getValue().maxBBox());

    return camera;
}

void SofaGLFWBaseGUI::changeCamera(sofa::component::visual::BaseCamera::SPtr newCamera)
{
    for (auto& w : s_mapWindows)
    {
        w.second->setCamera(newCamera);
    }
}

void SofaGLFWBaseGUI::setWindowIcon(GLFWwindow* glfwWindow)
{
    //STBImage relies on DataRepository to find files: it must be extended with the resource files from this plugin
    sofa::helper::system::DataRepository.addFirstPath(SOFAGLFW_RESOURCES_DIR);

    sofa::helper::io::STBImage img;
    if (img.load("SOFA.png"))
    {
        GLFWimage images[1];
        images[0].height = img.getHeight();
        images[0].width = img.getWidth();
        images[0].pixels = img.getPixels();
        glfwSetWindowIcon(glfwWindow, 1, images);
    }
    sofa::helper::system::DataRepository.removePath(SOFAGLFW_RESOURCES_DIR);
}

bool SofaGLFWBaseGUI::createWindow(int width, int height, const char* title, bool fullscreenAtStartup)
{
    m_guiEngine->init();

    if (m_groot == nullptr)
    {
        msg_error("SofaGLFWBaseGUI") << "No simulation root has been defined. Quitting.";
        return false;
    }

    GLFWwindow* glfwWindow = nullptr;
    if (fullscreenAtStartup)
    {
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

        m_lastWindowWidth = width;
        m_lastWindowHeight = height;
        m_lastWindowPositionX = 100;
        m_lastWindowPositionY = 100;

        glfwWindow = glfwCreateWindow(mode->width, mode->height, title, primaryMonitor, m_firstWindow);
    }
    else
    {
        glfwWindow = glfwCreateWindow(width > 0 ? width : 100, height > 0 ? height : 100, title, nullptr, m_firstWindow);
    }
    assert(glfwWindow);

    setWindowIcon(glfwWindow);

    if (!m_firstWindow)
        m_firstWindow = glfwWindow;

    if (glfwWindow)
    {
        glfwSetKeyCallback(glfwWindow, key_callback);
        glfwSetCursorPosCallback(glfwWindow, cursor_position_callback);
        glfwSetMouseButtonCallback(glfwWindow, mouse_button_callback);
        glfwSetScrollCallback(glfwWindow, scroll_callback);
        glfwSetWindowCloseCallback(glfwWindow, close_callback);
        glfwSetWindowPosCallback(glfwWindow, window_pos_callback);

        // this set empty callbacks
        // solve a crash when glfw is quitting and tries to use nullptr callbacks
        // could be potentially useful in the future anyway
        glfwSetWindowFocusCallback(glfwWindow, window_focus_callback);
        glfwSetCursorEnterCallback(glfwWindow, cursor_enter_callback);
        glfwSetMonitorCallback(monitor_callback);
        glfwSetCharCallback(glfwWindow, character_callback);
        glfwSetWindowUserPointer(glfwWindow, this);

        makeCurrentContext(glfwWindow);

        m_guiEngine->initBackend(glfwWindow);

        auto camera = findCamera(m_groot);
        
        SofaGLFWWindow* sofaWindow = new SofaGLFWWindow(glfwWindow, camera);

        s_mapWindows[glfwWindow] = sofaWindow;
        s_mapGUIs[glfwWindow] = this;

        return true;
    }

    return false;
}

void SofaGLFWBaseGUI::updateViewportPosition(const float viewportPositionX, const float viewportPositionY)
{
    m_viewPortPosition = { viewportPositionX, viewportPositionY };
}

void SofaGLFWBaseGUI::resizeWindow(int width, int height)
{
    if (hasWindow())
    {
        glfwSetWindowSize(m_firstWindow, width, height);
    }
}

void SofaGLFWBaseGUI::destroyWindow()
{
}

GLFWmonitor* SofaGLFWBaseGUI::getCurrentMonitor(GLFWwindow *glfwWindow)
{
    int monitorsCount, i;
    int windowsX, windowsY, windowsWidth, windowsHeight;
    int monitorX, monitorY, monitorWidth, monitorHeight;
    int overlap, bestOverlap;
    GLFWmonitor *bestMonitor;
    GLFWmonitor **monitors;
    const GLFWvidmode *mode;

    bestOverlap = 0;
    bestMonitor = nullptr;

    glfwGetWindowPos(glfwWindow, &windowsX, &windowsY);
    glfwGetWindowSize(glfwWindow, &windowsWidth, &windowsHeight);
    monitors = glfwGetMonitors(&monitorsCount);

    for (i=0; i<monitorsCount; i++)
    {
        mode = glfwGetVideoMode(monitors[i]);
        glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);
        monitorWidth = mode->width;
        monitorHeight = mode->height;

        overlap = std::max(0, std::min(windowsX + windowsWidth, monitorX + monitorWidth) - std::max(windowsX, monitorX)) *
                  std::max(0, std::min(windowsY + windowsHeight, monitorY + monitorHeight) - std::max(windowsY, monitorY));

        if (bestOverlap < overlap)
        {
            bestOverlap = overlap;
            bestMonitor = monitors[i];
        }
    }

    return bestMonitor;
}

bool SofaGLFWBaseGUI::isFullScreen(GLFWwindow* glfwWindow) const
{
    if (hasWindow())
    {
        glfwWindow = (!glfwWindow) ? m_firstWindow : glfwWindow;
        return glfwGetWindowMonitor(glfwWindow) != nullptr;
    }
    return false;
}

void SofaGLFWBaseGUI::switchFullScreen(GLFWwindow* glfwWindow, unsigned int /* screenID */)
{
    if (hasWindow())
    {
        // only manage the first window for now
        // and the main screen
        glfwWindow = (!glfwWindow) ? m_firstWindow : glfwWindow;

        bool isFullScreen = glfwGetWindowMonitor(glfwWindow) != nullptr;

        if (!isFullScreen)
        {
            // backup window position and window size
            glfwGetWindowPos(glfwWindow, &m_lastWindowPositionX, &m_lastWindowPositionY);
            glfwGetWindowSize(glfwWindow, &m_lastWindowWidth, &m_lastWindowHeight);

            GLFWmonitor* monitor = getCurrentMonitor(glfwWindow);
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            glfwSetWindowMonitor(glfwWindow, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
        }
        else
        {
            glfwSetWindowAttrib(glfwWindow, GLFW_DECORATED, GLFW_TRUE);
            glfwSetWindowMonitor(glfwWindow, nullptr, m_lastWindowPositionX, m_lastWindowPositionY, m_lastWindowWidth, m_lastWindowHeight, GLFW_DONT_CARE);
        }
    }
    else
    {
        msg_error("SofaGLFWBaseGUI") << "No window to set fullscreen"; // can happen with runSofa/BaseGUI
    }
}

void SofaGLFWBaseGUI::setWindowBackgroundColor(const sofa::type::RGBAColor& newColor, unsigned int /* windowID */)
{
    // only manage the first window for now
    if (hasWindow())
    {
        s_mapWindows[m_firstWindow]->setBackgroundColor(newColor);
    }
    else
    {
        msg_error("SofaGLFWBaseGUI") << "No window to set the background in";// can happen with runSofa/BaseGUI
    }
}


void SofaGLFWBaseGUI::setWindowBackgroundImage(const std::string& filename, unsigned int /* windowID */)
{
    SOFA_UNUSED(filename);
}

void SofaGLFWBaseGUI::setWindowTitle(GLFWwindow* window, const char* title)
{
    if(hasWindow())
    {
        auto* glfwWindow = (window) ? window : m_firstWindow ;
        glfwSetWindowTitle(glfwWindow, title);
    }
    else
    {
        msg_error("SofaGLFWBaseGUI") << "No window to set the title on"; // can happen with runSofa/BaseGUI
    }
}

void SofaGLFWBaseGUI::makeCurrentContext(GLFWwindow* glfwWindow)
{
    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval( 0 ); //request disabling vsync
    if (!m_bGlewIsInitialized)
    {
        glewInit();
        m_bGlewIsInitialized = true;
    }
}

std::size_t SofaGLFWBaseGUI::runLoop(std::size_t targetNbIterations)
{
    if (!m_groot)
    {
        msg_error("SofaGLFWBaseGUI") << "Cannot start main loop: root node is invalid";
        return 0;
    }

    m_vparams = sofa::core::visual::VisualParams::defaultInstance();
    m_viewPortWidth = m_vparams->viewport()[2];
    m_viewPortHeight = m_vparams->viewport()[3];

    bool running = true;
    std::size_t currentNbIterations = 0;
    std::stringstream tmpStr;
    std::vector<uint8_t> pixels;

    while (!s_mapWindows.empty() && running)
    {
        SIMULATION_LOOP_SCOPE

        // Keep running
        runStep();

        for (auto& [glfwWindow, sofaGlfwWindow] : s_mapWindows)
        {
            if (sofaGlfwWindow)
            {
                // while user did not request to close this window (i.e press escape), draw
                if (!glfwWindowShouldClose(glfwWindow))
                {
                    makeCurrentContext(glfwWindow);

                    m_guiEngine->beforeDraw(glfwWindow);
                    sofaGlfwWindow->draw(m_groot, m_vparams);
                    drawSelection(m_vparams);

                    m_guiEngine->afterDraw();

                    m_guiEngine->startFrame(this);
                    m_guiEngine->endFrame();

                    m_viewPortHeight = m_vparams->viewport()[3];
                    m_viewPortWidth = m_vparams->viewport()[2];

                    // Read framebuffer
                    if(this->groot->getAnimate() && this->m_isVideoRecording)
                    {
                        const auto [width, height] = this->m_guiEngine->getFrameBufferPixels(pixels);
                        m_videoRecorderFFMPEG.addFrame(pixels.data(), width, height);
                    }

                    glfwSwapBuffers(glfwWindow);
                }
                else
                {
                    // otherwise close this window
                    close_callback(glfwWindow);
                }
            }
        }

        glfwPollEvents();

        currentNbIterations++;
        running = (targetNbIterations > 0) ? currentNbIterations < targetNbIterations : true;
    }

    return currentNbIterations;
}

void SofaGLFWBaseGUI::initVisual()
{
    sofa::simulation::node::initTextures(m_groot.get());

    component::visual::VisualStyle::SPtr visualStyle = nullptr;
    m_groot->get(visualStyle);
    if (!visualStyle)
    {
        visualStyle = sofa::core::objectmodel::New<component::visual::VisualStyle>();
        visualStyle->setName(sofa::core::objectmodel::BaseClassNameHelper::getShortName<decltype(visualStyle.get())>());

        core::visual::DisplayFlags* displayFlags = visualStyle->d_displayFlags.beginEdit();
        displayFlags->setShowVisualModels(sofa::core::visual::tristate::true_value);
        visualStyle->d_displayFlags.endEdit();

        m_groot->addObject(visualStyle);
        visualStyle->init();
    }

    //init gl states
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);
    glEnable(GL_NORMALIZE);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Setup 'light 0'
    float lightAmbient[4] = { 0.5f, 0.5f, 0.5f,1.0f };
    float lightDiffuse[4] = { 0.9f, 0.9f, 0.9f,1.0f };
    float lightSpecular[4] = { 1.0f, 1.0f, 1.0f,1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
   
    // Enable color tracking
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // All materials hereafter have full specular reflectivity with a high shine
    float materialSpecular[4] = { 1.0f, 1.0f, 1.0f,1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMateriali(GL_FRONT, GL_SHININESS, 128);

    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHT0);

    m_vparams = sofa::core::visual::VisualParams::defaultInstance();
    for (auto& [glfwWindow, sofaGlfwWindow] : s_mapWindows)
    {
        sofaGlfwWindow->centerCamera(m_groot, m_vparams);
    }
}

void SofaGLFWBaseGUI::runStep()
{
    if(simulationIsRunning() && m_guiEngine && m_groot)
    {
        sofa::helper::AdvancedTimer::begin("Animate");

        m_guiEngine->animateBeginEvent(m_groot.get());
        simulation::node::animate(m_groot.get(), m_groot->getDt());
        m_guiEngine->animateEndEvent(m_groot.get());
        simulation::node::updateVisual(m_groot.get());

        sofa::helper::AdvancedTimer::end("Animate");
    }
}

void SofaGLFWBaseGUI::terminate()
{
    if (!m_bGlfwIsInitialized)
        return;

    if (m_guiEngine)
        m_guiEngine->terminate();

    if(m_isVideoRecording)
        m_videoRecorderFFMPEG.finishVideo();

    glfwTerminate();
}

void SofaGLFWBaseGUI::error_callback(int error, const char* description)
{
    SOFA_UNUSED(error);
    msg_error("SofaGLFWBaseGUI") << "Error: " << description << ".";
}

int SofaGLFWBaseGUI::handleArrowKeys(int key)
{
    // Handling arrow keys with custom codes
    switch (key)
    {
            case GLFW_KEY_UP: return 19;   // Custom code for up
            case GLFW_KEY_DOWN: return 21; // Custom code for down
            case GLFW_KEY_LEFT: return 18; // Custom code for left
            case GLFW_KEY_RIGHT: return 20; // Custom code for right
    }
    // Default case return the given value as GLFW handle it
    return key;
}

void SofaGLFWBaseGUI::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const char sofaKey = SofaGLFWBaseGUI::handleArrowKeys(key);
    const bool isCtrlKeyPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
    const bool isAltKeyPressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT ) == GLFW_PRESS;

    auto currentGUI = s_mapGUIs.find(window);
    if (currentGUI == s_mapGUIs.end() || currentGUI->second == nullptr)
    {
        return;
    }
    currentGUI->second->getGUIEngine()->key_callback(window, key, scancode, action, mods);

    auto rootNode = currentGUI->second->getRootNode();
    if (!rootNode)
    {
        return;
    }

    if (isCtrlKeyPressed && isAltKeyPressed)
    {
        if (action == GLFW_PRESS)
        {
            sofa::core::objectmodel::KeypressedEvent keyPressedEvent(sofaKey);
            rootNode->propagateEvent(sofa::core::ExecParams::defaultInstance(), &keyPressedEvent);
        }
        else if (action == GLFW_RELEASE)
        {
            sofa::core::objectmodel::KeyreleasedEvent keyReleasedEvent(sofaKey);
            rootNode->propagateEvent(sofa::core::ExecParams::defaultInstance(), &keyReleasedEvent);
        }
    }

    // Handle specific keys for additional functionality
    switch (key)
    {
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS)
            {
                const bool isRunning = currentGUI->second->simulationIsRunning();
                currentGUI->second->setSimulationIsRunning(!isRunning);
            }
            break;
        case GLFW_KEY_F11:
            if (action == GLFW_PRESS)
            {
                currentGUI->second->switchFullScreen(window);
            }
            break;
        case GLFW_KEY_LEFT_SHIFT:
            if (currentGUI->second->isMouseInteractionEnabled() && currentGUI->second->getPickHandler())
            {
                if (action == GLFW_PRESS)
                {
                    currentGUI->second->getPickHandler()->activateRay(0, 0, rootNode.get());
                }
                else if (action == GLFW_RELEASE)
                {
                    currentGUI->second->getPickHandler()->deactivateRay();
                }
            }
            break;
        default:
            break;
    }

    // Handle specific keyName for additional functionality for layout dependent keys
    const char* keyName = glfwGetKeyName(key, scancode);
    if (keyName)
    {
        if (isCtrlKeyPressed && !isAltKeyPressed)
        {
            if (strcmp(keyName, "q") == 0)
            {
                if (action == GLFW_PRESS )
                {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
            }
        }
    }
}

void SofaGLFWBaseGUI::moveRayPickInteractor(int eventX, int eventY)
{
    const sofa::core::visual::VisualParams::Viewport& viewport = m_vparams->viewport();

    double lastProjectionMatrix[16];
    double lastModelviewMatrix[16];

    m_vparams->getProjectionMatrix(lastProjectionMatrix);
    m_vparams->getModelViewMatrix(lastModelviewMatrix);

    sofa::type::Vec3d p0;
    sofa::type::Vec3d px;
    sofa::type::Vec3d py;
    sofa::type::Vec3d pz;
    sofa::type::Vec3d px1;
    sofa::type::Vec3d py1;
    gluUnProject(eventX,   viewport[3]-1-(eventY),   0,   lastModelviewMatrix, lastProjectionMatrix, viewport.data(), &(p0[0]),  &(p0[1]),  &(p0[2]));
    gluUnProject(eventX+1, viewport[3]-1-(eventY),   0,   lastModelviewMatrix, lastProjectionMatrix, viewport.data(), &(px[0]),  &(px[1]),  &(px[2]));
    gluUnProject(eventX,   viewport[3]-1-(eventY+1), 0,   lastModelviewMatrix, lastProjectionMatrix, viewport.data(), &(py[0]),  &(py[1]),  &(py[2]));
    gluUnProject(eventX,   viewport[3]-1-(eventY),   0.1, lastModelviewMatrix, lastProjectionMatrix, viewport.data(), &(pz[0]),  &(pz[1]),  &(pz[2]));
    gluUnProject(eventX+1, viewport[3]-1-(eventY),   0.1, lastModelviewMatrix, lastProjectionMatrix, viewport.data(), &(px1[0]), &(px1[1]), &(px1[2]));
    gluUnProject(eventX,   viewport[3]-1-(eventY+1), 0,   lastModelviewMatrix, lastProjectionMatrix, viewport.data(), &(py1[0]), &(py1[1]), &(py1[2]));

    px1 -= pz;
    py1 -= pz;
    px -= p0;
    py -= p0;
    pz -= p0;

    const double r0 = sqrt(px.norm2() + py.norm2());
    double r1 = sqrt(px1.norm2() + py1.norm2());
    r1 = r0 + (r1 - r0) / pz.norm();
    px.normalize();
    py.normalize();
    pz.normalize();

    sofa::type::Mat4x4d transform;
    transform.identity();
    transform[0][0] = px[0];
    transform[1][0] = px[1];
    transform[2][0] = px[2];
    transform[0][1] = py[0];
    transform[1][1] = py[1];
    transform[2][1] = py[2];
    transform[0][2] = pz[0];
    transform[1][2] = pz[1];
    transform[2][2] = pz[2];
    transform[0][3] = p0[0];
    transform[1][3] = p0[1];
    transform[2][3] = p0[2];

    sofa::type::Mat3x3d mat;
    mat = transform;
    sofa::type::Quat<SReal> q;
    q.fromMatrix(mat);

    sofa::type::Vec3d position, direction;
    position = transform * sofa::type::Vec4d(0, 0, 0, 1);
    direction = transform * sofa::type::Vec4d(0, 0, 1, 0);
    direction.normalize();
    getPickHandler()->updateRay(position, direction);
}

void SofaGLFWBaseGUI::setMousePos(int xpos, int ypos) {
    if(m_firstWindow)
    {
        glfwSetInputMode(m_firstWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // Required on Wayland
        glfwSetCursorPos(m_firstWindow, xpos, ypos);
        glfwSetInputMode(m_firstWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void SofaGLFWBaseGUI::window_pos_callback(GLFWwindow* window, int xpos, int ypos)
{
    SofaGLFWBaseGUI* gui = static_cast<SofaGLFWBaseGUI*>(glfwGetWindowUserPointer(window));
    gui->m_windowPosition[0] = static_cast<float>(xpos);
    gui->m_windowPosition[1] = static_cast<float>(ypos);
}

void SofaGLFWBaseGUI::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    auto currentGUI = s_mapGUIs.find(window);

    if (currentGUI != s_mapGUIs.end() && currentGUI->second)
    {
        if (!currentGUI->second->getGUIEngine()->dispatchMouseEvents())
            return;
    }
    SofaGLFWBaseGUI* gui = currentGUI->second;

    translateToViewportCoordinates(gui,xpos,ypos);

    bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;


    auto currentSofaWindow = s_mapWindows.find(window);

    if (shiftPressed)
    {
        for (const auto button : {GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_MOUSE_BUTTON_RIGHT})
        {
            if (glfwGetMouseButton(window, button) == GLFW_PRESS)
            {
                currentSofaWindow->second->mouseEvent(window,gui->m_viewPortWidth,gui->m_viewPortHeight, button, 1, 1, gui->m_translatedCursorPos[0], gui->m_translatedCursorPos[1]);
            }
        }
    }

    if (currentSofaWindow != s_mapWindows.end() && currentSofaWindow->second)
    {
        currentSofaWindow->second->mouseMoveEvent(static_cast<int>(xpos), static_cast<int>(ypos), currentGUI->second);
    }
}

void SofaGLFWBaseGUI::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    auto currentGUI = s_mapGUIs.find(window);
    if (currentGUI == s_mapGUIs.end() || !currentGUI->second) {
        return;
    }

    SofaGLFWBaseGUI* gui = currentGUI->second;

    auto rootNode = currentGUI->second->getRootNode();
    if (!rootNode) {
        return;
    }

    const bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    if (shiftPressed )
    {
        // Check if the animation is running
        if (!currentGUI->second->simulationIsRunning())
        {
            msg_info("SofaGLFWBaseGUI") << "Animation is not running. Ignoring mouse interaction.";
            return;
        }

        const auto currentSofaWindow = s_mapWindows.find(window);
        if (currentSofaWindow != s_mapWindows.end() && currentSofaWindow->second)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            translateToViewportCoordinates(gui,xpos,ypos);

            currentSofaWindow->second->mouseEvent(
                window, gui->m_viewPortWidth, gui->m_viewPortHeight, button,
                action, mods,
                gui->m_translatedCursorPos[0],
                gui->m_translatedCursorPos[1]);
        }
    }
    else
    {
        if (!currentGUI->second->getGUIEngine()->dispatchMouseEvents())
            return;

        auto currentSofaWindow = s_mapWindows.find(window);
        if (currentSofaWindow != s_mapWindows.end() && currentSofaWindow->second)
        {
            currentSofaWindow->second->mouseButtonEvent(button, action, mods);
        }
    }
}

void SofaGLFWBaseGUI::translateToViewportCoordinates (SofaGLFWBaseGUI* gui,double xpos, double ypos)
{
    gui->m_translatedCursorPos = sofa::type::Vec2d{xpos, ypos} - (gui->m_viewPortPosition - gui->m_windowPosition);
}

void SofaGLFWBaseGUI::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto currentGUI = s_mapGUIs.find(window);
    if (currentGUI != s_mapGUIs.end() && currentGUI->second)
    {
        if (!currentGUI->second->getGUIEngine()->dispatchMouseEvents())
            return;
    }
    
    auto currentSofaWindow = s_mapWindows.find(window);
    if (currentSofaWindow != s_mapWindows.end() && currentSofaWindow->second)
    {
        currentSofaWindow->second->scrollEvent(xoffset, yoffset);
    }
}

void SofaGLFWBaseGUI::close_callback(GLFWwindow* window)
{
    auto currentSofaWindow = s_mapWindows.find(window);
    if (currentSofaWindow != s_mapWindows.end())
    {
        if (SofaGLFWWindow* glfwWindow = currentSofaWindow->second)
        {
            glfwWindow->close();
            delete glfwWindow;
        }
        s_mapWindows.erase(window);
    }
}

void SofaGLFWBaseGUI::window_focus_callback(GLFWwindow* window, int focused)
{
    SOFA_UNUSED(window);
    SOFA_UNUSED(focused);
    //if (focused)
    //{
    //    // The window gained input focus
    //}
    //else
    //{
    //    // The window lost input focus
    //}
}

void SofaGLFWBaseGUI::cursor_enter_callback(GLFWwindow* window, int entered)
{
    SOFA_UNUSED(window);
    SOFA_UNUSED(entered);

    //if (entered)
    //{
    //    // The cursor entered the content area of the window
    //}
    //else
    //{
    //    // The cursor left the content area of the window
    //}
}

void SofaGLFWBaseGUI::monitor_callback(GLFWmonitor* monitor, int event)
{
    SOFA_UNUSED(monitor);
    SOFA_UNUSED(event);

    //if (event == GLFW_CONNECTED)
    //{
    //    // The monitor was connected
    //}
    //else if (event == GLFW_DISCONNECTED)
    //{
    //    // The monitor was disconnected
    //}
}

void SofaGLFWBaseGUI::character_callback(GLFWwindow* window, unsigned int codepoint)
{
    SOFA_UNUSED(window);
    SOFA_UNUSED(codepoint);

    // The callback function receives Unicode code points for key events
    // that would have led to regular text input and generally behaves as a standard text field on that platform.
}

bool SofaGLFWBaseGUI::centerWindow(GLFWwindow* window)
{
    if (hasWindow())
    {
        // only manage the first window for now
        // and the main screen
        window = (!window) ? m_firstWindow : window;
    }

    if (!window)
        return false;

    int sx = 0, sy = 0;
    int px = 0, py = 0;
    int mx = 0, my = 0;
    int monitor_count = 0;
    int best_area = 0;
    int final_x = 0, final_y = 0;

    glfwGetWindowSize(window, &sx, &sy);
    glfwGetWindowPos(window, &px, &py);

    // Iterate throug all monitors
    GLFWmonitor** m = glfwGetMonitors(&monitor_count);
    if (!m)
        return false;

    for (int j = 0; j < monitor_count; ++j)
    {

        glfwGetMonitorPos(m[j], &mx, &my);
        const GLFWvidmode* mode = glfwGetVideoMode(m[j]);
        if (!mode)
            continue;

        // Get intersection of two rectangles - screen and window
        const int minX = std::max(mx, px);
        const int minY = std::max(my, py);

        const int maxX = std::min(mx + mode->width, px + sx);
        const int maxY = std::min(my + mode->height, py + sy);

        // Calculate area of the intersection
        const int area = std::max(maxX - minX, 0) * std::max(maxY - minY, 0);

        // If its bigger than actual (window covers more space on this monitor)
        if (area > best_area)
        {
            // Calculate proper position in this monitor
            final_x = mx + (mode->width - sx) / 2;
            final_y = my + (mode->height - sy) / 2;

            best_area = area;
        }

    }

    // We found something
    if (best_area)
        glfwSetWindowPos(window, final_x, final_y);

    // Something is wrong - current window has NOT any intersection with any monitors. Move it to the default one.
    else
    {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        if (primary)
        {
            const GLFWvidmode* desktop = glfwGetVideoMode(primary);

            if (desktop)
                glfwSetWindowPos(window, (desktop->width - sx) / 2, (desktop->height - sy) / 2);
            else
                return false;
        }
        else
            return false;
    }

    return true;
}

bool SofaGLFWBaseGUI::toggleVideoRecording()
{
    if(m_isVideoRecording)
    {
        m_isVideoRecording = false;
        m_videoRecorderFFMPEG.finishVideo();
    }
    else
    {
        // Initialize recorder with default parameters
        const int width = std::max(1, m_viewPortWidth);
        const int height = std::max(1, m_viewPortHeight);

        if(initRecorder(width, height))
        {
            m_isVideoRecording = true;
        }
        else
        {
            msg_error("SofaGLFWBaseGUI") << "Failed to initialize recorder.";
            return false;
        }
    }
    return true;
}

bool SofaGLFWBaseGUI::initRecorder(int width,
                                   int height,
                                   unsigned int framerate,
                                   unsigned int bitrate,
                                   const std::string& codecExtension,
                                   const std::string& codecName)
{
    // Validate parameters
    if (width <= 0 || height <= 0)
    {
        msg_error("SofaGLFWBaseGUI") << "Invalid video dimensions: " << width << "x" << height;
        return false;
    }

    std::string ffmpeg_exec_path = "";
    const std::string ffmpegIniFilePath = sofa::helper::Utils::getSofaPathTo("etc/SofaGLFW.ini");
    std::map<std::string, std::string> iniFileValues = sofa::helper::Utils::readBasicIniFile(ffmpegIniFilePath);
    if (iniFileValues.find("FFMPEG_EXEC_PATH") != iniFileValues.end())
    {
        // get absolute path of FFMPEG executable
        ffmpeg_exec_path = sofa::helper::system::SetDirectory::GetRelativeFromProcess(iniFileValues["FFMPEG_EXEC_PATH"].c_str());
        msg_info("SofaGLFWBaseGUI") << " The file " << ffmpegIniFilePath << " points to " << ffmpeg_exec_path << " for the ffmpeg executable.";
    }
    else
    {
        msg_warning("SofaGLFWBaseGUI") << " The file " << helper::Utils::getSofaPathPrefix() <<"/etc/SofaGLFW.ini either doesn't exist or doesn't contain the string FFMPEG_EXEC_PATH."
                                                                                               " The initialization of the FFMPEG video recorder will likely fail."
                                                                                               " To fix this, provide a valid path to the ffmpeg executable inside this file using the syntax \"FFMPEG_EXEC_PATH=/usr/bin/ffmpeg\".";
    }

    std::string screenshotPath = sofa::gui::common::BaseGUI::getScreenshotDirectoryPath();
    m_videoFilePath = m_videoFilename.empty()? generateFilename("video", codecExtension): m_videoFilename;
    m_videoFilePath = sofa::helper::system::FileSystem::append(screenshotPath, m_videoFilePath);
    return m_videoRecorderFFMPEG.init(ffmpeg_exec_path, m_videoFilePath, width, height, framerate, bitrate, codecName);
}

bool SofaGLFWBaseGUI::isVideoRecording() const
{
    return m_isVideoRecording;
}

std::string SofaGLFWBaseGUI::generateFilename(const std::string& prefix, const std::string &extension)
{
    std::string filename = getSceneFileName();
    // Add the date and time to the filename
    auto now = std::chrono::system_clock::now();
    auto localTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;

    if (!prefix.empty())
        ss << prefix << "_";

    if (!filename.empty())
    {
        std::filesystem::path path(filename);
        ss << path.filename().replace_extension("").string() << "_";
    }

    ss << std::put_time(std::localtime(&localTime), "%F_%H-%M-%S");

    if (!extension.empty())
        ss << "." << extension;

    filename = ss.str();
    return filename;
}

} // namespace sofaglfw
