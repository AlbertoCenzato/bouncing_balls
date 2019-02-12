#include "bouncing_balls.hpp"


int main() {

	bounce::BouncingBalls::Config config(10, 10, false, {1});
	bounce::BouncingBalls bouncing_balls(config);

	bouncing_balls.generate("C:\\Users\\micheluzzo\\Desktop\\data");

	return 0;
}