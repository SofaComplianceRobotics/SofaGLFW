/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2021 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/

#include "Module_SofaImGui.h"

#include <pybind11/stl.h>
#include <pybind11/cast.h>

#include <SofaImGui/init.h>
#include <Binding_IOWindow.h>
#include <Binding_MoveWindow.h>
#include <Binding_MyRobotWindow.h>
#include <Binding_PlottingWindow.h>
#include <Binding_ProgramWindow.h>
#include <Binding_SimulationState.h>

#include <sofa/gui/common/GUIManager.h>
#include <sofa/core/behavior/BaseMechanicalState.h>

#include <SofaImGui/ImGuiGUI.h>
#include <SofaImGui/ImGuiGUIEngine.h>
#include <SoftRobots.Inverse/component/solver/QPInverseProblemSolver.h>
#include <sofa/component/constraint/lagrangian/solver/ConstraintSolverImpl.h>


namespace py { using namespace pybind11; }

namespace sofaimgui::python3
{

void setIPController(sofa::simulation::Node &TCPTargetNode,
                     sofa::simulation::Node &TCPNode,
                     sofa::component::constraint::lagrangian::solver::ConstraintSolverImpl &solver)
{
    msg_deprecated("SofaImGui.setIPController") << "This method is deprecated, use setInverseProblemSolver, addTCP, and addActuator instead.";

    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;
    softrobotsinverse::solver::QPInverseProblemSolver::SPtr qpsolver = dynamic_cast<softrobotsinverse::solver::QPInverseProblemSolver*>(&solver);

    if (engine && qpsolver)
    {
        sofa::simulation::Node::SPtr groot = dynamic_cast<sofa::simulation::Node*>(TCPTargetNode.getRoot());

        // Find the PositionEffector component corresponding to the rotation if any
        sofa::type::vector<softrobotsinverse::constraint::PositionEffector<sofa::defaulttype::Rigid3dTypes> *> effectors;
        groot->getContext()->getObjects(effectors, sofa::core::objectmodel::BaseContext::SearchDirection::SearchRoot);
        // softrobotsinverse::constraint::PositionEffector<sofa::defaulttype::Rigid3dTypes>* rotationEffector{ nullptr };

        for (auto* effector: effectors)
        {
            auto useDirections = effector->d_useDirections.getValue();
            if (useDirections[0] || useDirections[1] || useDirections[2])
                continue;
            // rotationEffector = effector;
            break;
        }

        engine->m_kinematicsGUIDataManager.setInverseProblemSolver(qpsolver);
    }
}

void setInverseProblemSolver(sofa::component::constraint::lagrangian::solver::ConstraintSolverImpl* solver)
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;
    softrobotsinverse::solver::QPInverseProblemSolver::SPtr qpsolver = dynamic_cast<softrobotsinverse::solver::QPInverseProblemSolver*>(solver);

    if (engine && qpsolver)
    {
        engine->m_kinematicsGUIDataManager.setInverseProblemSolver(qpsolver);
    }
}

void addTCP(softrobots::behavior::SoftRobotsBaseConstraint *constraint, py::object min, py::object max, const std::string &group, const std::string& help)
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;

    if (engine)
    {
        if (constraint && constraint->m_constraintType == softrobots::behavior::SoftRobotsBaseConstraint::EFFECTOR)
        {
            engine->m_kinematicsGUIDataManager.addTCP(constraint,
                                                       getDataFromPyObject(min, "float"),
                                                       getDataFromPyObject(max, "float"),
                                                       group,
                                                       help);
        } else {
            msg_error("[addTCP]") << "Expects a PositionEffector component as the first parameter.";
        }
    }
}

void addActuator(std::string label, softrobots::behavior::SoftRobotsBaseConstraint *constraint, py::object min, py::object max, const std::string &group, const std::string& help)
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;

    if (engine)
    {
        if(constraint && constraint->m_constraintType == softrobots::behavior::SoftRobotsBaseConstraint::ACTUATOR)
        {
            engine->m_kinematicsGUIDataManager.addActuator(label,
                                                            constraint,
                                                            getDataFromPyObject(min, "float"),
                                                            getDataFromPyObject(max, "float"),
                                                            group,
                                                            help);
        } else {
            msg_error("[addTCP]") << "Expects an Actuator component as the first parameter.";
        }
    }
}

void addAccessoryComponent(std::string accessoryLabel, std::string componentLabel, softrobots::behavior::SoftRobotsBaseConstraint *constraint, py::object min, py::object max, const std::string &group, const std::string& help)
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;

    if (engine)
    {
        if (constraint && (constraint->m_constraintType == softrobots::behavior::SoftRobotsBaseConstraint::ACTUATOR || constraint->m_constraintType == softrobots::behavior::SoftRobotsBaseConstraint::EFFECTOR))
        {
            engine->m_kinematicsGUIDataManager.addAccessoryComponent(accessoryLabel,
                                                                  componentLabel,
                                                                  constraint,
                                                                  getDataFromPyObject(min, "float"),
                                                                  getDataFromPyObject(max, "float"),
                                                                  group,
                                                                  help);
        }
        else
        {
            msg_error("[addTCP]") << "Expects either a PositionEffector of Actuator component as the third parameter.";
        }
    }
}

bool getRobotConnectionToggle()
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;

    if (engine)
        return engine->getRobotConnection();

    return false;
}

void setRobotConnectionToggle(const bool& robotConnectionToggle)
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;

    if (engine)
        engine->setRobotConnection(robotConnectionToggle);
}

PYBIND11_MODULE(ImGui, m)
{
    // Deprecated
    m.def("setIPController", &setIPController);

    m.def("setInverseProblemSolver", &setInverseProblemSolver);
    m.def("addTCP", &addTCP);
    m.def("addActuator", &addActuator);
    m.def("addAccessoryComponent", &addAccessoryComponent);

    m.def("getRobotConnectionToggle", &getRobotConnectionToggle);
    m.def("setRobotConnectionToggle", &setRobotConnectionToggle);

    moduleAddIOWindow(m);
    moduleAddMoveWindow(m);
    moduleAddMyRobotWindow(m);
    moduleAddPlottingWindow(m);
    moduleAddProgramWindow(m);
    moduleAddDataMonitor(m);
}

std::pair<sofa::core::BaseData*, bool> getDataFromPyObject(py::object& obj, std::string type)
{
    if (obj.is_none())
        return std::pair<sofa::core::BaseData*, bool>(nullptr, false);

    if (py::isinstance<sofa::core::objectmodel::BaseData>(obj))
        return std::pair<sofa::core::BaseData*, bool>(py::cast<sofa::core::objectmodel::BaseData*>(obj), false); //sofapython3::addData(py::none(), "Label", obj, py::none(), "", "group", type); //py::cast<sofa::core::objectmodel::BaseData*>(obj);

    sofa::core::BaseData* data = sofapython3::PythonFactory::createInstance(type);
    if (!obj.is_none() and data)
    {
        sofapython3::PythonFactory::fromPython(data, obj);
        return std::pair<sofa::core::BaseData*, bool>(data, true);
    }

    msg_error("Module_SofaImGui") << "Unable to convert py::object " << obj;
    return std::pair<sofa::core::BaseData*, bool>(nullptr, false);
}

} // namespace sofaimgui::python3
