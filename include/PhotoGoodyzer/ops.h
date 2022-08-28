#pragma once

#include "PhotoGoodyzer/Channel.h"
#include "PhotoGoodyzer/Image.h"

/// Functions in this namespace provide advanced image operations mainly based on
/// <a href="https://doi.org/10.1016/j.jvcir.2007.06.003">ICam06</a> and
/// <a href="https://doi.org/10.1002/col.22131">CAM16</a> Color Appearance Models, for example local
/// lightness adaptation, color temperature correction, etc. Currently these functions support
/// Image<float> and Channel<float> classes with default deleters only, therefore one may need
/// to convert existing data in the specified classes to implement them.
namespace pg::ops {

/// Resizes the channel to the desired dimensions; currently works only for Channel<float> using
/// stb_image_resize.h.
Array<float> Resize(const Array<float>& other, int new_width, int new_height);

/// Performs advanced gamma compresiion based on iCam06, CAM16 model; images must be in
/// ColorSpace::LMS. // Currently works only for Image<float> and Channel<float>
Image<float> CAMCompress(const Image<float>& src, const Channel<float>& adapt_matrix,
                         const Channel<float>& ref_white, float gamma);

/// Performs gamma decompresiion based on iCam06, CAM16 model; images must be in ColorSpace::LMS.
/// Currently works only for Image<float> and Channel<float>
Image<float> CAMDecompress(const Image<float>& LMS, float gamma);

/// Performs local lightness adaptation (reduces local over/under exposion); images must be in
/// ColorSpace::XYZ. Currently works only for Image<float>
Image<float> LocLightAdapt(const Image<float>& XYZ);

/// Correct apparent illuminant temperature to D65; images must be in
/// ColorSpace::Lab. Currently works only for Image<float>
Image<float> CorrectColorTemperature(const Image<float>& img_lab_src);

/// Computes the distance map from the channel; Currently works only for Channel<float>
Channel<float> MakeDistMap(const Channel<float>& other);

/// Applies Gaussian Blur to the channel. Currently works only for Channel<float>
Channel<float> ApplyGaussianBlur(const Channel<float>& src, int scale_parameter = 2);

/// Downscale the channel to the target size. Currently works only for Channel<float>
Channel<float> Downscale(const Channel<float>& other, int target_size = 128);

/// Pads a channel with horizontal and vertical fields reflected to the channel. Currently works
/// only for Channel<float>
Channel<float> PadReflect(const Channel<float>& other, int add_width, int add_height);

/// Computes iCAM06 based adaptation matrix used for @ref IPTAdapt(const Image<float>&, float).
/// Currently works only for Channel<float>
Channel<float> GetAdaptMatrix(const Channel<float>& white);

/// Performs iCam06 based IPT adaptation; images must be in ColorSpace::XYZ. Currently works only
/// for Channel<float>
Image<float> IPTAdapt(const Image<float>& XYZ, float max_L = 16250.0f);

/// Correct black and white points in source ColorSpace::RGB image, transforms it to
/// ColorSpace::Lab and returns the lightness channel. Currently works only for Channel<float> and
/// Image<float>
Channel<float> RgbToBWCorrectedLab(Image<float>& img_rgb);

/// Performs histogram equalization of the lightness channel, copies it to the source
/// ColorSpace::Lab image and transforms the image to ColorSpace::XYZ. Currently works only for
/// Channel<float> and Image<float>
Image<float> GetEqualizedXYZFromLab(const Image<float>& src_Lab, Channel<float>& lightness);

}    // namespace pg::ops
