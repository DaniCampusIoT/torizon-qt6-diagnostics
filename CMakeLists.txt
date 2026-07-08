cmake_minimum_required(VERSION 3.16)
project(torizonqt6diagnostics LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_add_executable(torizonqt6diagnostics WIN32
    main.cpp
    src/spi_device.cpp
    src/i2c_bus.cpp
    src/diagnostics_model.cpp
    include/diagnostics_model.hpp   # <-- añade esta línea
)

target_include_directories(torizonqt6diagnostics PRIVATE include)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(torizonqt6diagnostics PRIVATE QT_QML_DEBUG)
endif()

set_target_properties(torizonqt6diagnostics PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "bin"
    ARCHIVE_OUTPUT_DIRECTORY "lib"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(torizonqt6diagnostics PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

qt_add_qml_module(torizonqt6diagnostics
    URI torizonqt6diagnostics
    VERSION 1.0
    QML_FILES
        "QML/main.qml"
    RESOURCES
        "QML/assets/torizon-logo.png"
)