
#include "FFT.h"

#include <fftw3.h>

#include <algorithm>
#include <stdexcept>

#include "PhotoGoodyzer/Array.h"

namespace pg {

class FFTImpl {
public:
    std::unique_ptr<float[], void (*)(float*)> in_;
    fftwf_complex* out_;
    const fftwf_plan plan_fwd;
    const fftwf_plan plan_bwd;
    const int width_;
    const int height_;
    const int size_;

    // Copying ctor and with OUT data out-of-place;
    FFTImpl(const Array<float>& other) :
        in_(fftwf_alloc_real(other.GetImgSize()), [](float* p) { fftwf_free(p); }),
        out_(fftwf_alloc_complex(other.GetHeight() * (other.GetWidth() / 2 + 1))),
        plan_fwd(fftwf_plan_dft_r2c_2d(other.GetHeight(), other.GetWidth(), InBegin(), OutBegin(),
                                       FFTW_ESTIMATE)),
        plan_bwd(fftwf_plan_dft_c2r_2d(other.GetHeight(), other.GetWidth(), OutBegin(), InBegin(),
                                       FFTW_ESTIMATE)),
        width_(other.GetWidth()),
        height_(other.GetHeight()),
        size_(other.GetImgSize()) {
        std::copy(other.begin(), other.end(), InBegin());
    }

    // Non-Copying non-destructive ctor, with OUT data out-of-place
    FFTImpl(float* in, int width, int height) :
        in_(in, [](float*) {}),
        out_(fftwf_alloc_complex(height * (width / 2 + 1))),
        plan_fwd(fftwf_plan_dft_r2c_2d(height, width, InBegin(), OutBegin(), FFTW_ESTIMATE)),
        plan_bwd(fftwf_plan_dft_c2r_2d(height, width, OutBegin(), InBegin(), FFTW_ESTIMATE)),
        width_(width),
        height_(height),
        size_(width * height) {}

    float* InBegin() const { return in_.get(); }
    float* InEnd() const { return std::next(in_.get(), size_); }
    fftwf_complex* OutBegin() const { return out_; }
    fftwf_complex* OutEnd() const { return std::next(out_, height_ * (width_ / 2 + 1)); }

    ~FFTImpl() {
        fftwf_destroy_plan(plan_fwd);
        fftwf_destroy_plan(plan_bwd);
        // fftwf_free(in_);
        fftwf_free(out_);
    }
};

FFTr2c::FFTr2c(const Array<float>& other) : impl(new FFTImpl(other)) {}

FFTr2c::FFTr2c(float* in, int width, int height) : impl(new FFTImpl(in, width, height)) {}

void FFTr2c::ForwardTransform() {
    fftwf_execute(impl->plan_fwd);
}

void FFTr2c::InverseTransform() {
    fftwf_execute(impl->plan_bwd);
    this->NormalizeIn();
}

void FFTr2c::NormalizeIn() {
    auto in_end = impl->InEnd();
    for (auto iter = impl->InBegin(); iter != in_end; ++iter) {
        *iter /= impl->size_;
    }
}

void FFTr2c::RemoveOutZeroFreq() {
    float value = (*impl->OutBegin())[0];
    auto out_end = impl->OutEnd();
    for (auto iter = impl->OutBegin(); iter != out_end; ++iter) {
        (*iter)[0] /= value;
    }
}

void FFTr2c::ReduceImagine() {
    auto out_end = impl->OutEnd();
    for (auto iter = impl->OutBegin(); iter != out_end; ++iter) {
        (*iter)[1] = 0.0f;
    }
}

void FFTr2c::LoadTo(Array<float>& other) {
    if (impl->size_ != other.GetImgSize()) {
        throw std::runtime_error("Sizes do not match");
    }
    std::copy(impl->InBegin(), impl->InEnd(), other.begin());
}

void FFTr2c::MultiplyOutByRealOut(const FFTr2c& other) {
    if (impl->width_ != other.impl->width_ || impl->height_ != other.impl->height_) {
        throw std::runtime_error("Sizes do not match");
    }
    auto other_iter = other.impl->OutBegin();
    auto this_out_end = impl->OutEnd();
    for (auto this_iter = impl->OutBegin(); this_iter != this_out_end; ++this_iter) {
        (*this_iter)[0] *= (*other_iter)[0];
        (*this_iter)[1] *= (*other_iter)[0];
        ++other_iter;
    }
}

void FFTr2c::ClipNegativeOutRealToZero() {
    auto out_end = impl->OutEnd();
    for (auto iter = impl->OutBegin(); iter != out_end; ++iter) {
        if ((*iter)[0] < 0.0f)
            (*iter)[0] = 0.0f;
    }
}

void FFTr2c::ClipNegativeInToZero() {
    auto in_end = impl->InEnd();
    for (auto iter = impl->InBegin(); iter != in_end; ++iter) {
        if (*iter < 0.0f)
            *iter++ = 0.0f;
    }
}

FFTr2c::~FFTr2c() = default;

}    // namespace pg