#TermServer cmake config file for src

#cmake version request
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#project name
PROJECT(star_pushersvr)


SET(CMAKE_CXX_FLAGS "-U__FILE__ -D__FILE__='\"$(subst $(dir $<),,$<)\"'")
SET(APPVERSION "1.0.0.0-")
#外部传递参数----SYSVERSION
IF(SYSVERSION)
	SET(APPVERSION ${SYSVERSION})
ENDIF()
SET(DATE)
EXEC_PROGRAM(date ARGS +"-%y%m%d%H%M" OUTPUT_VARIABLE DATE)
SET(APPVERSION ${APPVERSION}${DATE})
ADD_DEFINITIONS(-DVERSION=${SYSVERSION})
ADD_DEFINITIONS(-DDATE=${DATE})


#Head file path
#ACE目录
INCLUDE_DIRECTORIES($ENV{ACE_ROOT}/include)
INCLUDE_DIRECTORIES($ENV{JSONCPP_ROOT}/include)
INCLUDE_DIRECTORIES($ENV{SPDLOG_ROOT}/include)
INCLUDE_DIRECTORIES($ENV{CURL_ROOT}/include)
INCLUDE_DIRECTORIES($ENV{FFMPEG_ROOT}/include)
INCLUDE_DIRECTORIES($ENV{RTMPPUSHER_ROOT}/include)
INCLUDE_DIRECTORIES($ENV{RTSPPUSHER_ROOT}/include)
INCLUDE_DIRECTORIES(. appConfig common httpClient httpServer service spdlogger stream codec)

#library path
LINK_DIRECTORIES($ENV{ACE_ROOT}/lib)
LINK_DIRECTORIES($ENV{OPENSSL_ROOT}/lib)
LINK_DIRECTORIES($ENV{JSONCPP_ROOT}/lib)
LINK_DIRECTORIES($ENV{SPDLOG_ROOT}/lib)
LINK_DIRECTORIES($ENV{CURL_ROOT}/lib)
LINK_DIRECTORIES($ENV{FFMPEG_ROOT}/lib)
LINK_DIRECTORIES($ENV{LAME_ROOT}/lib)
LINK_DIRECTORIES($ENV{RTMPPUSHER_ROOT}/lib)
LINK_DIRECTORIES($ENV{RTSPPUSHER_ROOT}/lib)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCPU_ARCH_AARCH64")
#SET(CMAKE_C_COMPILER $ENV{ARM_TOOLCHAIN}/bin/aarch64-linux-gnu-gcc)
#SET(CMAKE_CXX_COMPILER $ENV{ARM_TOOLCHAIN}/bin/aarch64-linux-gnu-g++)
#SET(CMAKE_FILD_ROOT_PATH $ENV{ARM_TOOLCHAIN}/aarch64-linux-gnu)

#外部传递参数----DEBUG_mode
IF(DEBUG_mode)
	SET(CMAKE_BUILD_TYPE Debug)
	set(CMAKE_CXX_STANDARD 11)
	SET(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wno-reorder -fpic -D_DEBUG -DENABLE_RTSP -std=c++0x -DCPPHTTPLIB_USE_POLL")
	MESSAGE("START COMPILE WITH Debug")
ELSE(DEBUG_mode)
	SET(CMAKE_BUILD_TYPE Release)
	set(CMAKE_CXX_STANDARD 11)
	SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wno-reorder -fpic -DENABLE_RTSP -std=c++0x -DCPPHTTPLIB_USE_POLL")
	MESSAGE("START COMPILE WITH Release")
ENDIF()

#################################################################
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/appConfig PUSHERSVR_SOURCE) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/common PUSHERSVR_SOURCE) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/httpClient PUSHERSVR_SOURCE) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/httpServer PUSHERSVR_SOURCE) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/service PUSHERSVR_SOURCE) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/spdlogger PUSHERSVR_SOURCE) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/stream PUSHERSVR_SOURCE) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/codec PUSHERSVR_SOURCE) 

ADD_EXECUTABLE(star_pushersvr main.cpp ${PUSHERSVR_SOURCE})
TARGET_LINK_LIBRARIES(star_pushersvr ACE pthread jsoncpp spdlog curl avutil swresample avformat avcodec avfilter swscale streampusher RTSPPusher mp3lame)

#install
#SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
#INSTALL(TARGETS server DESTINATION bin)

