# PhotoGoodyzer

### A lightweight image processing library and CLI, GUI applications written in C++

The library provides image tranformations between several color spaces ([sRGB](https://en.wikipedia.org/wiki/SRGB), [linear RGB](https://en.wikipedia.org/wiki/CIE_1931_color_space), [XYZ](https://en.wikipedia.org/wiki/CIE_1931_color_space), [Lab](https://en.wikipedia.org/wiki/CIELAB_color_space), [LMS](https://en.wikipedia.org/wiki/LMS_color_space), [IPT](https://doi.org/10.1016/j.jvcir.2007.06.003)) and image channel manipulations such as histogram equalization, clipping, rescaling, Gaussian blur. The library supports element-wise arithmetic operations and mathematical fuctions with the use of *expression templates*. The library may use already allocated buffers as the source data for classes and operate with std::vectors. The library implements advanced image processing operations based on [iCam06](https://doi.org/10.1016/j.jvcir.2007.06.003) and [CAM16](https://doi.org/10.1002/col.22131) color appearance models, simulating some of human eye algorithms, such as chromatic and local lightness adaptation. The implementation of these operations allows to receive an enhanced, "more natural" view of digital photographs. The library depends only on [FFTW3](https://www.fftw.org).

The CLI application takes in source RGB888bit images and an output directory (optional) and produce four output images with iCam06-, CAM16-based operations implemented for every source image.

The GUI application works in a similar manner, but for a single image, with some parallelization and an ability to mix the output images into one. The GUI application requires [Qt5 Widgets](https://github.com/qt/qtbase). A demo of the GUI application is shown below. More examples as well as executables compiled for Windows and MacOs can be found on [this website](https://qmel.github.io/).

[demo](https://user-images.githubusercontent.com/60773881/187043299-b910970d-a639-4b11-bdff-174be1b9b68d.mov)

## Project structure

The project provides folowing targets:
- `pglib` - the library which implements the logic
- `pgcli` - the CLI application
- `pggui` - the GUI application (optional)
- `pgdoc` - documentation of the project compiled with [doxygen](https://github.com/doxygen/doxygen) (optional)
- `tests` - tests provided with the use of [Catch2](https://github.com/catchorg/Catch2/tree/v2.x) (optional)

Both `pgcli` and `pggui` depend on `pglib`. Also, `pglib` can be used directly by other projects. `pglib` requires [FFTW3](https://www.fftw.org). Currently, the project uses single header [stb_image](https://github.com/nothings/stb) libraries for image IO implementations in `pgcli` (included as single header files in `lib`). `pggui` requires [Qt5 Widgets](https://github.com/qt/qtbase). The project uses [Catch2](https://github.com/catchorg/Catch2/tree/v2.x) for testing (included as a single header file in `lib`).

The directory structure is as follows:
- `include` - public headers provided by pglib
- `lib` - small 3rd-party header-only libraries
- `src` - private source files of all the primary targets
- `tests` - source files of all the tests
- `doc` - generated documentation (optional)

## Building

### Mandatory prerequisites

- A C++17-compliant compiler
- CMake 3.15 or newer
- If applicable, MacOsX Catalina or newer (>= 10.15) 
- If applicable, Windows 7 or newer

### Optional prerequisites

- [Qt5 Widgets](https://github.com/qt/qtbase) for the GUI application
- [doxygen](https://github.com/doxygen/doxygen)
