#include "PhotoGoodyzer/ArrayBase.h"

namespace pg {

ArrayBase::ArrayBase(int width, int height, int num_of_channels) :
    width_(width),
    height_(height),
    img_size_(width * height),
    num_of_channels_(num_of_channels),
    array_size_(width * height * num_of_channels) {}

size_t ArrayBase::size() const {
    return array_size_;
}

int ArrayBase::GetWidth() const {
    return width_;
}

int ArrayBase::GetHeight() const {
    return height_;
}

int ArrayBase::GetImgSize() const {
    return img_size_;
}
int ArrayBase::GetNumOfChannels() const {
    return num_of_channels_;
}

}    // namespace pg
