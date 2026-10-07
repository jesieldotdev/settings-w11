cmake_minimum_required(VERSION 3.20)
project(settings-w11 LANGUAGES CXX)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 REQUIRED COMPONENTS Widgets Qml Quick QuickWidgets)
find_package(KF6 REQUIRED COMPONENTS KCMUtils CoreAddons Config DBusAddons Service)

set(CMAKE_AUTOMOC ON)
add_executable(settings-w11 src/main.cpp src/mainwindow.cpp src/appsmodel.cpp)
file(GLOB PAGES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} pages/*.qml pages/components/*.qml)
qt_add_resources(settings-w11 "pages" PREFIX "/" FILES ${PAGES})
target_link_libraries(settings-w11 Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::QuickWidgets KF6::KCMUtils KF6::CoreAddons KF6::ConfigCore KF6::DBusAddons KF6::Service)

install(TARGETS settings-w11 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES data/settings-w11.desktop DESTINATION ${KDE_INSTALL_APPDIR})
