#include <filesystem>
#include <iostream>

#include "PhotoGoodyzer.h"

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STBIW_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define WIDE_MAIN int wmain(int argc, wchar_t* argv[])
#define USUAL_MAIN int main(int argc, char* argv[])

using namespace pg;
using namespace pg::ops;

void Write(const Image<unsigned char>& img, const char* output_filename_ptr) {
    int is_ok = 0;
    is_ok = stbi_write_bmp(output_filename_ptr, img.GetWidth(), img.GetHeight(),
                           img.GetNumOfChannels(), img.begin());
    if (is_ok == 0) {
        throw std::runtime_error("File was not written\n");
    }
}

Image<unsigned char> ReadFromFile(const char* filename_ptr) {
    int width = 0, height = 0, num_of_channels = 0;
    auto ptr = stbi_load(filename_ptr, &width, &height, &num_of_channels, 0);
    Image<unsigned char> img(ColorSpace::sRGB, std::move(ptr), width, height, num_of_channels,
                             stbi_image_free);
    if (img.empty()) {
        throw std::runtime_error("File was not read");
    } else if (num_of_channels != 3) {
        throw std::runtime_error("Files other than 3x8-bit RGB are not supported yet");
    }
    return img;
}

#ifdef IS_WINDOWS    // comes from Cmake
    #include <windows.h>
    WIDE_MAIN
#else
    USUAL_MAIN
#endif    // IS_WINDOWS
{
#ifdef IS_WINDOWS
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, ".utf8");
#endif
    std::vector<std::filesystem::path> src_filepaths;
    std::filesystem::path out_dir = argv[0];
    out_dir = out_dir.parent_path();
    if (argc == 1) {
        std::cerr << "Usage: pgcli [image1.jpg image2.jpg ..] destination_directory(optional)"
                  << std::endl;
        return -1;
    } else if (argc == 2) {
        src_filepaths.push_back(argv[1]);
    } else {
        for (int i = 1; i != argc - 1; ++i) {
            src_filepaths.push_back(argv[i]);
        }
        std::filesystem::path last_path = argv[argc - 1];
        if (std::filesystem::exists(last_path) && !(std::filesystem::is_directory(last_path))) {
            src_filepaths.push_back(last_path);
        } else {
            out_dir = std::move(last_path);
            std::filesystem::create_directories(out_dir);
        }
    }
    for (const auto& src_filepath : src_filepaths) {
        std::cout << "Processing: " << src_filepath << std::endl;
        std::filesystem::path out_file_no_extension = out_dir / src_filepath.stem();
        Image<float> img_float = LinRGBFromSRGB(ReadFromFile(src_filepath.string().c_str()));
        Image<float> eq;
        {
            Channel<float> lightness = RgbToBWCorrectedLab(img_float);
            {    // May be parralel
                Image<float> bw_ct = CorrectColorTemperature(img_float);
                bw_ct.ChangeColorSpace(ColorSpace::XYZ);
                bw_ct.ChangeColorSpace(ColorSpace::RGB);
                Write(SRGBFromLinRGB(bw_ct),
                      (out_file_no_extension.string() + "_BWcorr.bmp").c_str());
            }
            eq = GetEqualizedXYZFromLab(img_float, lightness);
        }
        eq = IPTAdapt(eq, 1.0f);
        {    // May be Parallel
            img_float.ChangeColorSpace(ColorSpace::XYZ);
            img_float.ChangeColorSpace(ColorSpace::RGB);
            Write(SRGBFromLinRGB(img_float),
                  (out_file_no_extension.string() + "_BWcorr_CTcorr.bmp").c_str());
        }
        Write(SRGBFromLinRGB(Image<float>(eq, ColorSpace::RGB)),
              (out_file_no_extension.string() + "_HistEQ.bmp").c_str());
        eq.ChangeColorSpace(ColorSpace::Lab);
        eq = CorrectColorTemperature(eq);
        eq.ChangeColorSpace(ColorSpace::XYZ);
        eq.ChangeColorSpace(ColorSpace::RGB);
        Write(SRGBFromLinRGB(eq), (out_file_no_extension.string() + "_HistEQ_CTcorr.bmp").c_str());
    }

    // std::cin.get();
    return 0;
}
