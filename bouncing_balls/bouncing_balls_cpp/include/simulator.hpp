#ifndef BOUNCE_SIMULATOR_HPP
#define BOUNCE_SIMULATOR_HPP

#include <chrono>
#include <memory>
#include <forward_list>
#include <filesystem>

#include "common_types.hpp"


class b2World;
class b2Body;
struct b2BodyDef;


namespace bounce {


class VideoRenderer;
class NpyWriter;

class Simulator {

public:

    static const uint32_t DEFAULT_FPS = 60;


    Simulator(VideoRenderer *renderer, bool save_metadata=true);


    uint32_t get_fps() const;

    void set_fps(uint32_t fps);


    Point2f get_rand_pos_px() const;

    void enable_bounding_box(bool enable);

    void suppress_output(bool no_output);

    void save_to(const fs::path &path, NpyWriter *writer);

    void add_line(const Point2f &p1, const Point2f &p2);

    // TODO: add rect type
    void add_rectangular_occlusion(const Rectf & rect);

    void add_circle(const Point2f &pos, const Vector2f &vel, float radius=5);

    void add_rand_circle(float mean_vel=5000, float radius=5);

    std::chrono::milliseconds step();

    void run_simulation(std::chrono::seconds time);

    void reset();

    void quit();

	//std::vector<std::vector<Point2f>> retrieve_and_clean_metadata();

    
private:
   
    std::forward_list<BodyData> body_data_storage_;  // NOTE: using forward_list since we only need to iterate in one direction and
													 // need a data structure in which a new allocation does not invalidate 
													 // references to contained objects as std::vector would do. In this way we avoid
    NpyWriter *writer_;								 // memory management issues or the use of smart pointers
    VideoRenderer *renderer_;
    std::unique_ptr<b2World> world_;
    

    bool save_metadata_;

    uint32_t fps_;
    std::chrono::milliseconds time_step_;
    
    b2Body *bounding_box_;
    std::chrono::high_resolution_clock clock_;
    
    std::vector<std::vector<Point2f>> metadata_;
    bool save_ = false;
    bool screen_output_ = false;

    float screen_width_m_;
    float screen_height_m_;


    BodyData* create_body_data_(const std::string &name, bool visible, float radius);

    void create_screen_bounding_box_();

    //void collect_metadata();

};

}

#endif