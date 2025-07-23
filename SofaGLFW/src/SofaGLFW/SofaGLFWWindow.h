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
#include <SofaGLFW/SofaGLFWBaseGUI.h>
#include <SofaGLFW/config.h>

#include <sofa/simulation/fwd.h>
#include <sofa/component/visual/BaseCamera.h>
#include <sofa/component/visual/VisualGrid.h>

struct GLFWwindow;

namespace sofaglfw
{

using sofa::component::visual::VisualGrid;

class SOFAGLFW_API SofaGLFWWindow
{
public:
    SofaGLFWWindow(GLFWwindow* glfwWindow, sofa::component::visual::BaseCamera::SPtr camera);
    virtual ~SofaGLFWWindow() = default;

    void draw(sofa::simulation::NodeSPtr groot, sofa::core::visual::VisualParams* vparams);
    void close();

    void mouseMoveEvent(sofa::simulation::NodeSPtr groot, int xpos, int ypos);
    void mouseButtonEvent(int button, int action, int mods);
    void scrollEvent(double xoffset, double yoffset);
    void setBackgroundColor(const sofa::type::RGBAColor& newColor);

    void setCamera(sofa::component::visual::BaseCamera::SPtr newCamera);
    void centerCamera(sofa::simulation::NodeSPtr node, sofa::core::visual::VisualParams* vparams) const;

    enum CameraAlignement{TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK};

    struct GridSquareSize{
        constexpr static float METER = 0.1;
        constexpr static float DECIMETER = 1;
        constexpr static float CENTIMETER = 10;
        constexpr static float MILLIMETER = 100;

        static sofa::type::vector<float> getSquareSizes() {return sofa::type::vector{METER, DECIMETER, CENTIMETER, MILLIMETER};}
        static std::string getString(const float& squareSize)
        {
            if (squareSize == METER)
                return "0.1";

            if (squareSize == DECIMETER)
                return "1";

            if (squareSize == CENTIMETER)
                return "10";

            if (squareSize == MILLIMETER)
                return "100";

            return "";
        }
    };

    static void resetSimulationView(sofaglfw::SofaGLFWBaseGUI *baseGUI);
    static void alignCamera(sofa::simulation::NodeSPtr groot, const CameraAlignement &align);
    static void setGridsPlane(sofa::simulation::NodeSPtr groot, const VisualGrid::PlaneType &plane = VisualGrid::PlaneType("y"));

private:
    GLFWwindow* m_glfwWindow{nullptr};
    sofa::component::visual::BaseCamera::SPtr m_currentCamera;
    int m_currentButton{ -1 };
    int m_currentAction{ -1 };
    int m_currentMods{ -1 };
    int m_currentXPos{ -1 };
    int m_currentYPos{ -1 };
    sofa::type::RGBAColor m_backgroundColor{ sofa::type::RGBAColor::black() };
};

} // namespace sofaglfw
