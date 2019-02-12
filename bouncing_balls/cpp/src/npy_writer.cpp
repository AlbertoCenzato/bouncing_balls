#include "npy_writer.hpp"


namespace bounce {


NpyWriter::NpyWriter() : fps_(25), height_(0), width_(0) {}

NpyWriter::~NpyWriter() {
	close();
}

void NpyWriter::set_fps(int32_t fps) {
	this->fps_ = fps;
}

int32_t NpyWriter::get_fps() const {
	return fps_;
}

void NpyWriter::set_resolution(int32_t height, int32_t width) {
	this->height_ = height;
	this->width_ = width;
}

Vector2i NpyWriter::get_resolution() const {
	return { height_, width_ };
}

NpyWriter& NpyWriter::operator<<(const cnpy::NpyArray &frame) {
	write(frame);
	return *this;
}

bool NpyWriter::open(const fs::path &path) {
    tmp_frame_storage_.clear();
    this->path_ = path;
    return true;
}

bool NpyWriter::close() {
    auto sequence_length = tmp_frame_storage_.size();
    if (sequence_length == 0)
        return true;

    // build a new npy array with the correct dimensions
    const auto &sample_frame = tmp_frame_storage_[0];
    const auto &frame_shape  = sample_frame.shape;
    const auto word_size     = sample_frame.word_size;
    const auto fortran_order = sample_frame.fortran_order;
    const auto frame_size    = sample_frame.num_bytes();
    
    std::vector<size_t> sequence_shape = {sequence_length};
    sequence_shape.insert(sequence_shape.end(), frame_shape.begin(), frame_shape.end());

    cnpy::NpyArray array(sequence_shape, word_size, fortran_order);

    // copy frames to the new array
    auto array_data_iter = array.data<uint8_t>();
    for (const auto &frame : tmp_frame_storage_) {
        auto frame_ptr = frame.data<uint8_t>();
        array_data_iter = std::copy(frame_ptr, frame_ptr + frame_size, array_data_iter);
    }

    // save
    cnpy::npy_save(path_.string(), array.data<uint8_t>(), sequence_shape, "w");

    tmp_frame_storage_.clear();
    path_ = "";

    return true;
}   

bool NpyWriter::write(const cnpy::NpyArray &frame) {
    tmp_frame_storage_.push_back(frame);
	return true;
}



}