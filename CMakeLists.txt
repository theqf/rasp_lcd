cmake_minimum_required(VERSION 3.10)
project(rasp_lcd)

set(CMAKE_CXX_STANDARD 11)
include_directories(
        "/usr/local/include/"
)
set(FFMPEG_LINUX_LIBS
        "avformat"
        "avdevice"
        "avfilter"
        "avcodec"
        "avutil"
        "swresample"
        "swscale"
        )

file(GLOB C_FILE "*.c")
file(GLOB CPP_FILE "*.cpp")

link_libraries("/usr/local/lib/libjpeg.a")
add_executable(rasp_lcd ${CPP_FILE} ${C_FILE})

target_link_libraries(rasp_lcd wiringPi ${FFMPEG_LINUX_LIBS} pthread)