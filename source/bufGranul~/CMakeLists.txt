
if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES x86_64;arm64)
endif ()

include(${C74_SDK}/max-sdk-base/script/max-pretarget.cmake)

#############################################################
# MAX EXTERNAL
#############################################################

include_directories(
    "${MAX_SDK_INCLUDES}"
    "${MAX_SDK_MSP_INCLUDES}"
    "${MAX_SDK_JIT_INCLUDES}"
    "${PROJECT_SRC}../common"
)

file(GLOB PROJECT_SRC
     "*.h"
     "*.c"
     "*.cpp"
     "../common/*.h"
     "../common/*.c"
)

add_library(
    ${PROJECT_NAME}
    MODULE
    ${PROJECT_SRC}
)

include(${C74_SDK}/max-sdk-base/script/max-posttarget.cmake)
