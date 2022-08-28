#pragma once

#include <catch.hpp>
#include <cmath>

#include "PhotoGoodyzer/Array.h"

template <typename T>
void RequireCalcInPlace(pg::Array<T>& img, T val) {
    img += 13;      val += 13;
    img *= 3;       val *= 3;
    img /= 2;       val /= 2;
    img -= 3;       val -= 3;
    img.Pow(4);     val = std::pow(val, 4);
    img -= 200'000; val -= 200'000;
    img.Abs();      val = std::abs(val);
    for (auto pix : img)
        REQUIRE(pix == val);
}

template<class T>
void ExprValueOp(T& result, const T& src) {
    result = -(+((src + 2) / 3 * 5 - 16));
}

template<class T>
void ExprArrayOp(T& result, const T& lhs, const T& rhs) {
    result = -(+((lhs + rhs) / rhs * rhs - rhs));
}

template <typename T>
void RequireExprValueOp(pg::Array<T>& result,
                        const pg::Array<T>& src, T src_val) {
    ExprValueOp(result, src);
    T cor_ans;
    ExprValueOp(cor_ans, src_val);
    for (auto pix : result)
        REQUIRE(pix == cor_ans);
}

template <typename T>
void RequireExprArrayOp(pg::Array<T>& result,
                        const pg::Array<T>& img_lhs,
                        const pg::Array<T>& img_rhs,
                        T val_lhs, T val_rhs) {
    ExprArrayOp(result, img_lhs, img_rhs);
    result = Pow(result, img_rhs / 3);
    T cor_ans;
    ExprArrayOp(cor_ans, val_lhs, val_rhs);
    cor_ans = std::pow(cor_ans, val_rhs / 3);
    for (auto pix : result)
        REQUIRE(pix == cor_ans);
}

template<typename T>
T ExprValueFuncOnType(T other) {
    return std::pow(std::cbrt(std::pow(std::sqrt(std::abs(-(other * other)) + 24), 4)  - 14), 2);
}

template<class T>
void ExprValueFuncOnImgExpr(T& result, const T& src) {
    result = Pow(Cbrt(Pow4(Sqrt(Abs(-Square(src)) + 24)) - 14), 2);
}

template <typename T>
void RequireExprValueFunc(pg::Array<T>& result,
                        const pg::Array<T>& src, T src_val) {
    // There may be some rounding errors in ths test
    ExprValueFuncOnImgExpr(result, src);
    T cor_ans = ExprValueFuncOnType(src_val);
    for (auto pix : result)
        REQUIRE(pix == cor_ans);
}
