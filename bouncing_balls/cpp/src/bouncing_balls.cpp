#include "bouncing_balls.hpp"

#include <assert.h>

#include "Box2D/Box2D.h"

#include "simulator.hpp"
#include "npy_writer.hpp"
#include "video_renderer.hpp"


namespace bounce {

	/**
	 * Returns the angle (in radians) between two given vectors
	 */
	float get_angle(const b2Vec2 &vect1, const b2Vec2 &vect2) {
		auto normalized_vec1 = vect1;
		auto normalized_vec2 = vect2;
		auto norm1 = normalized_vec1.Normalize();
		auto norm2 = normalized_vec2.Normalize();
		return acos(b2Dot(vect1, vect2) / (norm1 * norm2));
	}

	/**
	 * Returns a random unit norm vector defining the trajectory
	 * that starts from point initPos and intersects with the rectangle rect
	 */
	b2Vec2 random_trajectory_through_rectangle(const b2Vec2 &init_pos, const Rectf &rect) {
		auto min_x = rect[0] + 10;
		auto max_x = rect[0] + rect[2] - 10;

		auto min_y = rect[1] + 10;
		auto max_y = rect[1] + rect[3] - 10;

		std::vector<Point2f> vertices = { Point2f(min_x, min_y),
										  Point2f(min_x, max_y),
										  Point2f(max_x, max_y),
										  Point2f(max_x, min_y) };

		Point2f vertex1, vertex2;
		if (init_pos.x < min_x) {
			if (init_pos.y < min_y) {
				vertex1 = vertices[1];
				vertex2 = vertices[3];
			} else if (init_pos.y > max_y) {
				vertex1 = vertices[0];
				vertex2 = vertices[2];
			} else {
				vertex1 = vertices[0];
				vertex2 = vertices[1];
			}
		} else if (init_pos.x > max_x) {
			if (init_pos.y < min_y) {
				vertex1 = vertices[0];
				vertex2 = vertices[2];
			} else if (init_pos.y > max_y) {
				vertex1 = vertices[1];
				vertex2 = vertices[3];
			} else {
				vertex1 = vertices[2];
				vertex2 = vertices[3];
			}
		} else {
			if (init_pos.y < min_y) {
				vertex1 = vertices[0];
				vertex2 = vertices[3];
			} else if (init_pos.y > max_y) {
				vertex1 = vertices[1];
				vertex2 = vertices[2];
			}
		}

		b2Vec2 vect1(vertex1[0] - init_pos.x, vertex1[1] - init_pos.y);  // work with a coordinate frame centered in initPos
		b2Vec2 vect2(vertex2[0] - init_pos.x, vertex2[1] - init_pos.y);  // work with a coordinate frame centered in initPos
		auto alpha1 = get_angle(b2Vec2(1, 0), vect1);
		if (vect1.y < 0) {
			alpha1 = -alpha1;
		}
		auto alpha2 = get_angle(b2Vec2(1, 0), vect2);
		if (vect2.y < 0) {
			alpha2 = -alpha2;
		}
		auto alpha12 = get_angle(vect1, vect2);

		auto rand_angle = rand_uniform() * alpha12;
		if (alpha1 > alpha2) {
			std::swap(alpha1, alpha2);
		}
		if (alpha1 < -M_PI / 2 && alpha2 > M_PI / 2) {
			rand_angle -= alpha1;
		}
		else {
			rand_angle += alpha1;
		}

		return b2Vec2(cos(rand_angle), sin(rand_angle));
	}


	/**
	 * Returns a random position, (x, y) tuple, such that(x, y)
	 * is not a point inside the rectangle area
	 */
	b2Vec2 random_pos_outside_rectangle(int32_t screen_height, int32_t screen_width, const Rectf &rect) {
		assert(screen_height > 0);
		assert(screen_width > 0);

		const auto tol = 3;
		float y;
		auto x = rand_uniform() * screen_width;
		if (rect[0] - tol < x < rect[0] + rect[2] + tol) {
			auto empty_interval_y = screen_height - rect[3] - 2 * tol;
			y = rand_uniform() * empty_interval_y;
			if (y > rect[1] - tol) {
				y += rect[3] + 2 * tol;
			}
		}
		else {
			y = rand_uniform() * screen_height;
		}
		
		return b2Vec2(x, y);
	}



// ------------------------------------------- Config -----------------------------------------------

const std::string BouncingBalls::Config::TRAIN = "train";
const std::string BouncingBalls::Config::TEST  = "test";
const std::string BouncingBalls::Config::VALID = "validation";

BouncingBalls::Config::Config(int sequences, int sequence_len, bool occlusion, const std::vector<int> &balls,
               const fs::path &data_dir, uint32_t balls_radius, float mean_vel, uint32_t dof, 
               uint32_t screen_height, uint32_t screen_width, Channels channels_ordering, 
               bool save_metadata)
{
        this->sequences     = sequences;
        this->sequence_len  = sequence_len;
        this->occlusion     = occlusion;
		this->balls			= balls;
        this->balls_radius  = balls_radius;
        this->data_dir      = data_dir.empty() ? "data" : data_dir;
        this->mean_vel      = mean_vel;
		this->dof			= dof;
        this->screen_height = screen_height;
        this->screen_width  = screen_width;
        this->save_metadata = save_metadata;
        this->channels_ordering = channels_ordering;
}



// ------------------------------------- BouncingBalls ---------------------------------------

const std::string BouncingBalls::FILE_NAME = "bouncing_balls_";
const std::string BouncingBalls::FILE_EXTENSION = ".npy";


BouncingBalls::BouncingBalls(const Config &config) : config_(config) {
	rectangle_ = { 22, 19, 20, 10 };
    renderer_ = std::make_unique<VideoRenderer>(config.screen_height, 
											   config.screen_width, 
											   config.channels_ordering);
	writer_ = std::make_unique<NpyWriter>();
}


void BouncingBalls::generate(const fs::path &dataset_dir, bool suppress_output) {
	if (!fs::exists(dataset_dir)) fs::create_directory(dataset_dir);
	auto file_path = dataset_dir / BouncingBalls::FILE_NAME;

	generate_(dataset_dir, file_path, suppress_output);
}
        
      
void BouncingBalls::generate_(const fs::path &dataset_dir, const fs::path &file_path, bool suppress_output) {
    Simulator env(renderer_.get(), config_.save_metadata);
    env.set_fps(BouncingBalls::FPS);
    env.suppress_output(suppress_output);
	//std::vector<std::vector<std::vector<Point2f>>> metadata;
    for (auto i = 0; i < config_.sequences; ++i) {
		auto path = file_path;
        path += (std::to_string(i) + BouncingBalls::FILE_EXTENSION);
        env.save_to(path, writer_.get());
        setup_environment_(env);
        for (auto t = 0; t < config_.sequence_len; ++t) {
            env.step();
        }
        //if (config.save_metadata) {
        //    metadata.push_back(env.retrieve_and_clean_metadata());
        //}
        env.reset();
    }

    //if (config.save_metadata) {
	//	cnpy::NpyArray array()
    //    cnpy::npy_save((dataset_dir / "metadata.npy").string(), np.array(metadata));
    //}
}


void BouncingBalls::setup_environment_(Simulator &env) {
    auto choice = int(rand_uniform() * config_.balls.size());
    auto balls = config_.balls[choice];
    if (config_.occlusion) {
        env.add_rectangular_occlusion(rectangle_);
        for (auto b = 0; b < balls; ++b) {
            auto vel = rand_normal(config_.mean_vel, config_.mean_vel / 10.f);
            auto position = random_pos_outside_rectangle(config_.screen_height, config_.screen_width, rectangle_);
            auto v = random_trajectory_through_rectangle(position, rectangle_);
            env.add_circle(Vector2f(position.x, position.y), Vector2f(v.x * vel, v.y * vel));
        }
    }
    else {
        for (auto b = 0; b < balls; ++b) {
            if (config_.dof == 1) {
                env.add_circle(env.get_rand_pos_px(), Vector2f(2 * 5000 * rand_uniform() - 5000, 0));
            }
            else if (config_.dof == 2) {
                env.add_rand_circle(config_.mean_vel);
            }
        }
    }
}


}