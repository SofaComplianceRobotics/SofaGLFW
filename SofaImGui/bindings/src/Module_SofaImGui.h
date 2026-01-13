#pragma once

#include <pybind11/pybind11.h>
#include <SofaPython3/PythonFactory.h>
#include <SofaPython3/DataHelper.h>

namespace py { using namespace pybind11; }

namespace sofaimgui{


	std::pair<sofa::core::BaseData*, bool> getDataFromPyObject(py::object& obj, std::string type = "double")
	{
		if(obj.is_none())
			return std::pair<sofa::core::BaseData*, bool>(nullptr, false);

		if (py::isinstance<sofa::core::objectmodel::BaseData>(obj))
		{
			return std::pair<sofa::core::BaseData*, bool>(py::cast<sofa::core::objectmodel::BaseData*>(obj), false); //sofapython3::addData(py::none(), "Label", obj, py::none(), "", "group", type); //py::cast<sofa::core::objectmodel::BaseData*>(obj);
		}
		else {
			sofa::core::BaseData* data = sofapython3::PythonFactory::createInstance(type);
			if (!obj.is_none() and data) {
				sofapython3::PythonFactory::fromPython(data, obj);
				return std::pair<sofa::core::BaseData*, bool>(data, true);
			}
		}
		msg_error("Module_SofaImGui") << "Unable to convert py::object " << obj;
		return std::pair<sofa::core::BaseData*, bool>(nullptr, false);
	}

}