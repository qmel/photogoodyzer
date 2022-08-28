#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

#include "../src/pglib/ImgExpr.h"
#include "PhotoGoodyzer/ArrayBase.h"

namespace pg {

/// Template class for 3-dimensional array representaion; base for the Image and Channel classes.
///
/// The class may obtain already allocated array and use custom deleter function. The class
/// implements basic element-wise arithmetic operators (+, -, *, /) and folowing funсtions with the
/// use of expression templates: Abs(Array), Square(Array), Pow4(Array), Pow3(Array), Sqrt(Array),
/// Cbrt(Array), Pow(Array, value). Expression templates also support operations on std::vector.
template <typename T>
class Array : public ArrayBase {
private:
    /// Data array
    std::unique_ptr<T[], std::function<void(T*)>> data_ = {nullptr, nullptr};

public:
    Array() = default;

    /// Constructs a blank array of given dimensions with the default deleter.
    Array(int width, int height, int num_of_channels) :
        ArrayBase(width, height, num_of_channels),
        data_(new T[this->size()], [](T* ptr) { delete[] ptr; }) {}

    /// Constructs an Array object that uses an existing memory buffer, data.
    /// The buffer must be continuous, row-majored, without separations and strides, with channels
    /// following each other (e.g. R, G, B, R, G, B...).
    /// The size of the buffer must be equal to width * height * num_of_channels.
    /// The buffer must remain valid throughout the life of the Array object. The Array does not
    /// delete the buffer at destruction unless a pointer to a cleanup_function is provided
    /// @param ptr Pointer to an existing buffer
    /// @param width Width in pixels
    /// @param height Height in pixels
    /// @param num_of_channels Number of channels
    /// @param cleanup_function Function called when the Array object is destructed
    Array(T* ptr, int width, int height, int num_of_channels,
          std::function<void(T*)> cleanup_function = nullptr) :
        ArrayBase(width, height, num_of_channels), data_(ptr, cleanup_function) {}

    /// Constructs an Array copy with the default deleter.
    Array(const Array& other) :
        Array(other.GetWidth(), other.GetHeight(), other.GetNumOfChannels()) {
        std::copy(other.begin(), other.end(), this->begin());
    }

    /// Moves an Array.
    Array(Array&& other) = default;

    ~Array() = default;

    Array& operator=(const Array& other) {
        *this = Array(other);
        return *this;
    }

    Array& operator=(Array&& other) = default;

    /// Convert an exression template to the Array.
    template <class Callable, class... Operands>
    void operator=(const ImgExpr<Callable, Operands...>& expr) {
        if (this->size() != expr.size())
            throw std::runtime_error("Sizes of an object and an expression must be equal");
        for (size_t i = 0; i < this->size(); ++i) {
            (*this)[i] = expr[i];
        }
    }

    /// Gives an access to an element in the array, max index = ArrayBase::size() - 1
    T& operator[](size_t i) { return data_[i]; }

    /// Return a value an element in the array, max index = ArrayBase::size() - 1
    T operator[](size_t i) const { return data_[i]; }

    /// Checks whether two arrays have same dimensions and pixel data.
    bool operator==(const Array<T>& rhs) {
        if (!AreEqualDimensions(*this, rhs)) {
            return false;
        } else {
            return IsEveryPixelEqual(rhs);
        }
    }

protected:
    bool IsEveryPixelEqual(const Array<T>& rhs) {
        auto rhs_ptr = rhs.begin();
        for (auto this_pix : *this) {
            if (this_pix != *rhs_ptr++)
                return false;
        }
        return true;
    }

public:
    bool empty() const { return !data_; }
    T* begin() const { return data_.get(); }
    T* end() const { return std::next(data_.get(), this->size()); }
    bool operator!() const { return !data_; }
    explicit operator bool() const { return bool(data_); }

    /// Fill a data array with value
    void Fill(T value) {
        for (auto& pix : *this)
            pix = value;
    }

    void operator-=(T value) {
        for (auto& pix : *this)
            pix -= value;
    }

    void operator+=(T value) {
        for (auto& pix : *this)
            pix += value;
    }

    void operator*=(T value) {
        for (auto& pix : *this)
            pix *= value;
    }

    void operator/=(T value) {
        for (auto& pix : *this)
            pix /= value;
    }

    /// Clips all values in data array to specified lower and upper bounds.
    void Clip(T lower_bound, T upper_bound) {
        for (auto& pix : *this) {
            if (pix < lower_bound)
                pix = lower_bound;
            if (pix > upper_bound)
                pix = upper_bound;
        }
    }

    /// Provides std::pow() for all values of a data array in-place
    void Pow(T value) {
        for (auto& pix : *this)
            pix = std::pow(pix, value);
    }

    /// Provides std::abs() for all values of a data array in-place
    void Abs() {
        for (auto& pix : *this)
            pix = std::abs(pix);
    }

    /// True if a data array has at least one NaN value
    bool HasNan() {
        for (auto& pix : *this) {
            if (pix != pix)
                return true;
        }
        return false;
    }
};

/// Checks whether two data arrays have same dimensions.
template <typename T1, typename T2>
bool AreEqualDimensions(const Array<T1>& lhs, const Array<T2>& rhs) {
    return (std::make_tuple(lhs.GetWidth(), lhs.GetHeight(), lhs.GetImgSize(),
                            lhs.GetNumOfChannels(), lhs.size()) ==
            std::make_tuple(rhs.GetWidth(), rhs.GetHeight(), rhs.GetImgSize(),
                            rhs.GetNumOfChannels(), rhs.size()));
}

/// Returns <min1, max1, min2, max2 ...> values of a data array
template <typename T>
std::vector<T> MinMaxValues(const Array<T>& img) {
    std::vector<T> result(img.GetNumOfChannels() * 2);
    auto iter = img.begin();
    for (int i = 0; i != img.GetNumOfChannels(); ++i) {
        result[2 * i] = *iter;
        result[2 * i + 1] = *iter++;
    }
    for (int _ = 1; _ != img.GetImgSize(); ++_) {
        for (int i = 0; i != img.GetNumOfChannels(); ++i) {
            result[2 * i] = std::min(result[2 * i], *iter);
            result[2 * i + 1] = std::max(result[2 * i + 1], *iter++);
        }
    }
    return result;
}

}    // namespace pg
