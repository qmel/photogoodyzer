#pragma once

namespace pg {

struct Illuminant {
    float x;
    float y;
    float z;
};

inline const Illuminant D65{0.950489f, 1.0f, 1.088840f};
inline const Illuminant D50{0.964212f, 1.0f, 0.825188f};

}    // namespace pg