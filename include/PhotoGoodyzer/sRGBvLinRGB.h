#pragma once

#include <cmath>
#include <limits>
#include <vector>

namespace pg {

/// A table of precalculated values for unsigned char RGB -> linear RGB conversion
extern std::vector<float> UCHAR_TO_RGB;

/// Convert an unsigned char sRGB value [0... 255] to a float linear RGB [0... 1] value using the
/// table of precalculated values
inline float sRGB_to_linRGB(unsigned char value) {
    return UCHAR_TO_RGB[int(value)];
}

/// Convert a float linear RGB [0... 1] value to an unsigned char sRGB [0... 255] value using
/// formula.
inline unsigned char linRGB_to_sRGB(float value) {
    if (value > 0.0031308f) {
        return (unsigned char)(255.0f * (1.055f * std::pow(value, (1.0f / 2.4f)) - 0.055f) + 0.5f);
    } else {
        return (unsigned char)(12.92f * value * 255.0f + 0.5f);
    }
}

///// Convert type2 sRGB value to type1 linear RGB [0... 1] value using formula.
// template <typename T1, typename T2>
// inline T1 sRGB_to_linRGB(T2 value) {
//     T1 result = value / std::numeric_limits<T2>::max();
//     result > 0.04045f ? result = std::pow((result + 0.055f) / 1.055f, 2.4f) : result /= 12.92f;
//     return result;
// }

}    // namespace pg
