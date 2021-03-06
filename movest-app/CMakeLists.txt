cmake_minimum_required(VERSION 3.3)
project(movest)
include(ExternalProject)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

add_executable(movest_enc src/encoder.c)
add_executable(movest_dec src/decoder.c)

set(EP_INSTALL ${CMAKE_BINARY_DIR}/install CACHE INTERNAL "")

set(MOVESTLIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/movestlib)
set(MOVESTLIBSRC_DIR ${CMAKE_SOURCE_DIR}/src/movestlib)
add_subdirectory(src/movestlib)

set(FFMPEG_DIR ${CMAKE_CURRENT_BINARY_DIR}/libs/ffmpeg)
ExternalProject_Add(ffmpeg_proj DEPENDS movestlib
        BINARY_DIR ${FFMPEG_DIR}
        SOURCE_DIR ${FFMPEG_DIR}
        URL libs/ffmpeg
        CONFIGURE_COMMAND
            # Install step is very unreliable otherwise
            mkdir -p ${FFMPEG_DIR}/include/movest &&
            mkdir -p ${FFMPEG_DIR}/lib &&
            cp ${MOVESTLIBSRC_DIR}/movest_connector.h ${FFMPEG_DIR}/include/movest &&
            cp ${MOVESTLIB_DIR}/libmovestlib.so ${FFMPEG_DIR}/lib &&
            env LD_PRELOAD=${MOVESTLIB_DIR}/libmovestlib.so <SOURCE_DIR>/configure
            --prefix=<BINARY_DIR>
            --extra-cflags=-I<BINARY_DIR>/include
            --extra-ldflags=-L<BINARY_DIR>/lib
            --extra-libs=-lmovestlib\ -lpthread
            --enable-gpl
            --enable-nonfree
            --enable-libfdk-aac
            --disable-doc
            --disable-pthreads
            --disable-programs
            --enable-shared
            --enable-static
            --enable-runtime-cpudetect
        BUILD_COMMAND make -j3
        INSTALL_COMMAND make install
)

#add_library(librscode SHARED IMPORTED)
#set_property(TARGET librscode PROPERTY IMPORTED_LOCATION ${EP_INSTALL}/lib/librscode.so)
#add_dependencies(movest_enc librscode)
#add_dependencies(movest_dec librscode)

#set(FFMS2_DIR ${CMAKE_CURRENT_BINARY_DIR}/libs/ffms2)
#ExternalProject_Add(
#        ffms2_proj
#        BINARY_DIR ${FFMS2_DIR}
#        SOURCE_DIR ${FFMS2_DIR}
#        URL libs/ffms2
#        CONFIGURE_COMMAND ./configure --prefix=<BINARY_DIR> PKG_CONFIG_PATH=<BINARY_DIR>/lib/pkgconfig AVRESAMPLE_LIBS=-lswresample
#        BUILD_COMMAND make
#        INSTALL_COMMAND make install
#)

#set(X264_DIR ${CMAKE_CURRENT_BINARY_DIR}/libs/x264)
#
#ExternalProject_Add(
#        libx264_proj
#        BINARY_DIR ${X264_DIR}
#        SOURCE_DIR ${X264_DIR}
#        URL libs/x264
#        CONFIGURE_COMMAND env PATH="<BINARY_DIR>/bin:$ENV{PATH}" ./configure
#        --prefix=${FFMPEG_DIR}
#        --extra-ldflags=-ljpeg\ -lpng\ -lm
#        --enable-shared
#        --enable-static
#        BUILD_COMMAND env PATH="<BINARY_DIR>/bin:$ENV{PATH}" make -j3
#        INSTALL_COMMAND make install
#)

add_library(libavcodec SHARED IMPORTED)
add_library(libavformat SHARED IMPORTED)
add_library(libavfilter SHARED IMPORTED)
add_library(libavutil SHARED IMPORTED)
add_library(libmovestlib SHARED IMPORTED)
#add_library(libecc STATIC IMPORTED)
#add_library(libx264 SHARED IMPORTED)

ExternalProject_Get_Property(ffmpeg_proj BINARY_DIR)
set_property(TARGET libavcodec PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/lib/libavcodec.so)
set_property(TARGET libavformat PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/lib/libavformat.so)
set_property(TARGET libavfilter PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/lib/libavfilter.so)
set_property(TARGET libavutil PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/lib/libavutil.so)
set_property(TARGET libmovestlib PROPERTY IMPORTED_LOCATION ${MOVESTLIB_DIR}/libmovestlib.so)
#set_property(TARGET libecc PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/libs/rscode/libecc.a)
#ExternalProject_Get_Property(libx264_proj BINARY_DIR)
#set_property(TARGET libx264 PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/lib/libx264.so)

add_dependencies(libavcodec ffmpeg_proj)
add_dependencies(libavformat ffmpeg_proj)
add_dependencies(libavfilter ffmpeg_proj)
add_dependencies(libavutil ffmpeg_proj)
#add_dependencies(ffmpeg_proj libx264)

#add_dependencies(ffms2_proj ffmpeg_proj)
#add_dependencies(libx264_proj ffms2_proj)
#add_dependencies(libx264 libx264_proj)

add_dependencies(movest_enc libavcodec)
add_dependencies(movest_enc libavformat)
add_dependencies(movest_enc libavfilter)
add_dependencies(movest_enc libavutil)
add_dependencies(movest_enc libmovestlib)
#add_dependencies(movest_enc libecc)
add_dependencies(movest_dec libavcodec)
add_dependencies(movest_dec libavformat)
add_dependencies(movest_dec libavfilter)
add_dependencies(movest_dec libavutil)
add_dependencies(movest_dec libmovestlib)
#add_dependencies(movest_dec libecc)
#add_dependencies(movest libx264)

ExternalProject_Get_Property(ffmpeg_proj SOURCE_DIR)
include_directories(movest_enc ${SOURCE_DIR})
include_directories(movest_dec ${SOURCE_DIR})
#ExternalProject_Get_Property(libx264_proj SOURCE_DIR)
#include_directories(movest ${SOURCE_DIR})


target_link_libraries(movest_enc libavcodec)
target_link_libraries(movest_enc libavformat)
target_link_libraries(movest_enc libavfilter)
target_link_libraries(movest_enc libavutil)
target_link_libraries(movest_enc libmovestlib)

target_link_libraries(movest_dec libavcodec)
target_link_libraries(movest_dec libavformat)
target_link_libraries(movest_dec libavfilter)
target_link_libraries(movest_dec libavutil)
target_link_libraries(movest_dec libmovestlib)

#target_link_libraries(movest_enc librscode)
#target_link_libraries(movest_dec librscode)

#target_link_libraries(movest libx264)
#target_link_libraries(movest m)
#target_link_libraries(movest gpac)

#add_subdirectory(libs/ffmpeg)