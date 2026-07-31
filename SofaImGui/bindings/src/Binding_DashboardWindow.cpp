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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>

#include <SofaPython3/Sofa/Core/Binding_Base.h>
#include <Binding_DashboardWindow.h>

#include <SofaPython3/PythonFactory.h>
#include <SofaPython3/PythonEnvironment.h>

#include <sofa/gui/common/GUIManager.h>
#include <SofaImGui/ImGuiGUI.h>
#include <SofaImGui/ImGuiGUIEngine.h>
#include <Module_SofaImGui.h>
#include <SofaImGui/windows/DashboardWindow.h>

SOFAPYTHON3_BIND_ATTRIBUTE_ERROR()

/// Makes an alias for the pybind11 namespace to increase readability.
namespace py { using namespace pybind11; }
using namespace pybind11::literals;

namespace sofaimgui::python3
{
    void addData(std::shared_ptr<ImGuiGUIEngine> engine,
                const std::string& label,
                py::object data,
                py::object min, py::object max,
                const std::string& group,
                const std::string& help,
                std::string type)
	{
		if (engine)
		{
            auto p_data = getDataFromPyObject(data, type);
            std::string p_group = group;
            std::string p_help = help;
            if (p_data.first && !p_data.second)
            {
                if (group == models::guidata::GUIData::DEFAULTGROUP)
                    p_group = p_data.first->getOwner()->getPathName();
                if (help.empty())
                    p_help = p_data.first->getHelp();
            }
            engine->m_dashboardWindow.addData(label,
                                              getDataFromPyObject(data, type),
                                              getDataFromPyObject(min, type),
                                              getDataFromPyObject(max, type),
                                              p_group,
                                              p_help);
		}
	}

void moduleAddDashboardWindow(py::module &m)
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;

    auto m_a = m.def_submodule("DashboardWindow", "");
    std::string m_a_name = py::str(m_a.attr("__name__"));

    m_a.def("addData",
        [engine](const std::string& label, py::object data, py::object min, py::object max, std::string group, std::string help, std::string type)
        {
            if (engine)
            {
                addData(engine, label, data, min, max, group, help, type);
            }
        }
        , "label"_a, "data"_a, "min"_a = py::none(), "max"_a = py::none(), "group"_a = models::guidata::GUIData::DEFAULTGROUP, "help"_a = "", "type"_a = "double"
        , "Add a data to the DashboardWindow."
    );
}

}

