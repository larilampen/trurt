cmake_minimum_required( VERSION 3.2.0 )

project(Trurt)

include(dlib/dlib/cmake)

include_directories(tclap/include)

# The version number
set(Trurt_VERSION_MAJOR 0)
set(Trurt_VERSION_MINOR 1)
set(Trurt_VERSION ${Trurt_VERSION_MAJOR}.${Trurt_VERSION_MINOR})
add_definitions(-DVERSION=${Trurt_VERSION})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

mark_as_advanced(CMAKE_INSTALL_PREFIX)

find_package(Qt5Widgets)

if("${CMAKE_VERSION}" VERSION_GREATER 3.1.0)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED YES)
  set(CMAKE_CXX_EXTENSIONS OFF)
ELSE()
  # older cmake versions don't know CMAKE_CXX_STANDARD
  set (CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")
ENDIF()

set(SRC_LIST main.cc outputs.cc VideoFaceDetector.cc)
add_executable(Trurt ${SRC_LIST})

target_link_libraries(Trurt dlib::dlib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic")

# for configured header files:
include_directories(${PROJECT_BINARY_DIR})

# Copy model files.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/haarcascade_frontalface_default.xml ${CMAKE_CURRENT_BINARY_DIR}/data/haarcascade_frontalface_default.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/landmarks68.dat ${CMAKE_CURRENT_BINARY_DIR}/data/landmarks68.dat COPYONLY)

find_package( OpenCV REQUIRED core imgproc objdetect highgui )
target_link_libraries(Trurt ${OpenCV_LIBS} )
