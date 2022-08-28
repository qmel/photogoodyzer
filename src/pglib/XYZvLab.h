#pragma once

#include <cmath>

namespace pg {

// From http://www.easyrgb.com/en/math.php
template <typename T>
inline T Labf_function(T value) {
    if (value > 0.008856f) {
        return std::cbrt(value);
    } else {
        return (7.787f * value) + (16.0f / 116.0f);
    }
}

template <typename T>
inline T LabReversedf_function(T value) {
    T value_cubed = value * value * value;
    if (value_cubed > 0.008856f) {
        return value_cubed;
    } else {
        return (value - 16.0f / 116.0f) / 7.787f;
    }
}

/*   // From https://en.wikipedia.org/wiki/CIELAB_color_space
  static float LAB_SIGMA = 6.0f/29.0f;
  static float LAB_SIGMA_SQUARED = std::pow(6.0f/29.0f, 2.0f);
  static float LAB_SIGMA_CUBED = std::pow(6.0f/29.0f, 3.0f);

  inline float Labf_function (float value) {
    if (value > LAB_SIGMA_CUBED) {
      return cbrt(value);
    } else {
      return (value / 3 / LAB_SIGMA_SQUARED + 4.0f/29.0f);
    }
  }
  inline float LabReversedf_function (float value) {
    if (value > LAB_SIGMA) {
      return std::pow(value, 3.0f);
    } else {
      return (3 * LAB_SIGMA_SQUARED * (value - 4.0f/29.0f));
    }
  }
 */

}    // namespace pg
