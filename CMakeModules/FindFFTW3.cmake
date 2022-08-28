find_path(FFTW3_INCLUDE_DIRS fftw3.h)

if (FFTW3_FIND_COMPONENTS)
    set(FFTW3_LIBRARIES)
    foreach (COMPONENT ${FFTW3_FIND_COMPONENTS})
        find_library(FFTW3_${COMPONENT}_LIBRARY ${COMPONENT})
        if (FFTW3_${COMPONENT}_LIBRARY)
            set(FFTW3_${COMPONENT}_FOUND TRUE)
            set(FFTW3_LIBRARIES ${FFTW3_LIBRARIES}
                    ${FFTW3_${COMPONENT}_LIBRARY})
        endif (FFTW3_${COMPONENT}_LIBRARY)
    endforeach ()
else (FFTW3_FIND_COMPONENTS)
    find_library(FFTW3_LIBRARIES fftw3)
endif (FFTW3_FIND_COMPONENTS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFTW3 HANDLE_COMPONENTS
        REQUIRED_VARS FFTW3_INCLUDE_DIRS FFTW3_LIBRARIES)