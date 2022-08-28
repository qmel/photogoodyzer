#include "PhotoGoodyzer/ops.h"

#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

#include "Equalizer.h"
#include "FFT.h"
#include "ImgExpr.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

namespace pg::ops {

Array<float> Resize(const Array<float>& other, int new_width, int new_height) {
    Array<float> dst(new_width, new_height, other.GetNumOfChannels());
    stbir_resize_float(other.begin(), other.GetWidth(), other.GetHeight(), 0, dst.begin(),
                       new_width, new_height, 0, dst.GetNumOfChannels());
    return dst;
}

Image<float> CAMCompress(const Image<float>& src, const Channel<float>& adapt_matrix,
                         const Channel<float>& ref_white, float gamma) {
    if (src.GetColorSpace() != ColorSpace::LMS) {
        throw std::runtime_error("Only for LMS images");
    } else if (std::make_pair(src.GetWidth(), src.GetHeight()) !=
                   std::make_pair(adapt_matrix.GetWidth(), adapt_matrix.GetHeight()) ||
               std::make_pair(src.GetWidth(), src.GetHeight()) !=
                   std::make_pair(ref_white.GetWidth(), ref_white.GetHeight())) {
        throw std::runtime_error("Widths and heights of image and matrices must be equal");
    }
    Image<float> dst(src.GetColorSpace(), src.GetWidth(), src.GetHeight(), src.GetNumOfChannels());
    auto src_iter = src.begin();
    auto white_iter = ref_white.begin();
    auto dst_iter = dst.begin();
    for (auto adapt_iter = adapt_matrix.begin(); adapt_iter != adapt_matrix.end();) {
        float fl_div_w = *adapt_iter / *white_iter;
        for (int _ = 0; _ != 3; ++_) {
            if (*src_iter < 0.0f) {
                float new_val = std::pow(-fl_div_w * *src_iter, gamma);
                *dst_iter = new_val / (new_val + 27.13f) * (-400.0f) + 0.1f;
            } else {
                float new_val = std::pow(fl_div_w * *src_iter, gamma);
                *dst_iter = new_val / (new_val + 27.13f) * 400.0f + 0.1f;
            }
            dst_iter++;
            src_iter++;
        }
        adapt_iter++;
        white_iter++;
    }
    return dst;
}

Image<float> CAMDecompress(const Image<float>& LMS, float gamma) {
    if (LMS.GetColorSpace() != ColorSpace::LMS) {
        throw std::runtime_error("For LMS images only");
    } else {
        Image<float> dst(LMS.GetColorSpace(), LMS.GetWidth(), LMS.GetHeight(),
                         LMS.GetNumOfChannels());
        auto src_iter = LMS.begin();
        for (auto dst_iter = dst.begin(); dst_iter != dst.end(); ++dst_iter) {
            float val = *src_iter++ - 0.1f;
            float sign = float(0.0f < val) - (val < 0.0f);
            val = std::abs(val);
            // also * 100/FL in original CAM16
            *dst_iter = sign * std::pow(27.13f * val / (400.0f - val), 1.0f / gamma);
        }
        return dst;
    }
}

Image<float> LocLightAdapt(const Image<float>& XYZ) {
    if (XYZ.GetColorSpace() != ColorSpace::XYZ) {
        throw std::runtime_error("Only for XYZ images");
    }
    float max_L = 16250.0f;    // maximum luminance(cd/m2): max_L = 20,000;
    Channel<float> white = CopyChannel(XYZ, 1);
    int src_w = white.GetWidth();
    int src_h = white.GetHeight();
    auto maxY = MinMaxValues(white)[1];
    white *= (max_L / maxY);    // Y to normalized luminance
    white = Downscale(white);
    white = ApplyGaussianBlur(white);
    white = Resize(white, src_w, src_h);
    // Cone response / Tone compression and Local lightness adaptation due to iCam06
    Channel<float> FL = GetAdaptMatrix(white);
    Image<float> result(XYZ.GetColorSpace(), XYZ.GetWidth(), XYZ.GetHeight(),
                        XYZ.GetNumOfChannels());
    result = XYZ * (max_L / maxY);
    result.ChangeColorSpace(ColorSpace::LMS);
    result = CAMCompress(result, FL, white, 0.7f);    // gamma in Icam06 = 0.7 (0.6<p<0.85); indoor
                                                      // scene prefer low p values, in CAM16 = 0.42
    // There are other functions in iCam06 here, but it seems that their influence is negligible
    result.ChangeColorSpace(ColorSpace::XYZ);
    return result;
}

Image<float> CorrectColorTemperature(const Image<float>& img_lab_src) {
    if (img_lab_src.GetColorSpace() != ColorSpace::Lab) {
        throw std::runtime_error("Only for Lab images");
    } else {
        float mean_a = 0.0f;
        float mean_b = 0.0f;
        auto src_ptr = img_lab_src.begin();
        for (int _ = 0; _ != img_lab_src.GetImgSize(); ++_) {
            float L = *src_ptr++ / 100.0f;
            mean_a += (*src_ptr++ * L);
            mean_b += (*src_ptr++ * L);
        }
        mean_a /= img_lab_src.GetImgSize();
        mean_b /= img_lab_src.GetImgSize();
        Image<float> result(ColorSpace::Lab, img_lab_src.GetWidth(), img_lab_src.GetHeight(),
                            img_lab_src.GetNumOfChannels());
        src_ptr = img_lab_src.begin();
        auto result_ptr = result.begin();
        for (int _ = 0; _ != img_lab_src.GetImgSize(); ++_) {
            float L = *src_ptr++;
            *result_ptr++ = L;
            *result_ptr++ = (*src_ptr++ - mean_a * L / 100.0f);    // a
            *result_ptr++ = (*src_ptr++ - mean_b * L / 100.0f);    // b
        }
        return result;
    }
}

Channel<float> MakeDistMap(const Channel<float>& other) {
    Channel<float> result(other.GetWidth(), other.GetHeight());
    auto iter = result.begin();
    for (int i = 0; i != result.GetHeight(); ++i) {
        for (int j = 0; j != result.GetWidth(); ++j) {
            int arg_j = std::min(j, result.GetWidth() - j);
            int arg_i = std::min(i, result.GetHeight() - i);
            *iter++ = std::sqrt(float(arg_j * arg_j + arg_i * arg_i));
        }
    }
    return result;
}

Channel<float> ApplyGaussianBlur(const Channel<float>& src, int scale_parameter) {
    int width_field = src.GetWidth() / 2;
    int height_field = src.GetHeight() / 2;
    int max_dim = std::max(src.GetWidth(), src.GetHeight());
    Channel<float> white = PadReflect(src, width_field, height_field);
    Channel<float> kernel = MakeDistMap(white);
    auto kernel_iter = kernel.begin();
    for (int _ = 0; _ != kernel.GetImgSize(); ++_) {
        auto value = *kernel_iter * scale_parameter / max_dim;
        *kernel_iter = std::exp(-value * value);
        kernel_iter++;
    }
    FFTr2c filter(kernel.begin(), kernel.GetWidth(), kernel.GetHeight());
    filter.ForwardTransform();
    filter.ReduceImagine();
    filter.ClipNegativeOutRealToZero();
    filter.RemoveOutZeroFreq();
    FFTr2c FFT_src(white.begin(), white.GetWidth(), white.GetHeight());
    FFT_src.ForwardTransform();
    FFT_src.MultiplyOutByRealOut(filter);    // apply filter
    FFT_src.InverseTransform();
    return Crop(white, width_field, height_field);
}

Channel<float> Downscale(const Channel<float>& other, int target_size) {
    int min_dim = std::min(other.GetWidth(), other.GetHeight());
    int step;
    min_dim <= target_size ? step = 1 : step = min_dim / target_size;
    if (step == 1) {
        Channel<float> dst(other);
        return dst;
    } else {
        int dst_width = other.GetWidth() / step;
        int rem_x = other.GetWidth() - (dst_width * step);
        int dst_height = other.GetHeight() / step;
        Channel<float> dst(dst_width, dst_height);
        auto dst_iter = dst.begin();
        auto src_iter = other.begin();
        for (int i = 0; i != dst_height; ++i) {
            std::vector<float> current_row(dst_width, 0);
            for (int col_steps = 0; col_steps != step; ++col_steps) {
                for (int j = 0; j != dst_width; ++j) {
                    auto after_step_iter = std::next(src_iter, step);
                    current_row[j] += std::accumulate(src_iter, after_step_iter, 0.0f);
                    src_iter = after_step_iter;
                }
                std::advance(src_iter, rem_x);
            }
            for (int j = 0; j != dst_width; ++j) {
                *dst_iter++ = current_row[j] / step / step;
            }
        }
        return dst;
    }
}

Channel<float> PadReflect(const Channel<float>& other, int add_width, int add_height) {
    Channel<float> dst(other.GetWidth() + 2 * add_width, other.GetHeight() + 2 * add_height);
    auto dst_iter = std::next(dst.begin(), dst.GetWidth() * add_height);
    auto other_iter = other.begin();
    // main body
    for (int _ex = 0; _ex != other.GetHeight(); ++_ex) {
        std::advance(dst_iter, add_width);
        auto end_row_iter = std::next(dst_iter, other.GetWidth());
        for (; dst_iter != end_row_iter; ++dst_iter) {
            *dst_iter = *other_iter++;
        }
        std::advance(dst_iter, add_width);
    }
    // left field
    dst_iter = std::next(dst.begin(), dst.GetWidth() * add_height);
    other_iter = other.begin();
    for (int _ = 0; _ != other.GetHeight(); ++_) {
        std::advance(other_iter, add_width);
        auto end_row_iter = std::next(dst_iter, add_width);
        for (; dst_iter != end_row_iter; ++dst_iter) {
            *dst_iter = *other_iter--;
        }
        std::advance(other_iter, other.GetWidth());
        std::advance(dst_iter, dst.GetWidth() - add_width);
    }
    // right field
    dst_iter = std::next(dst.begin(), dst.GetWidth() * add_height);
    other_iter = std::next(other.begin(), other.GetWidth() - 2);
    for (int _ = 0; _ != other.GetHeight(); ++_) {
        std::advance(dst_iter, other.GetWidth() + add_width);
        auto end_row_iter = std::next(dst_iter, add_width);
        for (; dst_iter != end_row_iter; ++dst_iter) {
            *dst_iter = *other_iter--;
        }
        std::advance(other_iter, add_width + other.GetWidth());
    }
    // upper field
    dst_iter = dst.begin();
    float* line_begin = std::next(dst.begin(), dst.GetWidth() * add_height * 2);
    float* line_end = std::next(line_begin, dst.GetWidth());
    for (int _ = 0; _ != add_height; ++_) {
        std::copy(line_begin, line_end, dst_iter);
        std::advance(line_begin, -dst.GetWidth());
        std::advance(line_end, -dst.GetWidth());
        std::advance(dst_iter, dst.GetWidth());
    }
    // lower field
    dst_iter = std::next(dst.begin(), (add_height + other.GetHeight()) * dst.GetWidth());
    line_begin = std::next(dst_iter, -2 * dst.GetWidth());
    line_end = std::next(line_begin, dst.GetWidth());
    for (int _ = 0; _ != add_height; ++_) {
        std::copy(line_begin, line_end, dst_iter);
        std::advance(line_begin, -dst.GetWidth());
        std::advance(line_end, -dst.GetWidth());
        std::advance(dst_iter, dst.GetWidth());
    }
    return dst;
}

Channel<float> GetAdaptMatrix(const Channel<float>& white) {
    /*   ================ Original iCAM06 formulas
      // La - The absolute luminance of the adapting field, If unknown, the adapting
      // field can be assumed to have average reflectance ("gray world" assumption): LA = LW / 5.
      Channel La = 0.2 * white;
      // LW is the absolute luminance of the reference white in cd/m2
      Channel k_in4 = 1 / (5.0f * La + 1.0f);
      k_in4.Powf(4.0f);
      // in FL first term is very very small. FL is the luminance level adaptation factor.
      Channel FL = 0.2 * k_in4 * (5 * La) + 0.1 * Powf((1 - k_in4), 2.0f) * Powf(5.0f * La, 1/3.0f);
      ================================= */
    // ====================Reduced formulas
    // k_in4
    Channel<float> result(white.GetWidth(), white.GetHeight());
    result = Pow4(1.0f / (white + 1.0f));
    // FL
    result = 0.2f * result * white + 0.1f * Square(1 - result) * Cbrt(white);
    return result;
}

Image<float> IPTAdapt(const Image<float>& XYZ, float max_L) {
    if (XYZ.GetColorSpace() != ColorSpace::XYZ) {
        throw std::runtime_error("Only for XYZ images");
    }
    auto max_Y = MinMaxValues(XYZ)[3];
    Image<float> result(XYZ.GetColorSpace(), XYZ.GetWidth(), XYZ.GetHeight(),
                        XYZ.GetNumOfChannels());
    result = XYZ * (max_L / max_Y);    // to normalized luminance again
    Channel<float> FL = CopyChannel(result, 1);
    FL = GetAdaptMatrix(FL);
    result.ChangeColorSpace(ColorSpace::LMS);
    result = Pow(Abs(result), 0.43f);    // gamma in Icam06 = 0.43
    result.ChangeColorSpace(ColorSpace::IPT);
    auto FL_iter = FL.begin();
    for (auto iter = result.begin(); iter != result.end(); std::advance(iter, 3)) {
        auto M_iter = std::next(iter, 1);
        auto S_iter = std::next(iter, 2);
        float c_val = std::sqrt(*M_iter * *M_iter + *S_iter * *S_iter);
        c_val = (1.29f * c_val * c_val - 0.27f * c_val + 0.42f) /
                (c_val * c_val - 0.31f * c_val + 0.42f);
        float FL_val = std::pow(*FL_iter + 1, 0.15f);
        *M_iter *= FL_val * c_val;
        *S_iter *= FL_val * c_val;
        FL_iter++;
    }
    /*       # Bartleson surround adjustment
        img_cor[:,:,0] = img_cor[:,:,0] * max_i           // gamma = 1  in ICam06HDR
        max_i = np.amax(img_cor[:,:,0])
        img_cor[:,:,0] /= max_i
        img_cor[:,:,0] = np.power(img_cor[:,:,0], gamma) */
    result.ChangeColorSpace(ColorSpace::LMS);
    result = Pow(Abs(result), 1.0f / 0.43f);    // gamma in Icam06 = 1/0.43
    result.ChangeColorSpace(ColorSpace::XYZ);
    max_Y = MinMaxValues(result)[3];
    result /= max_Y;
    return result;
}

Channel<float> RgbToBWCorrectedLab(Image<float>& img_rgb) {
    if (img_rgb.GetColorSpace() != ColorSpace::RGB) {
        throw std::runtime_error("Only for linear RGB images");
    }
    img_rgb.ChangeColorSpace(ColorSpace::XYZ);
    img_rgb = LocLightAdapt(img_rgb);
    img_rgb = IPTAdapt(img_rgb);
    img_rgb.ChangeColorSpace(ColorSpace::Lab);
    Channel<float> lightness = CopyChannel(img_rgb, 0);
    const auto [lower_bound, upper_bound] = lightness.Percentile(.2f / 256, 255.8f / 256);
    lightness.Rescale(lower_bound, upper_bound, 0.0f, 100.0f);
    LoadFromChannel(img_rgb, lightness, 0);
    return lightness;
}

Image<float> GetEqualizedXYZFromLab(const Image<float>& src_Lab, Channel<float>& lightness) {
    if (src_Lab.GetColorSpace() != ColorSpace::Lab) {
        throw std::runtime_error("Only for Lab images");
    }
    Image<float> result = src_Lab;
    lightness.Equalize(0.0f, 100.0f);
    LoadFromChannel(result, lightness, 0);
    result.ChangeColorSpace(ColorSpace::XYZ);
    return result;
}

}    // namespace pg::ops
