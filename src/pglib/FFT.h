#pragma once

#include <memory>

#include "PhotoGoodyzer/Array.h"

namespace pg {

class FFTImpl;

class FFTr2c {
private:
    std::unique_ptr<FFTImpl> impl;

public:
    FFTr2c() = delete;

    // Copying ctor and with OUT data out-of-place;
    FFTr2c(const Array<float>& other);

    // Non-Copying non-destructive ctor, with OUT data out-of-place
    FFTr2c(float* in, int width, int height);

    void ForwardTransform();
    void InverseTransform();
    void NormalizeIn();
    void RemoveOutZeroFreq();
    void ReduceImagine();

    void LoadTo(Array<float>& other);
    void MultiplyOutByRealOut(const FFTr2c& other);
    void ClipNegativeOutRealToZero();
    void ClipNegativeInToZero();

    ~FFTr2c();
};

}    // namespace pg