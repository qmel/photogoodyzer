#define CATCH_CONFIG_MAIN

#include "tests.h"

#include <catch.hpp>

#include "PhotoGoodyzer.h"

using namespace pg;

TEST_CASE("Empty", "[Image][Channel]") {
    Image<float> img;
    REQUIRE(img.empty());
    CHECK_FALSE(img);
    REQUIRE(!img);
    Channel<unsigned char> chan(600, 400);
    CHECK_FALSE(std::empty(chan));
    REQUIRE(chan);
    CHECK_FALSE(!chan);
}

TEST_CASE(
    "Dimensions"
    "[Image][Channel]") {
    Image<unsigned char> img(ColorSpace::XYZ, 600, 400, 3);
    REQUIRE((img.GetWidth() == 600 && img.GetHeight() == 400 && img.GetNumOfChannels() == 3 &&
             img.GetImgSize() == 240000 && img.size() == 720000));
    Image<float> img2;
    REQUIRE((img2.GetWidth() == 0 && img2.GetHeight() == 0 && img2.GetNumOfChannels() == 0 &&
             img2.GetImgSize() == 0 && img2.size() == 0));
    Channel<float> chan(600, 400);
    REQUIRE((chan.GetWidth() == 600 && chan.GetHeight() == 400 && chan.GetNumOfChannels() == 1 &&
             chan.GetImgSize() == 240000 && chan.size() == 240000));
    Image<float> chan2;
    REQUIRE((chan2.GetWidth() == 0 && chan2.GetHeight() == 0 && chan2.GetNumOfChannels() == 0 &&
             chan2.GetImgSize() == 0 && chan2.size() == 0));
}

TEST_CASE(
    "Simple in-place arithmetics"
    "[Image][Channel]") {
    int src_val = GENERATE(range(-100, 100));
    Image<int> img(ColorSpace::XYZ, 100, 50, 3);
    img.Fill(src_val);
    Channel<int> chan(100, 50);
    chan.Fill(src_val);
    SECTION("Image<int> result +=, -=, *=, /=") {
        RequireCalcInPlace(img, src_val);
    }
    SECTION("Channel<int> result +=, -=, *=, /=") {
        RequireCalcInPlace(img, src_val);
    }
}

TEST_CASE(
    "Expression templates with value"
    "[Expression templates][Image][Channel]") {
    int src_val = GENERATE(range(1, 11));
    Image<int> img(ColorSpace::XYZ, 100, 50, 3);
    img.Fill(src_val);
    Channel<int> chan(100, 50);
    chan.Fill(src_val);
    SECTION("Image<int> result = ImgExpr(+ - * /)") {
        Image<int> result(img.GetColorSpace(), img.GetWidth(), img.GetHeight(),
                          img.GetNumOfChannels());
        RequireExprValueOp(result, img, src_val);
    }
    SECTION("Channel<int> result = ImgExpr(+ - * /)") {
        Channel<int> result(chan.GetWidth(), chan.GetHeight());
        RequireExprValueOp(result, chan, src_val);
    }
    SECTION("Image<int> result = Abs, Square, Pow4, Sqrt, Cbrt, Pow") {
        Image<int> result(img.GetColorSpace(), img.GetWidth(), img.GetHeight(),
                          img.GetNumOfChannels());
        RequireExprValueFunc(result, img, src_val);
    }
    SECTION("Channel<int> result = Abs, Square, Pow4, Sqrt, Cbrt, Pow") {
        Channel<int> result(chan.GetWidth(), chan.GetHeight());
        RequireExprValueFunc(result, chan, src_val);
    }
}

TEST_CASE(
    "Expression templates with array"
    "[Expression templates][Image][Channel]") {
    int val_1 = GENERATE(range(1, 11));
    int val_2 = GENERATE(range(1, 11));
    Image<int> img_1(ColorSpace::XYZ, 100, 50, 3);
    img_1.Fill(val_1);
    Image<int> img_2(ColorSpace::XYZ, 100, 50, 3);
    img_2.Fill(val_2);
    Channel<int> chan_1(100, 50);
    chan_1.Fill(val_1);
    Channel<int> chan_2(100, 50);
    chan_2.Fill(val_2);
    SECTION("Image<int> result = ImgExpr(+ - * / Pow)") {
        Image<int> result(img_1.GetColorSpace(), img_1.GetWidth(), img_1.GetHeight(),
                          img_1.GetNumOfChannels());
        RequireExprArrayOp(result, img_1, img_2, val_1, val_2);
    }
    SECTION("Channel<int> result = ImgExpr(+ - * / Pow)") {
        Channel<int> result(chan_1.GetWidth(), chan_1.GetHeight());
        RequireExprArrayOp(result, chan_1, chan_2, val_1, val_2);
    }
}

TEST_CASE(
    "Expression templates with empty"
    "[Expression templates][Image][Channel]") {
    int src_val = 42;
    Image<int> img;
    Channel<int> chan;
    SECTION("Image<int> result = ImgExpr(+ - * /)") {
        Image<int> result(img.GetColorSpace(), img.GetWidth(), img.GetHeight(),
                          img.GetNumOfChannels());
        RequireExprValueOp(result, img, src_val);
    }
    SECTION("Channel<int> result = ImgExpr(+ - * /)") {
        Channel<int> result(chan.GetWidth(), chan.GetHeight());
        RequireExprValueOp(result, chan, src_val);
    }
    SECTION("Image<int> result = Abs, Square, Pow4, Sqrt, Cbrt, Pow") {
        Image<int> result(img.GetColorSpace(), img.GetWidth(), img.GetHeight(),
                          img.GetNumOfChannels());
        RequireExprValueFunc(result, img, src_val);
    }
    SECTION("Channel<int> result = Abs, Square, Pow4, Sqrt, Cbrt, Pow") {
        Channel<int> result(chan.GetWidth(), chan.GetHeight());
        RequireExprValueFunc(result, chan, src_val);
    }
}