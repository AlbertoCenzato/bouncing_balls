#ifndef BOUNCE_NPY_WRITER_HPP
#define BOUNCE_NPY_WRITER_HPP

#include <filesystem>

#include "common_types.hpp"


namespace fs = std::filesystem;

namespace bounce {

class NpyWriter {

    std::vector<cnpy::NpyArray> tmp_frame_storage_;
    fs::path path_;
	int32_t fps_;
	int32_t height_;
	int32_t width_;

public:

	NpyWriter();
	~NpyWriter();

	void set_fps(int32_t fps);
	int32_t get_fps() const;

	void set_resolution(int32_t height, int32_t width);
	Vector2i get_resolution() const;

    bool open(const fs::path &path);
    bool close();   
    bool write(const cnpy::NpyArray &frame);

	NpyWriter& operator<<(const cnpy::NpyArray &frame);
};

}


#endif