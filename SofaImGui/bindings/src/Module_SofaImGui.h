#pragma once

#include <pybind11/pybind11.h>
#include <SofaPython3/PythonFactory.h>
#include <SofaPython3/DataHelper.h>

namespace py { using namespace pybind11; }

namespace sofaimgui::python3
{
	std::pair<sofa::core::BaseData*, bool> getDataFromPyObject(py::object& obj, std::string type = "double");
}