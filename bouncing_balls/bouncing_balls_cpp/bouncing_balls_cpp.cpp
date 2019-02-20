#include "3rdparty/pybind11/include/pybind11/pybind11.h"
#include "3rdparty/pybind11/include/pybind11/stl.h"

#include <vector>
#include <string>

#include "include/bouncing_balls.hpp"

namespace py = pybind11;
//namespace fs = std::experimental::filesystem;


PYBIND11_MODULE(bouncing_balls_cpp, m) {
	py::enum_<bounce::Channels>(m, "Kind")
		.value("FIRST", bounce::Channels::FIRST)
		.value("LAST", bounce::Channels::LAST);

	py::class_<bounce::BouncingBalls::Config>(m, "Config")
		.def(py::init<int, int, bool, const std::vector<int> &, const std::string &, float, float, uint32_t, uint32_t, uint32_t, bounce::Channels, bool>(),
			py::arg("sequences") = 0,
			py::arg("sequence_len") = 0,
			py::arg("occlusion") = false,		// TODO: fix these default args, find a way to bind them with the defaults in Config constructor declaration
			py::arg("balls") = std::vector<int>{},				// TODO: fix these default args, find a way to bind them with the defaults in Config constructor declaration
			py::arg("data_dir") = "",			// TODO: fix these default args, find a way to bind them with the defaults in Config constructor declaration
			py::arg("balls_radius") = 5.f,		// TODO: fix these default args, find a way to bind them with the defaults in Config constructor declaration
			py::arg("mean_vel") = 5000.f,
			py::arg("dof") = 2,
			py::arg("screen_height") = 48,
			py::arg("screen_width") = 64,
			py::arg("channels_ordering") = bounce::Channels::LAST,
			py::arg("save_metadata") = false)
		.def_readwrite("sequences", &bounce::BouncingBalls::Config::sequences);

	py::class_<bounce::BouncingBalls>(m, "BouncingBalls")
		.def(py::init<const bounce::BouncingBalls::Config &>())
		.def("generate", &bounce::BouncingBalls::generate);
}