#pragma once

#include <memory>
#include <stdexcept>

#include "../src/pglib/TransferMatrix.h"
#include "../src/pglib/XYZvLab.h"
#include "PhotoGoodyzer/Array.h"
#include "PhotoGoodyzer/ColorSpace.h"

namespace pg {

/// Template class for an image providing color space transformation.
///
/// The class may obtain already allocated array and use custom deleter function. The class
/// implements basic element-wise arithmetic operators (+, -, *, /) and folowing funсtions with the
/// use of expression templates: Abs(Image), Square(Image), Pow4(Image), Pow3(Image), Sqrt(Image),
/// Cbrt(Image), Pow(Image, value). Expression templates also support operations on std::vector.
template <typename T>
class Image : public Array<T> {
private:
    ColorSpace color_space_ = ColorSpace::RGB;

public:
    Image() = default;

    /// Constructs a blank image of given dimensions with the default array deleter.
    explicit Image(ColorSpace color_space, int width, int height, int num_of_channels) :
        Array<T>(width, height, num_of_channels), color_space_(color_space) {}

    /// Constructs an Image object that uses an existing memory buffer, data.
    /// The buffer must be continuous, row-majored, without separations and strides, with channels
    /// following each other (e.g. R, G, B, R, G, B...).
    /// The size of the buffer must be equal to width * height * num_of_channels.
    /// The buffer must remain valid throughout the life of the Image object. The Image does not
    /// delete the buffer at destruction unless a pointer to a cleanup_function is provided.
    /// @param color_space ColorSpace of an image
    /// @param ptr Pointer to an existing buffer
    /// @param width Width in pixels
    /// @param height Height in pixels
    /// @param num_of_channels Number of channels
    /// @param cleanup_function Function called when the Image object is destructed
    Image(ColorSpace color_space, T* ptr, int width, int height, int num_of_channels,
          std::function<void(T*)> cleanup_function = nullptr) :
        Array<T>(ptr, width, height, num_of_channels, cleanup_function),
        color_space_(color_space) {}

    /// Constructs an image from other image with transformation to desired ColorSpace;
    /// uses the default deleter.
    Image(const Image& other, ColorSpace desired_clrs) :
        Image(desired_clrs, other.GetWidth(), other.GetHeight(), other.GetNumOfChannels()) {
        auto this_ptr = this->begin();
        auto other_ptr = other.begin();
        if (other.GetColorSpace() == desired_clrs) {
            std::copy(other.begin(), other.end(), this->begin());
        } else {
            auto map_iter = DST_FROM_SRC.find({desired_clrs, other.GetColorSpace()});
            if (map_iter == DST_FROM_SRC.end()) {
                this->CheckNonLinearTransform(other, desired_clrs);
            } else {
                TransferMatrix tm = map_iter->second;
                T r, g, b;
                for (int _ = 0; _ != this->GetImgSize(); ++_) {
                    r = *other_ptr++;
                    g = *other_ptr++;
                    b = *other_ptr++;
                    *this_ptr++ = tm.row1[0] * r + tm.row1[1] * g + tm.row1[2] * b;
                    *this_ptr++ = tm.row2[0] * r + tm.row2[1] * g + tm.row2[2] * b;
                    *this_ptr++ = tm.row3[0] * r + tm.row3[1] * g + tm.row3[2] * b;
                }
            }
        }
        if (this->GetColorSpace() == ColorSpace::RGB) {
            this->Clip(0.0f, 1.0f);
        }
    }

    /// Constructs an image copy with the default deleter.
    Image(const Image& other) :
        Image(other.GetColorSpace(), other.GetWidth(), other.GetHeight(),
              other.GetNumOfChannels()) {
        std::copy(other.begin(), other.end(), this->begin());
    }

    /// Moves an image.
    Image(Image&& other) = default;

    ~Image() = default;

    const ColorSpace& GetColorSpace() const { return color_space_; }

    /// Convert the image to the desired ColorSpace in-place.
    void ChangeColorSpace(ColorSpace desired_clrs) {
        auto map_iter = DST_FROM_SRC.find({desired_clrs, this->GetColorSpace()});
        if (map_iter == DST_FROM_SRC.end()) {
            this->CheckNonLinearTransform(*this, desired_clrs);
        } else {
            TransferMatrix tm = map_iter->second;
            auto pixel_ptr = this->begin();
            auto color_ptr = this->begin();
            T r, g, b;
            for (int _ = 0; _ != this->GetImgSize(); ++_) {
                r = *color_ptr++;
                g = *color_ptr++;
                b = *color_ptr++;
                *pixel_ptr++ = tm.row1[0] * r + tm.row1[1] * g + tm.row1[2] * b;
                *pixel_ptr++ = tm.row2[0] * r + tm.row2[1] * g + tm.row2[2] * b;
                *pixel_ptr++ = tm.row3[0] * r + tm.row3[1] * g + tm.row3[2] * b;
            }
            color_space_ = desired_clrs;
        }
        if (this->GetColorSpace() == ColorSpace::RGB) {
            this->Clip(0.0f, 1.0f);
        }
    }

    Image& operator=(const Image& other) {
        *this = Image(other);
        return *this;
    }

    Image& operator=(Image&& other) = default;

    /// Convert an exression template to an image.
    template <class Callable, class... Operands>
    Image& operator=(const ImgExpr<Callable, Operands...>& expr) {
        Array<T>::operator=(expr);
        return *this;
    }

private:
    void CheckNonLinearTransform(const Image& src, ColorSpace desired_clrs) {
        if (desired_clrs == ColorSpace::Lab && src.GetColorSpace() == ColorSpace::XYZ) {
            LabFromXYZ(src);
        } else if (desired_clrs == ColorSpace::XYZ && src.GetColorSpace() == ColorSpace::Lab) {
            XYZFromLab(src);
        } else {
            throw std::runtime_error("There are no such transformation\n");
        }
    }

    void LabFromXYZ(const Image& img_XYZ) {
        auto XYZ_ptr = img_XYZ.begin();
        auto Lab_ptr = this->begin();
        for (int _ = 0; _ != img_XYZ.GetImgSize(); ++_) {
            T x = Labf_function(*XYZ_ptr++ / 0.950489f);    // for Standart Illuminnat D65
            T y = Labf_function(*XYZ_ptr++);
            T z = Labf_function(*XYZ_ptr++ / 1.088840f);
            *Lab_ptr++ = 116.0f * y - 16.0f;    // L
            *Lab_ptr++ = 500.0f * (x - y);      // a
            *Lab_ptr++ = 200.0f * (y - z);      // b
        }
        color_space_ = ColorSpace::Lab;
    }

    void XYZFromLab(const Image& img_Lab) {
        auto Lab_ptr = img_Lab.begin();
        auto XYZ_ptr = this->begin();
        for (int _ = 0; _ != img_Lab.GetImgSize(); ++_) {
            T var_Y = (*Lab_ptr++ + 16.0f) / 116.0f;
            T var_X = *Lab_ptr++ / 500.0f + var_Y;
            T var_Z = var_Y - *Lab_ptr++ / 200.0f;
            *XYZ_ptr++ = LabReversedf_function(var_X) * 0.950489f;    // X
            *XYZ_ptr++ = LabReversedf_function(var_Y);                // Y
            *XYZ_ptr++ = LabReversedf_function(var_Z) * 1.088840f;    // Z
        }
        color_space_ = ColorSpace::XYZ;
    }
};

/// Convert unsigned char sRGB image to float Linear RGB image.
Image<float> LinRGBFromSRGB(const Image<unsigned char>& src_sRGB);

/// Convert float Linear RGB image to unsigned char sRGB image.
Image<unsigned char> SRGBFromLinRGB(const Image<float>& src_rgb);

/// Convert unsigned char sRGB image to existing float Linear RGB image
void LinRGBFromSRGB(Image<float>& dst_linRGB, const Image<unsigned char>& src_sRGB);

/// Convert float Linear RGB image to existing unsigned char sRGB image.
void SRGBFromLinRGB(Image<unsigned char>& dst_sRGB, const Image<float>& src_linRGB);

}    // namespace pg
