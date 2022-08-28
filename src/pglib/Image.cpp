#include "PhotoGoodyzer/Image.h"

#include <stdexcept>

#include "PhotoGoodyzer/sRGBvLinRGB.h"

namespace pg {

Image<float> LinRGBFromSRGB(const Image<unsigned char>& src_sRGB) {
    if (src_sRGB.GetColorSpace() != ColorSpace::sRGB)
        throw std::runtime_error("Source image must be in sRGB");
    Image<float> img(ColorSpace::RGB, src_sRGB.GetWidth(), src_sRGB.GetHeight(),
                     src_sRGB.GetNumOfChannels());
    auto this_ptr = img.begin();
    auto src_ptr = src_sRGB.begin();
    size_t array_size = img.size();
    for (size_t _ = 0; _ != array_size; ++_)
        *this_ptr++ = sRGB_to_linRGB(*src_ptr++);
    return img;
}

Image<unsigned char> SRGBFromLinRGB(const Image<float>& src_rgb) {
    if (src_rgb.GetColorSpace() != ColorSpace::RGB)
        throw std::runtime_error("Source image must be in Linear RGB");
    Image<unsigned char> img(ColorSpace::sRGB, src_rgb.GetWidth(), src_rgb.GetHeight(),
                             src_rgb.GetNumOfChannels());
    auto this_ptr = img.begin();
    auto src_ptr = src_rgb.begin();
    size_t array_size = img.size();
    for (size_t _ = 0; _ != array_size; ++_)
        *this_ptr++ = linRGB_to_sRGB(*src_ptr++);
    return img;
}

void LinRGBFromSRGB(Image<float>& dst_linRGB,
                    const Image<unsigned char>& src_sRGB) {
    if (src_sRGB.GetColorSpace() != ColorSpace::sRGB)
        throw std::runtime_error("Source image must be in sRGB");
    if (dst_linRGB.GetColorSpace() != ColorSpace::RGB)
        throw std::runtime_error("Destination image must be in Linear RGB");
    if (!AreEqualDimensions(dst_linRGB, src_sRGB))
        throw std::runtime_error("Dimensions must be equal");
    auto dst_ptr = dst_linRGB.begin();
    auto src_ptr = src_sRGB.begin();
    size_t array_size = dst_linRGB.size();
    for (size_t _ = 0; _ != array_size; ++_)
        *dst_ptr++ = sRGB_to_linRGB(*src_ptr++);
}

void SRGBFromLinRGB(Image<unsigned char>& dst_sRGB,
                    const Image<float>& src_linRGB) {
    if (src_linRGB.GetColorSpace() != ColorSpace::RGB)
        throw std::runtime_error("Source image must be in Linear RGB");
    if (dst_sRGB.GetColorSpace() != ColorSpace::sRGB)
        throw std::runtime_error("Destination image must be in sRGB");
    if (!AreEqualDimensions(dst_sRGB, src_linRGB))
        throw std::runtime_error("Dimensions must be equal");
    auto dst_ptr = dst_sRGB.begin();
    auto src_ptr = src_linRGB.begin();
    size_t array_size = dst_sRGB.size();
    for (size_t _ = 0; _ != array_size; ++_)
        *dst_ptr++ = linRGB_to_sRGB(*src_ptr++);
}

}    // namespace pg
