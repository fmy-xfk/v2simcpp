#include <pybind11\pybind11.h>
#include <pybind11\stl.h>
#include <pybind11\functional.h>
#include <iostream>
#include "..\V2SimCore\v2sim.h"

namespace py = pybind11;

PYBIND11_MODULE(PyV2Sim, m)
{
    m.doc() = "V2Sim C++ core Python wrapper";
    // V2SimError
    py::register_exception<V2SimError>(m, "V2SimError");

    // RangeList
    py::class_<RangeList>(m, "RangeList")
        .def(py::init<bool>(), py::arg("always") = false)
        .def(py::init<const std::vector<std::pair<int, int>>&, int, int>(),
            py::arg("d"), py::arg("period") = 0, py::arg("times") = 1)
        .def("__contains__", &RangeList::Contains)
        .def("SetForce", &RangeList::SetForce)
        .def("ClearForce", &RangeList::ClearForce)
        .def("__len__", &RangeList::size);

    // SegFunc
    py::class_<SegFunc>(m, "SegFunc")
        .def(py::init<>())
        .def(py::init<const std::vector<int>&, const std::vector<double>&, int, int>(),
            py::arg("timelist"), py::arg("data"), py::arg("period") = 0, py::arg("times") = 1)
        .def(py::init<std::initializer_list<std::pair<int, double>>, int, int>(),
            py::arg("d"), py::arg("period") = 0, py::arg("times") = 1)
        .def("__len__", &SegFunc::size)
        .def("GetPeriod", &SegFunc::GetPeriod)
        .def("GetRepeatTimes", &SegFunc::GetRepeatTimes)
        .def("Get", &SegFunc::Get)
        .def("Add", &SegFunc::Add)
        .def("__call__", &SegFunc::operator())
        .def("SetOverride", &SegFunc::SetOverride)
        .def("ClearOverride", &SegFunc::ClearOverride)
        .def("GetOverride", &SegFunc::GetOverride)
        .def("__neg__", &SegFunc::operator-)
        .def("__mul__", &SegFunc::operator*)
        .def("Expand", &SegFunc::Expand)
        .def("SelfExpand", &SegFunc::SelfExpand)
        .def("TimeLine", &SegFunc::TimeLine)
        .def("Data", &SegFunc::Data);

    // Point
    py::class_<Point>(m, "Point")
        .def(py::init<double, double, int>(),
            py::arg("x") = 0, py::arg("y") = 0, py::arg("label") = 0)
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def_readwrite("label", &Point::label)
        .def("dist_to", &Point::dist_to);

    // KDTree
    py::class_<KDTree>(m, "KDTree")
        .def(py::init<>())
        .def(py::init<const std::vector<Point>&>())
        .def("Init", &KDTree::Init)
        .def("findNearestNeighbor", &KDTree::findNearestNeighbor)
        .def("findKNearestNeighbors", &KDTree::findKNearestNeighbors);

    // VehStatus enum
    py::enum_<VehStatus>(m, "VehStatus")
        .value("Driving", VehStatus::Driving)
        .value("Pending", VehStatus::Pending)
        .value("Charging", VehStatus::Charging)
        .value("Parking", VehStatus::Parking)
        .value("Depleted", VehStatus::Depleted)
        .export_values();

    // Trip
    py::class_<Trip>(m, "Trip")
        .def(py::init<const std::string&, int, const std::string&, const std::string&,
            const std::vector<std::string>&, bool, bool>(),
            py::arg("id"), py::arg("dpt_time"), py::arg("fTAZ"), py::arg("tTAZ"),
            py::arg("route"), py::arg("auto_detect_fixed_route") = true, py::arg("fixed_route") = false)
        .def(py::init<const std::string&, int, const std::string&, const std::string&,
            const std::string&, bool, bool>(),
            py::arg("id"), py::arg("dpt_time"), py::arg("fTAZ"), py::arg("tTAZ"),
            py::arg("route"), py::arg("auto_detect_fixed_route") = true, py::arg("fixed_route") = false)
        .def_readwrite("ID", &Trip::ID)
        .def_readwrite("DepartTime", &Trip::DepartTime)
        .def_readwrite("FromTAZ", &Trip::FromTAZ)
        .def_readwrite("ToTAZ", &Trip::ToTAZ)
        .def_readwrite("FixedRoute", &Trip::FixedRoute)
        .def("Route", &Trip::Route, py::return_value_policy::reference)
        .def("FromEdge", &Trip::FromEdge, py::return_value_policy::reference)
        .def("ToEdge", &Trip::ToEdge, py::return_value_policy::reference)
        .def("__repr__", &Trip::__repr__, py::return_value_policy::reference);

    py::class_<BattCorrFunc>(m, "BattCorrFunc");

    // BattCorrFuncPool
    py::class_<BattCorrFuncPool>(m, "BattCorrFuncPool")
        .def_static("Add", &BattCorrFuncPool::Add)
        .def_static("Get", &BattCorrFuncPool::Get, py::return_value_policy::reference);

    py::class_<V2GAlloc>(m, "V2GAlloc");

    // V2GAllocPool
    py::class_<V2GAllocPool>(m, "V2GAllocPool")
        .def_static("Add", &V2GAllocPool::Add)
        .def_static("Get", &V2GAllocPool::Get, py::return_value_policy::reference);

    // V2SimCore
    py::class_<V2SimInterface>(m, "V2SimInterface")
        .def(py::init<int, int, int, const std::string&, const std::string&, const std::string&, const std::string,
            const std::string, bool, bool, bool>(),
            py::arg("start_time"), py::arg("end_time"), py::arg("step_length"),
            py::arg("roadnet"), py::arg("ev_file"), py::arg("fcs_file"), py::arg("scs_file"),
            py::arg("output_dir"), py::arg("log_fcs") = true, py::arg("log_scs") = true, py::arg("log_ev") = false)
        .def("getTime", &V2SimInterface::getTime)
        .def("getStartTime", &V2SimInterface::getStartTime)
        .def("getEndTime", &V2SimInterface::getEndTime)
        .def("getStepLength", &V2SimInterface::getStepLength)
        .def("Start", &V2SimInterface::Start)
        .def("Step", &V2SimInterface::Step, py::arg("len") = -1)
        .def("Stop", &V2SimInterface::Stop)
        .def("EV_IndexOf", &V2SimInterface::EV_IndexOf)
		.def("EV_getName", &V2SimInterface::EV_getName)
		.def("EV_getStatus", &V2SimInterface::EV_getStatus)
		.def("EV_setStatus", &V2SimInterface::EV_setStatus)
		.def("EV_getTargetCSIndex", &V2SimInterface::EV_getTargetCSIndex)
		.def("EV_setTargetCSIndex", &V2SimInterface::EV_setTargetCSIndex)
		.def("EV_getCost", &V2SimInterface::EV_getCost)
		.def("EV_setCost", &V2SimInterface::EV_setCost)
		.def("EV_getRevenue", &V2SimInterface::EV_getRevenue)
		.def("EV_setRevenue", &V2SimInterface::EV_setRevenue)
		.def("EV_getBattCap", &V2SimInterface::EV_getBattCap)
		.def("EV_setBattCap", &V2SimInterface::EV_setBattCap)
		.def("EV_getBattElec", &V2SimInterface::EV_getBattElec)
		.def("EV_setBattElec", &V2SimInterface::EV_setBattElec)
		.def("EV_getPcFast", &V2SimInterface::EV_getPcFast)
		.def("EV_setPcFast", &V2SimInterface::EV_setPcFast)
		.def("EV_getPcFast_kW", &V2SimInterface::EV_getPcFast_kW)
		.def("EV_setPcFast_kW", &V2SimInterface::EV_setPcFast_kW)
		.def("EV_getPcSlow", &V2SimInterface::EV_getPcSlow)
		.def("EV_setPcSlow", &V2SimInterface::EV_setPcSlow)
		.def("EV_getPcSlow_kW", &V2SimInterface::EV_getPcSlow_kW)
		.def("EV_setPcSlow_kW", &V2SimInterface::EV_setPcSlow_kW)
		.def("EV_getEtaC", &V2SimInterface::EV_getEtaC)
		.def("EV_setEtaC", &V2SimInterface::EV_setEtaC)
		.def("EV_getPdV2G", &V2SimInterface::EV_getPdV2G)
		.def("EV_setPdV2G", &V2SimInterface::EV_setPdV2G);
}