#pragma once

namespace pg {

/// Possible color space of the image.
enum struct ColorSpace {
    /// <a href="https://en.wikipedia.org/wiki/SRGB">Gamma-compressed Red,
    /// Green, Blue</a>; values must be in [0... 255] range.
    sRGB,

    /// <a href="https://en.wikipedia.org/wiki/CIE_1931_color_space">Linear Red, Green, Blue</a>;
    /// values must be in [0... 1] range.
    RGB,

    /// <a href="https://en.wikipedia.org/wiki/CIE_1931_color_space">CIEXYZ</a>. Values are in [0...
    /// 1] range.
    XYZ,

    /// <a href="https://en.wikipedia.org/wiki/CIELAB_color_space">CIELab</a>. Values are in [0...
    /// 100] range for Lightness and in [-100... 100] range for a and b.
    Lab,

    /// <a href="http://brucelindbloom.com/index.html?Eqn_ChromAdapt.html"
    /// >Von Kries cone response domain (ρ, γ, β) for D65 Illuminant</a>. Values are in [0... 1]
    /// range.
    LMS,

    /// <a href="https://doi.org/10.1016/j.jvcir.2007.06.003">ICam06 IPT color model</a>. Values are
    /// in [0... 1] range.
    IPT
};

}    // namespace pg
