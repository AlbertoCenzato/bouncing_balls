#include "video_renderer.hpp"

#include <vector>
#include <cassert>

#include "Box2D/Box2D.h"

namespace bounce {

const float VideoRenderer::PPM = 1.0f;


VideoRenderer::VideoRenderer(int32_t screen_width, int32_t screen_height, Channels channel_ordering) 
    : screen_height_px(screen_height), screen_width_px(screen_width), channel_ordering_(channel_ordering)
{ 
	assert(screen_height_px >= 0 && screen_width_px >= 0);
	auto h = static_cast<size_t>(screen_height_px);
	auto w = static_cast<size_t>(screen_width_px);
    if (channel_ordering == Channels::LAST)
        image_shape_ = {h, w, size_t(1)};
    else
        image_shape_ = {1, h, w};
}

float VideoRenderer::pixels_to_meters(float pixels) {
	return pixels / VideoRenderer::PPM;
}

float VideoRenderer::meters_to_pixels(float meters) {
	return meters * VideoRenderer::PPM;
}

b2Vec2 VideoRenderer::to_world_frame(const b2Vec2& point_px) const {
	auto x = VideoRenderer::pixels_to_meters(point_px.x);
	auto y = VideoRenderer::pixels_to_meters(screen_height_px - point_px.y);
	return { x, y };
}

b2Vec2 VideoRenderer::to_screen_frame(const b2Vec2 & point_m) const {
	auto x = VideoRenderer::meters_to_pixels(point_m.x);
	auto y = screen_height_px - VideoRenderer::meters_to_pixels(point_m.y);
	return { x, y };
}

        
cnpy::NpyArray VideoRenderer::get_frame(b2World *world) {
    cnpy::NpyArray image(image_shape_, 1, false);
    auto screen = image.data<uint8_t>();

    // Draw the world
    for (auto body : world->GetBodyList()) {
        // The body gives us the position and angle of its shapes
        const auto body_data = get_body_data(body);
        if (!body_data->visible) continue;

        for (auto fixture : body->GetFixtureList()) {
            // The fixture holds information like density and friction,
            // and also the shape.
            const auto shape = fixture->GetShape();
            const auto shape_type = shape->GetType();
            switch(shape_type) {
			case b2Shape::e_polygon: {
				const auto vertexes = get_shape_vertexes_<b2PolygonShape>(body, shape);
				draw_polygon_(vertexes, screen);
				break; }
			case b2Shape::e_edge: {
				const auto vertexes = get_shape_vertexes_<b2EdgeShape>(body, shape);
				draw_edge_(vertexes, screen);
				break; }
			case b2Shape::e_circle: {
				//auto circle = static_cast<b2CricleShape* const>(shape);
				const auto center_px = to_screen_frame(body->GetPosition());
				draw_circle_(center_px, meters_to_pixels(body_data->radius), screen);
				break; }
            }
        }
    }

    return image;
}



void VideoRenderer::draw_polygon_(const std::vector<b2Vec2>& vertexes, uint8_t *screen) {
    throw NotImplementedException();
}

void VideoRenderer::draw_edge_(const std::vector<b2Vec2>& vertexes, uint8_t *screen) {
    throw NotImplementedException();
}

void VideoRenderer::draw_circle_(const b2Vec2& center, int32_t radius, uint8_t *screen) {
    auto x0 = int32_t(center.x);
    auto y0 = int32_t(center.y);

    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y) {
		fill_line_(x0 - x, x0 + x, y0 + y, screen);
		fill_line_(x0 - x, x0 + x, y0 - y, screen);
		fill_line_(x0 - y, x0 + y, y0 + x, screen);
		fill_line_(x0 - y, x0 + y, y0 - x, screen);

        if (err <= 0) {
            ++y;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            --x;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}




}