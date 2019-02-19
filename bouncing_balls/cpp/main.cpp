#include <chrono>
#include <thread>
#include <iostream>

#include "bouncing_balls.hpp"


int main() {

	auto start = std::chrono::system_clock::now();

	bounce::BouncingBalls::Config config(10, 40, false, {1}, "C:\\Users\\micheluzzo\\Desktop\\data", 5.f);
	bounce::BouncingBalls bouncing_balls(config);

	bouncing_balls.generate("C:\\Users\\micheluzzo\\Desktop\\data");

	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = end - start;
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);

	std::cout << "finished computation at " << std::ctime(&end_time)
			<< "elapsed time: " << elapsed_seconds.count() << "s\n";

	std::this_thread::sleep_for(std::chrono::seconds(15));

	return 0;
}