#pragma once

#include <map>
#include <utility>

#include "PhotoGoodyzer/ColorSpace.h"

namespace pg {

struct TransferMatrix {
    float row1[3] = {};
    float row2[3] = {};
    float row3[3] = {};
};

// An array of color transfer matrices. See in TransferMatrix.cpp
extern std::map<std::pair<ColorSpace, ColorSpace>, TransferMatrix> DST_FROM_SRC;

}    // namespace pg
