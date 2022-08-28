set(SOURCE_FILES
    ArrayBase.cpp
    Image.cpp
    FFT.cpp
    TransferMatrix.cpp
    ops.cpp
    sRGBvLinRGB.cpp
)

find_package(FFTW3 REQUIRED COMPONENTS fftw3f)

add_library(pglib ${SOURCE_FILES})
target_include_directories(pglib PUBLIC ${FFTW3_INCLUDE_DIRS})
target_link_libraries(pglib PUBLIC ${FFTW3_LIBRARIES})

# Set up the public include directory which is to be used by the library users
get_filename_component(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include" REALPATH)
get_filename_component(LIB_DIR "${PROJECT_SOURCE_DIR}/lib" REALPATH)
target_include_directories(pglib PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<BUILD_INTERFACE:${LIB_DIR}>
    $<INSTALL_INTERFACE:include>)

# Install the library and its public header files
install(TARGETS pglib EXPORT PhotoGoodyzerTargets)
install(FILES "${INCLUDE_DIR}/PhotoGoodyzer.h" TYPE INCLUDE)
install(DIRECTORY "${INCLUDE_DIR}/PhotoGoodyzer" TYPE INCLUDE)
