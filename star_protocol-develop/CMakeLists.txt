CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

# 程序名称
PROJECT(starprotocol)

#add_compile_options(-std=c++11)
#add_compile_options(-fvisibility=hidden)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -U__FILE__ -D__FILE__='\"$(subst $(dir $<),,$<)\"'")
SET(CMAKE_BUILD_TYPE Release)


#外部传递参数----DEBUG_mode
IF(DEBUG_mode)
	SET(CMAKE_BUILD_TYPE Debug)
	set(CMAKE_CXX_STANDARD 11)
	SET(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -D_DEBUG -fPIC -std=c++0x -Wno-reorder -Wno-unused-variable -Wno-unused-function")
	MESSAGE("START COMPILE WITH Debug")
ELSE(DEBUG_mode)
	SET(CMAKE_BUILD_TYPE Release)
	set(CMAKE_CXX_STANDARD 11)
	SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -fPIC -std=c++0x -Wno-reorder -Wno-unused-variable -Wno-unused-function")
	MESSAGE("START COMPILE WITH Release")
ENDIF()

# 保存寻呼录音调试开关
IF(RECORD_mode)
	SET(CMAKE_CXX_FLAGS "-D_DEBUG_SAVE_RECORD")
ENDIF()

# 头文件路径(-I)
#INCLUDE_DIRECTORIES($ENV{ACE_ROOT}/include)
INCLUDE_DIRECTORIES(include src)
INCLUDE_DIRECTORIES($ENV{SPDLOG_ROOT}/include)
INCLUDE_DIRECTORIES($ENV{JSONCPP_ROOT}/include)

# 源文件路径
AUX_SOURCE_DIRECTORY(./src DIR_SRCS)
AUX_SOURCE_DIRECTORY(./test DIR_TEST)

# 共有部分

#library path
LINK_DIRECTORIES($ENV{SPDLOG_ROOT}/lib)
LINK_DIRECTORIES($ENV{JSONCPP_ROOT}/lib)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${DIR_SRCS})

# 链接配置(-l)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} spdlog jsoncpp)

# 测试程序
ADD_EXECUTABLE(test ${DIR_TEST})
TARGET_LINK_LIBRARIES(test starprotocol)
