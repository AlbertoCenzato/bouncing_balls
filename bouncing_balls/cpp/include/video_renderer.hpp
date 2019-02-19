#ifndef BOUNCE_VIDEO_RENDERER_HPP
#define BOUNCE_VIDEO_RENDERER_HPP

#include <algorithm>

#include "Box2D/Box2D.h"

#include "common_types.hpp"


namespace bounce {


/**
 * This class has the role of rendering the simulated physical world.
 */
class VideoRenderer {
    
public:

	// Box2D deals with meters, but we want to display pixels,
	// so define a conversion factor:
	static const float PPM;  // pixels per meter
	const int32_t screen_height_px, screen_width_px;

	static float pixels_to_meters(float pixels);
	static float meters_to_pixels(float meters);

    /**
     * Args:
     *   - screen_width:  width (in pixels) of the rendered scene
     *   - screen_height: heigth (in pixels) of the rendered scene
     *   - channel_ordering: Channel enum specifying if get_frame() returns
     *                     (channels, height, width) images or
     *                     (height, width, channels) images
     */
    VideoRenderer(int32_t screen_height, int32_t screen_width, Channels channel_ordering);     

	b2Vec2 to_world_frame(const b2Vec2 &point_px) const;
	b2Vec2 to_screen_frame(const b2Vec2 &point_m) const;
        
    cnpy::NpyArray get_frame(b2World *world);

	template<class ShapeType>
	std::vector<b2Vec2> get_shape_vertexes_(b2Body *body, b2Shape* shape);

	template<>
	std::vector<b2Vec2> get_shape_vertexes_<b2PolygonShape>(b2Body *body, b2Shape* shape) {
		auto poly = static_cast<b2PolygonShape*>(shape);
		const auto num_vertexes = poly->GetVertexCount();
		std::vector<b2Vec2> vertexes;
		for (int32_t i = 0; i < num_vertexes; ++i) {
			const auto &vertex_local = poly->GetVertex(i);
			const auto vertex_world = body->GetWorldPoint(vertex_local);
			vertexes.push_back(to_screen_frame(vertex_world));
		}

		return vertexes;
	}

	template<>
	std::vector<b2Vec2> get_shape_vertexes_<b2EdgeShape>(b2Body *body, b2Shape* shape) {
		auto edge = static_cast<b2EdgeShape*>(shape);
		const auto vertex0 = body->GetWorldPoint(edge->m_vertex0);
		const auto vertex1 = body->GetWorldPoint(edge->m_vertex1);

		return { to_screen_frame(vertex0), to_screen_frame(vertex1) };
	}

private:

    Channels channel_ordering_;
    std::vector<size_t> image_shape_;

	void draw_polygon_(const std::vector<b2Vec2>& vertexes, uint8_t *screen);
    void draw_edge_(const std::vector<b2Vec2>& vertexes, uint8_t *screen);
    void draw_circle_(const b2Vec2& center, int32_t radius, uint8_t *screen);

    void putpixel_(int32_t x, int32_t y, uint8_t *screen);

	void fill_line_(int32_t x_begin, int32_t x_end, int32_t y, uint8_t *screen) const;

};


inline void VideoRenderer::putpixel_(int32_t x, int32_t y, uint8_t *screen) {
    if (x < 0 || x >= screen_width_px || y < 0 || y >= screen_height_px)
        return;
    
    screen[y * screen_width_px + x] = 0xFF;
}

inline void VideoRenderer::fill_line_(int32_t x_begin, int32_t x_end, 
									  int32_t y, uint8_t *screen) const
{
	if (x_end < 0 || x_begin >= screen_width_px || y < 0 || y >= screen_height_px)
		return;

	if (x_begin < 0) 
		x_begin = 0;
	if (x_end >= screen_width_px)
		x_end = screen_width_px - 1;

	auto line_index = screen + y * screen_width_px;
	std::fill(line_index + x_begin, line_index + x_end, 0xFF);
}

}


#endif