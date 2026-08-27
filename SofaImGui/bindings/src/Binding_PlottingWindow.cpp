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
#include <Binding_PlottingWindow.h>

#include <SofaPython3/PythonFactory.h>
#include <SofaPython3/PythonEnvironment.h>

#include <sofa/gui/common/GUIManager.h>

#include <SofaImGui/ImGuiGUI.h>
#include <SofaImGui/ImGuiGUIEngine.h>

#include <Module_SofaImGui.h>

SOFAPYTHON3_BIND_ATTRIBUTE_ERROR()

/// Makes an alias for the pybind11 namespace to increase readability.
namespace py { using namespace pybind11; }
using namespace pybind11::literals;

namespace sofaimgui::python3
{

void moduleAddPlottingWindow(py::module &m)
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;

    auto m_a = m.def_submodule("PlottingWindow", "");
    auto m_a_name = py::str(m_a.attr("__name__"));

    m_a.def("addData",
        [engine, m_a_name](const std::string &label, py::object data, std::string type, const int& subplotIndex)
        {
            if (engine)
            {
                if (subplotIndex >= windows::PlottingWindow::MAX_NB_PLOT)
                    msg_warning(m_a_name) << "The maximum number of sublots is " << windows::PlottingWindow::MAX_NB_PLOT << ". Using first subplot instead." ;

                engine->m_plottingWindow.addData(label,
                                                 getDataFromPyObject(data, type),
                                                 subplotIndex);
            }
        }
        , "label"_a, "data"_a, "type"_a = "double", "subplotIndex"_a = 0
        ,"Add data to plot, with description."
        );
}

}

