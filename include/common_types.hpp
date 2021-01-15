#ifndef BOUNCE_COMMON_TYPES_HPP
#define BOUNCE_COMMON_TYPES_HPP

#include <valarray>
#include <array>
#include <iterator>
#include <filesystem> 

#define _USE_MATH_DEFINES
#include <math.h>

#include "cnpy/cnpy.h"

#include "b2Iterator.hpp"

namespace fs = std::filesystem;


class b2Body;

namespace bounce {

	using Point2i = std::valarray<int32_t>;
	using Point2f = std::valarray<float>;

	using Vector2i = Point2i;   // point and vector support the same operations but are logically different
	using Vector2f = Point2f;   // point and vector support the same operations but are logically different

	using Recti = std::array<float, 4>;
	using Rectf = std::array<int32_t, 4>;


	enum class Channels {
		FIRST = 1,
		LAST = 2
	};


	struct BodyData {

		std::string name;
		bool visible;
		float radius;

		BodyData(const std::string &name, bool visible, float radius = 5.f)
			: name(name), visible(visible), radius(radius) {}
	};

	BodyData* get_body_data(b2Body *body);


	class NotImplementedException : public std::logic_error
	{
	public:
		NotImplementedException() : std::logic_error("Function not yet implemented") { };
	};


	float rand_uniform();

	float rand_normal(float mean, float std);

}

#endif