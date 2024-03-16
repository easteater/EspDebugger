cmake_minimum_required(VERSION 3.5)

project(EspDebugger LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src/gps_module_config/"
    "${PROJECT_SOURCE_DIR}/src/gnss_parser/"
    "${PROJECT_SOURCE_DIR}/src/locate_infomation/"
    "${PROJECT_SOURCE_DIR}/src/main_window/"
    "${PROJECT_SOURCE_DIR}/src/signal_to_noise_ratio/"
    "${PROJECT_SOURCE_DIR}/src/tool/"
    "${PROJECT_SOURCE_DIR}/src/gnss_simulator/"
)

# file(GLOB xx "*.cpp")
find_package(Qt5 COMPONENTS Widgets REQUIRED SerialPort     )

if(ANDROID)
  add_library(EspDebugger SHARED
  )
else()
  add_executable(EspDebugger
            resources.qrc
            main.cpp
            src/gps_module_config/gpsmoduleconfig.cpp src/gps_module_config/gpsmoduleconfig.h src/gps_module_config/gpsmoduleconfig.ui
            src/gnss_parser/gnssparser.cpp src/gnss_parser/gnssparser.h
            src/locate_infomation/locateinformation.cpp src/locate_infomation/locateinformation.h src/locate_infomation/locateinformation.ui
            src/main_window/mainwindow.cpp src/main_window/mainwindow.h src/main_window/mainwindow.ui
            src/signal_to_noise_ratio/signaltonoiseratio.cpp src/signal_to_noise_ratio/signaltonoiseratio.h src/signal_to_noise_ratio/signaltonoiseratio.ui
            src/tool/tool.cpp src/tool/tool.h
            src/gnss_simulator/gnsssimulator.h src/gnss_simulator/gnsssimulator.cpp


  )
endif()
find_package(Qt5 COMPONENTS WebEngineWidgets REQUIRED)

target_link_libraries(EspDebugger PRIVATE Qt5::Widgets Qt5::WebEngineWidgets Qt5::SerialPort)

 include_directories(${Qt5Widgets_INCLUDE_DIRS}
                    ${Qt5WebEngineWidgets_INCLUDE_DIRS}
                    ${Qt5SerialPort_INCLUDE_DIRS})
