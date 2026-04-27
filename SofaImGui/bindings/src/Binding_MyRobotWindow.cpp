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
#include <Binding_MyRobotWindow.h>

#include <SofaPython3/PythonFactory.h>
#include <SofaPython3/PythonEnvironment.h>

#include <sofa/gui/common/GUIManager.h>

#include <SofaImGui/ImGuiGUI.h>
#include <SofaImGui/models/guidata/GUIData.h>
#include <Module_SofaImGui.h>

SOFAPYTHON3_BIND_ATTRIBUTE_ERROR()

/// Makes an alias for the pybind11 namespace to increase readability.
namespace py { using namespace pybind11; }
using namespace pybind11::literals;

namespace sofaimgui::python3
{

void addInformation(std::shared_ptr<ImGuiGUIEngine> engine, const std::string &label, py::object data, const std::string &group, const std::string& help, std::string type = "double")
{
    if (engine)
    {
        engine->m_myRobotWindow.addData(label,
                                        getDataFromPyObject(data, type),
			                            std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                                        std::pair<sofa::core::BaseData*, bool>(nullptr, false),
                                        group, help, sofaimgui::windows::MyRobotWindow::Section::INFORMATION);
    }
}

void addSetting(std::shared_ptr<ImGuiGUIEngine> engine, const std::string &label, py::object data, py::object min, py::object max, const std::string &group, const std::string& help, std::string type="double")
{
    if (engine)
    {
        engine->m_myRobotWindow.addData(label,
                                        getDataFromPyObject(data, type),
                                        getDataFromPyObject(min, type),
                                        getDataFromPyObject(max, type),
                                        group, help, sofaimgui::windows::MyRobotWindow::Section::SETTINGS);
    }
}

std::vector<std::string> listAvailablePortsCallback(const py::object& instance, const std::string& callbackName)
{
    sofapython3::PythonEnvironment::gil acquire;

    if (py::hasattr(instance, callbackName.c_str()))
    {
        py::object fct = instance.attr(callbackName.c_str());
        if (PyCallable_Check(fct.ptr()))
            return fct().cast<std::vector<std::string>>();
    }
    return std::vector<std::string>();
}

void moduleAddMyRobotWindow(py::module &m)
{
    ImGuiGUI* gui = ImGuiGUI::getGUI();
    std::shared_ptr<ImGuiGUIEngine> engine = gui? gui->getGUIEngine() : nullptr;

    //************************
	// MyRobotWindow submodule
	//************************
    auto m_a = m.def_submodule("MyRobotWindow", "");
	std::string m_a_name = py::str(m_a.attr("__name__"));

    m_a.def("addInformation",
        [engine, m_a_name](const std::string &description, py::object data)
        {
            msg_deprecated(m_a_name) << "addInformation is deprecated and will be removed in future versions.Please use Sofa.ImGui.MyRobotWindow.Information.addData instead.";
            addInformation(engine, description, data, models::guidata::GUIData::DEFAULTGROUP, "");
        }, "[DEPRECATED] Add an information to the window."
        );

    m_a.def("addInformationInGroup",
        [engine, m_a_name](const std::string &description, py::object data, const std::string &group)
        {
            msg_deprecated(m_a_name) << "addInformationInGroup is deprecated and will be removed in future versions.Please use Sofa.ImGui.MyRobotWindow.Information.addData instead.";
            addInformation(engine, description, data, group, "");
        }, "[DEPRECATED] Add an information to the window."
        );

    m_a.def("addSetting",
        [engine, m_a_name](const std::string &description, py::object data, py::object min, py::object max)
        {
            msg_deprecated(m_a_name) << "addSetting is deprecated and will be removed in future versions.Please use Sofa.ImGui.MyRobotWindow.Settings.addData instead.";
            if (engine)
            {
                addSetting(engine, description, data, min, max, models::guidata::GUIData::DEFAULTGROUP, "");
            }
        }, "[DEPRECATED] Add a setting to the window."
        );

    m_a.def("addSettingInGroup",
        [engine, m_a_name](const std::string &description, py::object data, py::object min, py::object max, const std::string &group)
        {
            msg_deprecated(m_a_name) << "addSettingInGroup is deprecated and will be removed in future versions.Please use Sofa.ImGui.MyRobotWindow.Settings.addData instead.";
            if (engine)
            {
                addSetting(engine, description, data, min, max, group, "");
            }
        }, "[DEPRECATED] Add a setting to the window."
        );

    m_a.def("listAvailablePortsCallback",
        [engine, m_a_name](const py::object &instance, const std::string& callbackName)
        {
			msg_deprecated(m_a_name)<< "listAvailablePortsCallback is deprecated and will be removed in future versions. Please use Sofa.ImGui.MyRobotWindow.Connection.registerAvailablePortsCallback instead.";
            if (engine)
            {
                auto& connection = engine->m_myRobotWindow.getConnection();
                connection.listAvailablePortsCallback = [instance, callbackName]() -> std::vector<std::string> {
                    return listAvailablePortsCallback(instance, callbackName);
                };
            }
        }, "[DEPRECATED] Registers the callback to list available ports. This function is called to list the available ports in Sofa.ImGui.MyRobotWindow.updateAvailablePorts.");

    m_a.def("getSelectedPort",
        [engine, m_a_name]() -> std::string
        {
            msg_deprecated(m_a_name) << "getSelectedPort is deprecated and will be removed in future versions. Please use Sofa.ImGui.MyRobotWindow.Connection.getSelectedPort instead.";
            if (engine)
            {
                return engine->m_myRobotWindow.getSelectedPort();
            }
            return std::string();
        }, "Get the port selected from the window."
    );
    m_a.def("updateAvailablePorts",
        [engine, m_a_name]()
        {
            msg_deprecated(m_a_name) << "updateAvailablePorts is deprecated and will be removed in future versions. Please use Sofa.ImGui.MyRobotWindow.Connection.updateAvailablePorts instead.";
            if (engine)
            {
                engine->m_myRobotWindow.setAvailablePorts(engine->m_myRobotWindow.getConnection().listAvailablePortsCallback());
            }
        }, "Update the list of available ports."
    );

	//*********************
	// Connection submodule
	//*********************
    auto m_a_connection = m_a.def_submodule("Connection", "");
    std::string m_a_connection_name = py::str(m_a_connection.attr("__name__"));

    m_a_connection.def("registerAvailablePortsCallback",
        [engine](const py::object& instance, const std::string& callbackName)
        {
            if (engine)
            {
                auto& connection = engine->m_myRobotWindow.getConnection();
                connection.listAvailablePortsCallback = [instance, callbackName]() -> std::vector<std::string> {
                    return listAvailablePortsCallback(instance, callbackName);
                    };
            }
        }, "Registers the callback to list available ports. This function is called to list the available ports in Sofa.ImGui.MyRobotWindow.Connection.updateAvailablePorts.");

    m_a_connection.def("getSelectedPort",
        [engine]() -> std::string
        {
            if (engine)
            {
                return engine->m_myRobotWindow.getSelectedPort();
            }
            return std::string();
        }, "Get the port selected from the window."
    );
    m_a_connection.def("updateAvailablePorts",
        [engine]()
        {
            if (engine)
            {
                engine->m_myRobotWindow.setAvailablePorts(engine->m_myRobotWindow.getConnection().listAvailablePortsCallback());
            }
        }, "Update the list of available ports."
    );

	//**********************
	// Information submodule
	//**********************

	auto m_a_information = m_a.def_submodule("Information", "");
	std::string m_a_information_name = py::str(m_a_information.attr("__name__"));

	m_a_information.def("addData",
		[engine](const std::string& label, py::object data, std::string group, std::string help)
		{
			if (engine)
			{
				if (group.empty())
                    addInformation(engine, label, data, models::guidata::GUIData::DEFAULTGROUP, help);
                else
					addInformation(engine, label, data, group, help);
			}
		}
        , "label"_a, "data"_a, "group"_a = models::guidata::GUIData::DEFAULTGROUP, "help"_a = ""
        ,"Add an information to the window."
	);

	//*******************
	// Settings submodule
	//*******************

	auto m_a_settings = m_a.def_submodule("Settings", "");
	std::string m_a_settings_name = py::str(m_a_settings.attr("__name__"));

	m_a_settings.def("addData",
		[engine](const std::string& label, py::object data, py::object min, py::object max, std::string group, std::string help, std::string type)
		{
			if (engine)
			{
				if (group.empty())
                    addSetting(engine, label, data, min, max, models::guidata::GUIData::DEFAULTGROUP, help, type);
				else
					addSetting(engine, label, data, min, max, group, help, type);
			}
		}
        , "label"_a, "data"_a, "min"_a = py::none(), "max"_a = py::none(), "group"_a = models::guidata::GUIData::DEFAULTGROUP, "help"_a = "", "type"_a = "double"
		, "Add a setting to the window."
	);

}

}
