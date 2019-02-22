#include "common_types.hpp"

#include <random>

#include "Box2D/Box2D.h"

namespace bounce {

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_real_distribution<float> uniform(0.0, 1.0);


BodyData* get_body_data(b2Body *body) {
    return static_cast<BodyData*>(body->GetUserData());
}


float rand_uniform() {
	return uniform(gen);
}

float rand_normal(float mean, float std) {
	std::normal_distribution<> normal(mean, std);
	return static_cast<float>(normal(gen));
}

}
