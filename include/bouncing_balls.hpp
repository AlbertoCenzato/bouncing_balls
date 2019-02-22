#ifndef BOUNCE_BOUNCING_BALLS_HPP
#define BOUNCE_BOUNCING_BALLS_HPP

#include "common_types.hpp"

#include <memory>

#include "video_renderer.hpp"
#include "npy_writer.hpp"


namespace bounce {

class Simulator;


class BouncingBalls {

public:

    struct Config {

		static const std::string TRAIN;
		static const std::string TEST;
		static const std::string VALID;

        int sequences;
        int sequence_len;
        bool occlusion;
        std::vector<int> balls;
        fs::path data_dir;
        float balls_radius;
        float mean_vel;
        uint32_t dof;
        uint32_t screen_height;
        uint32_t screen_width;
        Channels channels_ordering;
        bool save_metadata;

        Config(int sequences=0, int sequence_len=0, bool occlusion=false, const std::vector<int> &balls={},
               const fs::path &data_dir="", float balls_radius=5.f, float mean_vel=5000.f, uint32_t dof=2, 
               uint32_t screen_height=48, uint32_t screen_width=64, Channels channels_ordering=Channels::LAST, 
               bool save_metadata=true);
    };

    static const uint32_t SCREEN_WIDTH  = 64;
    static const uint32_t SCREEN_HEIGHT = 64;
    static const Channels CHANNEL_ORDERING = Channels::FIRST;

    static const uint32_t FPS = 60;

    static const std::string FILE_NAME;
    static const std::string FILE_EXTENSION;

    BouncingBalls(const Config &config);

    void generate(const std::string & dataset_dir, bool suppress_output=true);
                

private:

    std::unique_ptr<NpyWriter> writer_;
    std::unique_ptr<VideoRenderer> renderer_;
	Rectf rectangle_;

    Config config_;

    void generate_(const fs::path &dataset_dir, const fs::path &file_path, bool suppress_output);

    void setup_environment_(Simulator &simulator);

};


}

#endif