#  specify the minimum version for CMake
cmake_minimum_required(VERSION 3.5)

#  project name
project(testAllegro)


#  ignore this line, hope it will work one day
#message("allegro libraries = " ${ALLEGRO_LIBRARIES})


#  set to c++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


#  include opengl too
##find_package(OpenGL REQUIRED)
##include_directories( ${OPENGL_INCLUDE_DIRS} )
#find_package(GLUT REQUIRED)
#include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )



#  source code files
message("project source dir =" ${PROJECT_SOURCE_DIR})
#include_directories(${PROJECT_SOURCE_DIR}/src)
file(GLOB ALL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        )
message("source files =" ${ALL_SOURCE_FILES})


#  this must be done before add_executable
#set(CMAKE_INSTALL_RPATH "${$ORIGIN}/dller")

#set(CMAKE_INSTALL_RPATH "$ORIGIN")

#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


#  Abandoned RPATH idea for now
#SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#SET(CMAKE_INSTALL_RPATH "$\{ORIGIN\}/../dlller/")


#  define project output
add_executable(testAllegro ${ALL_SOURCE_FILES})
##target_link_libraries(testAllegro ${OPENGL_LIBRARIES} )


if (WIN32)
    #  windows stuff
    #  compiler location: "C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++"
	#  escape slashes, great
    INCLUDE_DIRECTORIES(              "C:\\Program Files (x86)\\Dev-Cpp\\allegro_5\\allegro\\include" )
	TARGET_LINK_LIBRARIES(testAllegro "C:/Program Files (x86)/Dev-Cpp/allegro_5/allegro/bin/allegro-5.2.dll")
endif()

if (UNIX AND NOT APPLE)
    #  (well, almost always)linux stuff
    INCLUDE_DIRECTORIES(  /usr/include/allegro5 )

    #  try using pkg config instead of manually specifying path
    #find_package(PkgConfig REQUIRED)
    #PKG_SEARCH_MODULE(ALLEGRO5 REQUIRED 
    #pkg_search_module(ALLEGRO5 REQUIRED allegro-5)
    
    TARGET_LINK_LIBRARIES(testAllegro  /usr/lib/x86_64-linux-gnu/liballegro.so)
    TARGET_LINK_LIBRARIES(testAllegro /usr/lib/x86_64-linux-gnu/liballegro_image.so)
    TARGET_LINK_LIBRARIES(testAllegro /usr/lib/x86_64-linux-gnu/liballegro_primitives.so)
    TARGET_LINK_LIBRARIES(testAllegro /usr/lib/x86_64-linux-gnu/liballegro_font.so)
    TARGET_LINK_LIBRARIES(testAllegro /usr/lib/x86_64-linux-gnu/liballegro_ttf.so)
    TARGET_LINK_LIBRARIES(testAllegro /usr/lib/x86_64-linux-gnu/liballegro_audio.so)
    TARGET_LINK_LIBRARIES(testAllegro /usr/lib/x86_64-linux-gnu/liballegro_acodec.so)
    
    
    #TARGET_LINK_LIBRARIES(testAllegro "${CMAKE_CURRENT_SOURCE_DIR}/dlller/liballegro.so")
    #TARGET_LINK_LIBRARIES(testAllegro "${CMAKE_CURRENT_SOURCE_DIR}/dlller/liballegro_image.so")
    #TARGET_LINK_LIBRARIES(testAllegro "${CMAKE_CURRENT_SOURCE_DIR}/dlller/liballegro_primitives.so")
    #TARGET_LINK_LIBRARIES(testAllegro "${CMAKE_CURRENT_SOURCE_DIR}/dlller/liballegro_font.so")
    #TARGET_LINK_LIBRARIES(testAllegro "${CMAKE_CURRENT_SOURCE_DIR}/dlller/liballegro_ttf.so")
    #TARGET_LINK_LIBRARIES(testAllegro "${CMAKE_CURRENT_SOURCE_DIR}/dlller/liballegro_audio.so")
    #TARGET_LINK_LIBRARIES(testAllegro "${CMAKE_CURRENT_SOURCE_DIR}/dlller/liballegro_acodec.so")
endif()




#[[
# Specify the minimum version for CMake
cmake_minimum_required(VERSION 2.8)

# projects name
project(TOP)

message("allegro libraries = " ${ALLEGRO_LIBRARIES})

set(CMAKE_CXX_STANDARD 11)  # enable C++11 standard
# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
# Finds Allegro using pkgconfig, so it must be configured correctly
find_package(Allegro5 REQUIRED)
# Set include and lib dirs.
include_directories(${ALLEGRO_INCLUDE_DIR})

#set(FD_LIBS ${LIBS} ${OBJC_LIBRARIES} ${ALLEGRO_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${ALLEGRO_LIBRARIES})

# The following folder will be included
# include_directories(${PROJECT_SOURCE_DIR}/src)
# include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories("${PROJECT_SOURCE_DIR}")

file(GLOB all_SRCS
        "*.h"
        "*.cpp"
        )

add_executable(top ${all_SRCS})
target_link_libraries(top ${FD_LIBS})
]]
