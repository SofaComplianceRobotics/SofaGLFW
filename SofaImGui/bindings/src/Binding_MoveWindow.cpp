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
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>

#include <SofaPython3/Sofa/Core/Binding_Base.h>
#include <Binding_MoveWindow.h>

#include <SofaPython3/PythonFactory.h>
#include <SofaPython3/PythonEnvironment.h>

#include <sofa/gui/common/GUIManager.h>

#include <SofaImGui/ImGuiGUI.h>
#include <SofaImGui/ImGuiGUIEngine.h>

SOFAPYTHON3_BIND_ATTRIBUTE_ERROR()

/// Makes an alias for the pybind11 namespace to increase readability.
namespace py { using namespace pybind11; }

namespace sofaimgui::python3
{

void moduleAddMoveWindow(py::module &m)
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;

    auto m_a = m.def_submodule("MoveWindow", "");
    std::string m_a_name = py::str(m_a.attr("__name__"));
    m_a.def("setTCPDescription",
            [m_a_name](const std::string &positionDescription, const std::string &rotationDescription)
            {
            SOFA_UNUSED(positionDescription);
            SOFA_UNUSED(rotationDescription);
            msg_deprecated(m_a_name) << "setTCPDescription is deprecated and will be removed in future versions. Use Sofa.ImGui.addTCP() instead";
            }, "[DEPRECATED] Use Sofa.ImGui.addTCP() instead. Set the description displayed on the GUI(positionDescription, rotationDescription). Use this to display the right unit."
            );

    m_a.def("setTCPLimits",
            [m_a_name](const float &minPosition, const float &maxPosition, const double &minOrientation, const double &maxOrientation)
            {
            SOFA_UNUSED(minPosition);
            SOFA_UNUSED(maxPosition);
            SOFA_UNUSED(minOrientation);
            SOFA_UNUSED(maxOrientation);
            msg_deprecated(m_a_name) << "setTCPLimits is deprecated and will be removed in future versions. Use Sofa.ImGui.addTCP() instead";
            }, "[DEPRECATED] Use Sofa.ImGui.addTCP() instead. Set the sliders limits."
            );

    m_a.def("setActuatorsDescription",
        [m_a_name](const std::string &description)
        {
        SOFA_UNUSED(description);
        msg_deprecated(m_a_name) << "setActuatorsDescription is deprecated and will be removed in future versions. Use Sofa.ImGui.addActuator() instead";
        }, "[DEPRECATED] Use Sofa.ImGui.addActuator() instead. Set the description displayed on the GUI. Use this to display the right info and unit."
        );

    m_a.def("setActuatorsLimits",
        [m_a_name](const double &min, const double &max)
        {
        SOFA_UNUSED(min);
        SOFA_UNUSED(max);
        msg_deprecated(m_a_name) << "setActuatorsLimits is deprecated and will be removed in future versions. Use Sofa.ImGui.addActuator() instead";
        }, "[DEPRECATED] Use Sofa.ImGui.addActuator() instead. Set the sliders limits for the actuator number 'id'."
        );

    m_a.def("setActuatorLimits",
            [m_a_name](const sofa::Index &id, const double &min, const double &max)
            {
            SOFA_UNUSED(id);
            SOFA_UNUSED(min);
            SOFA_UNUSED(max);
            msg_deprecated(m_a_name) << "setActuatorLimits is deprecated and will be removed in future versions. Use Sofa.ImGui.addActuator() instead";
            }, "[DEPRECATED] Use Sofa.ImGui.addActuator() instead. Set the sliders limits for the actuator number 'id'."
            );

    m_a.def("setActuators",
            [m_a_name, engine](const std::vector<sofa::core::objectmodel::BaseData*> &actuatorsData,
                                const std::vector<size_t> &indicesInProblem,
                                const std::string valueType)
            {
                SOFA_UNUSED(indicesInProblem);
                SOFA_UNUSED(valueType);
                if (engine)
                {
                    int i=0;
                    for (auto data : actuatorsData)
                    {
                        if(data)
                        {
                            py::object min = py::cast(-3.);
                            py::object max = py::cast(3.);

                            softrobots::behavior::SoftRobotsBaseConstraint *constraint = dynamic_cast<softrobots::behavior::SoftRobotsBaseConstraint *>(data->getOwner());
                            if (constraint)
                            {
                                engine->m_kinematicsGUIDataManager->addActuator("M" + std::to_string(i++),
                                                                                constraint,
                                                                                getDataFromPyObject(min, "float"),
                                                                                getDataFromPyObject(max, "float"),
                                                                                "",
                                                                                "");
                            }
                        }
                    }
                }
                msg_deprecated(m_a_name) << "setActuators is deprecated and will be removed in future versions. Use Sofa.ImGui.addActuator() instead";
            }, "[DEPRECATED] Use Sofa.ImGui.addActuator() instead. Set the actuators."
            );

    m_a.def("addAccessory",
        [m_a_name, engine](const std::string &description, sofa::core::BaseData* data,
                 const float& min, const float& max)
        {
            if (engine)
            {
                if (data)
                {
                    py::object pydata = py::cast(data);
                    py::object pymin = py::cast(min);
                    py::object pymax = py::cast(max);
                    engine->m_kinematicsGUIDataManager->addAccessoryFeature("Accessory",
                                                                            description,
                                                                            getDataFromPyObject(pydata, "float"),
                                                                            getDataFromPyObject(pymin, "float"),
                                                                            getDataFromPyObject(pymax, "float"));
                }
            }


            msg_deprecated(m_a_name) << "addAccessory is deprecated and will be removed in future versions. Use Sofa.ImGui.addAccessoryFeature() instead";
        }, "[DEPRECATED] Use Sofa.ImGui.addAccessoryFeature() instead. Add an accessory to the window."
        );
}


}
