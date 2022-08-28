#pragma once

#include <cstddef>

namespace pg {

/// Base for the Array class characterizing dimensions of a data array.
class ArrayBase {
private:
    int width_ = 0;
    int height_ = 0;

    /// img_size_ = width_* height_
    int img_size_ = 0;
    int num_of_channels_ = 0;

    /// array_size_ = width_* height_ * num_of_channels
    std::size_t array_size_ = 0;

protected:
    ArrayBase() = default;

    ArrayBase(int width, int height, int num_of_channels);

public:
    /// Returns size of the data array (width_* height_ * num_of_channels_)
    /// @return array_size_ = width_* height_ * num_of_channels_.
    size_t size() const;

    /// Returns width of the data array
    /// @return width of the array
    int GetWidth() const;

    /// Returns height of the array
    /// @return height_ of the array
    int GetHeight() const;

    /// Returns image_size_ = width_* height_
    /// @return image_size_ = width_* height_.
    int GetImgSize() const;

    /// Returns number of channels in the array (depth of the array)
    /// @return number of channels in the array (depth of the array)
    int GetNumOfChannels() const;
};

}    // namespace pg
