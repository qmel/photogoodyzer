#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "../src/pglib/Equalizer.h"
#include "PhotoGoodyzer/Array.h"

/// Functions and classes in this namespace provide basic image manipulations
/// such as element-wise arithmetic operations, converting to different colorspaces, channel
/// copying, croping, histogram equalization, basic mathematical functions. Classes use template
/// type and custom deleter function, allowing to perform operations on already allocated data.
/// Classes use expression templates.
namespace pg {

/// Template class for a single channel of an image, providing manipulations such as rescaling,
/// equalization, cropping, etc.
///
/// The class may obtain already allocated array and use custom deleter function. The class
/// implements basic element-wise arithmetic operators (+, -, *, /) and folowing funсtions with the
/// use of expression templates: Abs(Channel), Square(Channel), Pow4(Channel), Pow3(Channel),
/// Sqrt(Channel), Cbrt(Channel), Pow(Channel, value). Expression templates also support operations
/// on std::vector.
template <typename T>
class Channel : public Array<T> {
private:
    /// Equalizer class object needed to perform histogram equalization of the channel
    mutable std::shared_ptr<Equalizer<T>> eq_ = nullptr;

public:
    Channel() = default;

    /// Constructs a blank channel of given dimensions with the default array deleter.
    explicit Channel(int width, int height) : Array<T>(width, height, 1) {}

    /// Constructs an Channel object that uses an existing memory buffer, data.
    /// The buffer must be continuous, row-majored, without separations and strides.
    /// The size of the buffer must be equal to width * height.
    /// The buffer must remain valid throughout the life of the Channel object. The Channel does not
    /// delete the buffer at destruction unless a pointer to a cleanup_function is provided.
    /// @param ptr Pointer to an existing buffer
    /// @param width Width in pixels
    /// @param height Height in pixels
    /// @param cleanup_function Function called when the Channel object is destructed
    Channel(T* ptr, int width, int height, std::function<void(T*)> cleanup_function = nullptr) :
        Array<T>(ptr, width, height, 1, cleanup_function) {}

    /// Constructs a channel copy with the default deleter.
    Channel(const Channel& other) : Channel(other.GetWidth(), other.GetHeight()) {
        std::copy(other.begin(), other.end(), this->begin());
    }

    /// Moves a channel.
    Channel(Channel&& other) = default;

    Channel& operator=(const Channel& other) {
        *this = Channel(other);
        return *this;
    }

    Channel& operator=(Channel&& other) = default;

    /// Convert an exression template to a Channel.
    template <class Callable, class... Operands>
    Channel& operator=(const ImgExpr<Callable, Operands...>& expr) {
        Array<T>::operator=(expr);
        return *this;
    }

    Channel& operator=(const Array<T>& other) {
        if (other.GetNumOfChannels() != 1)
            throw std::runtime_error("For conversion an Array must have only 1 channel");
        Array<T>::operator=(other);
        return *this;
    }

    Channel& operator=(Array<T>&& other) {
        if (other.GetNumOfChannels() != 1)
            throw std::runtime_error("For conversion an Array must have only 1 channel");
        Array<T>::operator=(std::move(other));
        return *this;
    }

    /// Rescale the channel in a way that values in the [in_min... in_max] range became values
    /// in the [out_min... out_max] range; values outside the [in_min... in_max] range are cliped to
    /// out_min, out_max
    void Rescale(T in_min, T in_max, T out_min = 0.0f, T out_max = 1.0f) {
        for (auto& pix : *this) {
            if (pix <= in_min) {
                pix = out_min;
            } else if (pix >= in_max) {
                pix = out_max;
            } else {
                pix = (pix - in_min) / (in_max - in_min) * (out_max - out_min) + out_min;
            }
        }
    }

    /// Return [lower, upper] values of the channel corresponding to lower_b, upper_b percentile in
    /// decimal form. For example, Percentile(0.25, 0.75) will return values corresponding to 25%
    /// and 75% of the array from lowest to highest values.
    std::pair<T, T> Percentile(float lower_b = 0.0f, float upper_b = 1.0f) const {
        if (!eq_)
            eq_ = std::make_shared<Equalizer<T>>(*this);
        T lower = eq_->FindLowerPercentile(lower_b);
        T upper = eq_->FindUpperPercentile(upper_b);
        return {lower, upper};
    }

    /// Perform histogram equalization of the channel; values of the channel tend to be distributed
    /// evenly between out_min and out_max. Values outside the [out_min... out_max] range are cliped
    /// to out_min, out_max.
    void Equalize(T out_min = 0, T out_max = 1) {
        if (!eq_)
            eq_ = std::make_shared<Equalizer<T>>(*this);
        eq_->ExportEqualized(*this, out_min, out_max);
    }
};

/// Crops the channel, excluding width_field from the left and right side,
/// and height_field from the top and botom side of the channel.
template <typename T>
Channel<T> Crop(const Channel<T>& other, int width_field, int heigth_field) {
    Channel<T> dst(other.GetWidth() - 2 * width_field, other.GetHeight() - 2 * heigth_field);
    auto line_begin = std::next(other.begin(), other.GetWidth() * heigth_field + width_field);
    auto line_end = std::next(line_begin, dst.GetWidth());
    for (auto iter = dst.begin(); iter != dst.end(); std::advance(iter, dst.GetWidth())) {
        std::copy(line_begin, line_end, iter);
        std::advance(line_begin, other.GetWidth());
        std::advance(line_end, other.GetWidth());
    }
    return dst;
}

/// Constructs a specified Channel object copying data from an Array.
/// @param src Source Array
/// @param channel_bias The channel bias in the Array. channel_bias
/// equals 0 for the first channel, equals 1 for the second channel, etc.
/// @returns New constructed channel.
template <typename T>
Channel<T> CopyChannel(const Array<T> src, int channel_bias) {
    if (channel_bias >= src.GetNumOfChannels())
        throw std::runtime_error(
            "Channel bias cannot be greater or equal to the number of channels");
    Channel<T> dst(src.GetWidth(), src.GetHeight());
    auto src_ptr = std::next(src.begin(), channel_bias);
    for (auto& pix : dst) {
        pix = *src_ptr;
        std::advance(src_ptr, src.GetNumOfChannels());
    }
    return dst;
}

/// Loads the data from the src channel to the specified channel of the dst Array.
/// @param dst Destination Array
/// @param src Source channel
/// @param channel_bias The channel bias in the Array. channel_bias
/// equals 0 for the first channel, equals 1 for the second channel, etc.
template <typename T>
void LoadFromChannel(Array<T>& dst, const Channel<T>& src, int channel_bias) {
    if (channel_bias >= dst.GetNumOfChannels())
        throw std::runtime_error(
            "Channel bias cannot be greater or equal to the number of channels");
    if (dst.GetImgSize() != src.GetImgSize())
        throw std::runtime_error("Sizes must be equal");
    auto dst_ptr = std::next(dst.begin(), channel_bias);
    for (auto pix : src) {
        *dst_ptr = pix;
        std::advance(dst_ptr, dst.GetNumOfChannels());
    }
}

/// Loads the data from the specified channels to the all channels of the dst Array.
/// Number of channels in the Array must be equal to the vector size; sizes of the channels and the
/// Array must be equal. First channel in the vector will be the first channel in the dst Array.
/// @param dst Destination Array
/// @param channels std::vector of source channels
template <typename T>
void LoadFromChannels(Array<T>& dst, const std::vector<Channel<T>>& channels) {
    if (dst.GetNumOfChannels() != int(channels.size()))
        throw std::runtime_error("Number of channels in Array must be equal to vector size");
    std::vector<T*> iters;
    for (const Channel<T>& channel : channels) {
        if (dst.GetImgSize() != channel.GetImgSize()) {
            throw std::runtime_error("Sizes of channels and Array must be equal");
        }
        iters.push_back(channel.begin());
    }
    auto dst_iter = dst.begin();
    for (int _ = 0; _ != dst.GetImgSize(); ++_) {
        for (int i = 0; i != int(iters.size()); ++i) {
            *dst_iter++ = *iters[i]++;
        }
    }
}

}    // namespace pg
