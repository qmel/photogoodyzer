#pragma once

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace pg {

class ArrayBase;

class ExprBase {};

template <class T>
struct IsVector {
    static constexpr bool value = false;
};

template <class T>
struct IsVector<std::vector<T>> {
    static constexpr bool value = true;
};

template <class T>
struct RemoveCVRef {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template <class T>
using RemoveCVRef_t = typename RemoveCVRef<T>::type;

template <class T>
constexpr bool IsVectorV = IsVector<RemoveCVRef_t<T>>::value;

template <class T>
constexpr bool has_size_and_idx = IsVectorV<T> || std::is_base_of_v<ExprBase, RemoveCVRef_t<T>> ||
                                  std::is_base_of_v<ArrayBase, RemoveCVRef_t<T>>;

template <class A, class B>
constexpr bool is_binary_op_ok = has_size_and_idx<A> || has_size_and_idx<B>;

template <class Operand>
auto Subscript(const Operand& v, size_t i) {
    if constexpr (has_size_and_idx<Operand>) {
        return v[i];
    } else {
        return v;
    }
}

template <class Callable, class... Operands>
class ImgExpr : public ExprBase {
private:
    size_t size_;
    Callable func_;
    std::tuple<const Operands&...> args_;

public:
    ImgExpr() = delete;
    ImgExpr(size_t size, Callable func, const Operands&... args) :
        size_(size), func_(func), args_(args...) {}

    size_t size() const { return size_; }

    ImgExpr(const ImgExpr& other) = default;
    ImgExpr(ImgExpr&& other) = default;
    ImgExpr& operator=(const ImgExpr& other) = default;
    ImgExpr& operator=(ImgExpr&& other) = default;

    auto operator[](size_t idx) const {
        auto const call_at_index = [this, idx](const Operands&... args) {
            return func_(Subscript(args, idx)...);
        };
        return std::apply(call_at_index, args_);
    }
};

template <class Rhs, class = std::enable_if_t<has_size_and_idx<Rhs>>>
auto operator+(const Rhs& rhs) {
    auto lambda = [](const auto& r) { return r; };
    return ImgExpr<decltype(lambda), Rhs>{rhs.size(), lambda, rhs};
}

template <class Rhs, class = std::enable_if_t<has_size_and_idx<Rhs>>>
auto operator-(const Rhs& rhs) {
    auto lambda = [](const auto& r) { return -r; };
    return ImgExpr<decltype(lambda), Rhs>{rhs.size(), lambda, rhs};
}

template <class Rhs, class = std::enable_if_t<has_size_and_idx<Rhs>>>
auto Abs(const Rhs& rhs) {
    auto lambda = [](const auto& r) { return std::abs(r); };
    return ImgExpr<decltype(lambda), Rhs>{rhs.size(), lambda, rhs};
}

template <class Rhs, class = std::enable_if_t<has_size_and_idx<Rhs>>>
auto Square(const Rhs& rhs) {
    auto lambda = [](const auto& r) { return r * r; };
    return ImgExpr<decltype(lambda), Rhs>{rhs.size(), lambda, rhs};
}

template <class Rhs, class = std::enable_if_t<has_size_and_idx<Rhs>>>
auto Pow4(const Rhs& rhs) {
    auto lambda = [](const auto& r) { return r * r * r * r; };
    return ImgExpr<decltype(lambda), Rhs>{rhs.size(), lambda, rhs};
}

template <class Rhs, class = std::enable_if_t<has_size_and_idx<Rhs>>>
auto Pow3(const Rhs& rhs) {
    auto lambda = [](const auto& r) { return r * r * r; };
    return ImgExpr<decltype(lambda), Rhs>{rhs.size(), lambda, rhs};
}

template <class Rhs, class = std::enable_if_t<has_size_and_idx<Rhs>>>
auto Sqrt(const Rhs& rhs) {
    auto lambda = [](const auto& r) { return std::sqrt(r); };
    return ImgExpr<decltype(lambda), Rhs>{rhs.size(), lambda, rhs};
}

template <class Rhs, class = std::enable_if_t<has_size_and_idx<Rhs>>>
auto Cbrt(const Rhs& rhs) {
    auto lambda = [](const auto& r) { return std::cbrt(r); };
    return ImgExpr<decltype(lambda), Rhs>{rhs.size(), lambda, rhs};
}

template <class Lhs, class Rhs>
size_t CheckSize(const Lhs& lhs, const Rhs& rhs) {
    size_t size = 0;
    if constexpr (has_size_and_idx<Lhs>) {
        size = lhs.size();
        if constexpr (has_size_and_idx<Rhs>) {
            if (lhs.size() != rhs.size())
                throw std::runtime_error(
                    "Sizes of left and right side in an expression must be equal");
        }
    } else {
        size = rhs.size();
    }
    return size;
}

template <class Lhs, class Rhs, class = std::enable_if_t<is_binary_op_ok<Lhs, Rhs>>>
auto operator+(const Lhs& lhs, const Rhs& rhs) {
    size_t size = CheckSize(lhs, rhs);
    auto lambda = [](const auto& l, const auto& r) { return l + r; };
    return ImgExpr<decltype(lambda), Lhs, Rhs>{size, lambda, lhs, rhs};
}

template <class Lhs, class Rhs, class = std::enable_if_t<is_binary_op_ok<Lhs, Rhs>>>
auto operator-(const Lhs& lhs, const Rhs& rhs) {
    size_t size = CheckSize(lhs, rhs);
    auto lambda = [](const auto& l, const auto& r) { return l - r; };
    return ImgExpr<decltype(lambda), Lhs, Rhs>{size, lambda, lhs, rhs};
}

template <class Lhs, class Rhs, class = std::enable_if_t<is_binary_op_ok<Lhs, Rhs>>>
auto operator*(const Lhs& lhs, const Rhs& rhs) {
    size_t size = CheckSize(lhs, rhs);
    auto lambda = [](const auto& l, const auto& r) { return l * r; };
    return ImgExpr<decltype(lambda), Lhs, Rhs>{size, lambda, lhs, rhs};
}

template <class Lhs, class Rhs, class = std::enable_if_t<is_binary_op_ok<Lhs, Rhs>>>
auto operator/(const Lhs& lhs, const Rhs& rhs) {
    size_t size = CheckSize(lhs, rhs);
    auto lambda = [](const auto& l, const auto& r) { return l / r; };
    return ImgExpr<decltype(lambda), Lhs, Rhs>{size, lambda, lhs, rhs};
}

template <class Lhs, class Rhs, class = std::enable_if_t<is_binary_op_ok<Lhs, Rhs>>>
auto Pow(const Lhs& lhs, const Rhs& rhs) {
    size_t size = CheckSize(lhs, rhs);
    auto lambda = [](const auto& l, const auto& r) { return std::pow(l, r); };
    return ImgExpr<decltype(lambda), Lhs, Rhs>{size, lambda, lhs, rhs};
}

}    // namespace pg
