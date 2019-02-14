#include "simulator.hpp"

#include <random>
#include <iostream>

#include "Box2D/Box2D.h"

#include "video_renderer.hpp"
#include "npy_writer.hpp"


namespace bounce {

Vector2f get_rand_vel(float mean_vel) {
    auto angle = 2 * float(M_PI) * rand_uniform();
    auto norm = rand_normal(mean_vel, mean_vel / 10);
    auto vx = std::cos(angle) * norm;
    auto vy = std::sin(angle) * norm;
    return {vx, vy};
}


BodyData* get_user_data(const b2Body *body) {
    return static_cast<BodyData*>(body->GetUserData());
}



// --------------------------- Simulator -----------------------------


Simulator::Simulator(VideoRenderer *renderer, bool save_metadata) : renderer_(renderer) {

    set_fps(DEFAULT_FPS);

    // --- Box2D setup ---
    world_ = std::make_unique<b2World>(b2Vec2(0.0, 0.0));

    screen_width_m_  = renderer->pixels_to_meters(renderer->screen_width_px);
    screen_height_m_ = renderer->pixels_to_meters(renderer->screen_height_px);
    
    create_screen_bounding_box_();
}


uint32_t Simulator::get_fps() const {
    return fps_;
}

void Simulator::set_fps(uint32_t fps) {
    this->fps_ = fps;
    time_step_ = std::chrono::milliseconds{1000 / int32_t(fps)};
}
    
Point2f Simulator::get_rand_pos_px() const {
    return {rand_uniform() * renderer_->screen_width_px, rand_uniform() * renderer_->screen_height_px};
}

void Simulator::create_screen_bounding_box_() {
    b2Vec2 screen_center{screen_width_m_ / 2.0f, screen_height_m_ / 2.0f};
    
    b2Vec2 bottom_left {-screen_center.x, -screen_center.y};
    b2Vec2 bottom_right{ screen_center.x, -screen_center.y};
    b2Vec2 top_right   { screen_center.x,  screen_center.y};
    b2Vec2 top_left    {-screen_center.x,  screen_center.y};

    b2Vec2 vs[4];
    vs[0].Set(-screen_center.x, -screen_center.y);
    vs[1].Set( screen_center.x, -screen_center.y);
    vs[2].Set( screen_center.x,  screen_center.y);
    vs[3].Set(-screen_center.x,  screen_center.y);
    b2ChainShape chain;
    chain.CreateLoop(vs, 4);

    b2BodyDef bounding_box_def;
    bounding_box_def.position.Set(screen_center.x, screen_center.y);
    bounding_box_def.userData = create_body_data_("world_bounding_box", false);
    bounding_box_def.active = true;

    bounding_box_ = world_->CreateBody(&bounding_box_def);

    b2FixtureDef box_fixture;
    box_fixture.shape = &chain;

    bounding_box_->CreateFixture(&box_fixture);
}


void Simulator::enable_bounding_box(bool enable) {
    bounding_box_->SetAwake(enable);
}

void Simulator::suppress_output(bool no_output) {
   //renderer->is_visible = !no_output;
}


void Simulator::save_to(const fs::path &path, NpyWriter *wrtr) {
    save_ = true;
    writer_ = wrtr;
    writer_->set_fps(fps_);
    writer_->set_resolution(renderer_->screen_width_px, renderer_->screen_height_px);
    writer_->open(path);
}


void Simulator::add_line(const Point2f &p1, const Point2f &p2) {
    auto p1_m = renderer_->to_world_frame(b2Vec2{p1[0], p1[1]});
    auto p2_m = renderer_->to_world_frame(b2Vec2{p2[0], p2[1]});

    b2Vec2 m{(p1[0] + p2[0]) / 2.0f, (p1[1] + p2[1]) / 2.0f};
    p1_m -= m; // compute p1 coordinates wrt m
    p2_m -= m; // compute p2 coordinates wrt m

    b2BodyDef line_def;
    line_def.position.Set(m.x, m.y);
    line_def.active = false;
    line_def.userData = create_body_data_("line", false);
    auto line = world_->CreateBody(&line_def);

    b2EdgeShape edge_shape;
    edge_shape.Set(p1_m, p2_m);

    b2FixtureDef fixture_def;
    fixture_def.shape = &edge_shape;
    line->CreateFixture(&fixture_def);    
    
    std::cout << "WARNING: line rendering disabled!" << std::endl;
}


void Simulator::add_rectangular_occlusion(const Rectf & rect) {
    auto width_px  = rect[2];
    auto height_px = rect[3];
	b2Vec2 center_px{ rect[0] + width_px / 2.0f, rect[1] + height_px / 2.0f };
    auto center_m = renderer_->to_world_frame(center_px);
    auto width_m  = renderer_->pixels_to_meters(width_px); 
    auto height_m = renderer_->pixels_to_meters(height_px);

    b2BodyDef occlusion_def;
    occlusion_def.position.Set(center_m.x, center_m.y);
    occlusion_def.active = false;
    occlusion_def.userData = create_body_data_("rectangular_occlusion", true);
    auto occlusion = world_->CreateBody(&occlusion_def);

    b2PolygonShape poly;
    poly.SetAsBox(height_m, width_m);

    b2FixtureDef fixture_def;
    fixture_def.shape = &poly;

    occlusion->CreateFixture(&fixture_def);    
}


void Simulator::add_circle(const Point2f &pos_px, const Vector2f &vel_px, float radius) {
	auto pos_m = renderer_->to_world_frame({ pos_px[0], pos_px[1] });
    b2Vec2 vel_m{renderer_->pixels_to_meters(vel_px[0]), renderer_->pixels_to_meters(vel_px[1])};

    b2BodyDef circle_def;
    circle_def.position.Set(pos_m.x, pos_m.y);
    circle_def.userData = create_body_data_("circle", true);
    circle_def.linearVelocity = vel_m;
    auto circle = world_->CreateBody(&circle_def);
    
    b2CircleShape shape;
    shape.m_p.Set(pos_m.x, pos_m.y);
    shape.m_radius = radius;

    circle->CreateFixture(&shape, 1);
}


void Simulator::add_rand_circle(float mean_vel, float radius) {
    const auto position_px = get_rand_pos_px();
    const auto velocity_px = get_rand_vel(mean_vel);
    add_circle(position_px, velocity_px, radius);
}


std::chrono::milliseconds Simulator::step() {
    const auto frame = renderer_->get_frame(world_.get());

    if (save_)
        *writer_ << frame;
    //if (save_metadata)
    //    collect_metadata();
    
    // Make Box2D simulate the physics of our world for one step.
    // Instruct the world to perform a single step of simulation.
    world_->Step(time_step_.count(), 10, 10);
    
    // Try to keep at the target FPS
    //if (renderer->is_visible) {
    //    clock.tick(fps);
    //    return clock.get_time();
    //}
    // self.clock.tick()
    return time_step_;
}


void Simulator::run_simulation(std::chrono::seconds time_s) {
    auto elapsed_ms = std::chrono::milliseconds::zero();
    std::chrono::milliseconds max_time_ms = time_s;
    while (elapsed_ms < max_time_ms)
        elapsed_ms += step();
    if (save_)
        writer_->close();
}


void Simulator::reset() {
    body_data_storage_.empty();
	world_ = std::make_unique<b2World>(b2Vec2(0.0, 0.0));
    create_screen_bounding_box_();
    if (save_)
        writer_->close();
//    self._renderer.reset()
    metadata_.empty();

    set_fps(DEFAULT_FPS);
}

void Simulator::quit() {
    //pygame.quit()
    if (save_)
        writer_->close();
}

//std::vector<std::vector<Point2f>> Simulator::retrieve_and_clean_metadata() {
//	std::vector<std::vector<Point2f>> meta;
//	std::swap(meta, metadata);
//	return meta;
//}

//void Simulator::collect_metadata() {
//    std::vector<Point2f> balls_coordinates;
//    for (const auto body : world->GetBodyList()) {
//        if (!get_user_data(body)->visible) continue;
//
//        // The fixture holds information like density, friction and shape.
//        for (const auto fixture : body->GetFixtureList()) {
//            const auto shape = fixture->GetShape();
//            if (shape->GetType() == b2Shape::e_circle) {
//                const auto &pos = body->GetPosition();
//                const auto center = renderer->to_screen_frame({pos.x, pos.y});
//                balls_coordinates.push_back(center);
//            }
//        }
//    }
//    metadata.push_back(std::move(balls_coordinates));
//}


BodyData* Simulator::create_body_data_(const std::string &name, bool visible) {
    body_data_storage_.emplace_back(name, visible);
    return &body_data_storage_.back();
}


}