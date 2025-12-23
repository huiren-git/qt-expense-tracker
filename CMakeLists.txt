cmake_minimum_required(VERSION 3.5)

project(qt-expense-tracker LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Widgets Sql Charts REQUIRED)

add_executable(qt-expense-tracker
  src/main.cpp
  src/mainwindow.cpp
  src/mainwindow.h
  src/mainwindow.ui
  src/db/database_manager.cpp
  src/db/database_manager.h
  src/ui/weekviewwidget.h
  src/ui/weekviewwidget.cpp
  src/ui/monthviewwidget.h
  src/ui/monthviewwidget.cpp
  src/ui/yearviewwidget.h
  src/ui/yearviewwidget.cpp
  src/ui/daydetailwidget.h
  src/ui/daydetailwidget.cpp
  src/ui/recordeditdialog.h
  src/ui/recordeditdialog.cpp
  src/ui/helpdialog.h
  src/ui/helpdialog.cpp
)

target_link_libraries(qt-expense-tracker
    PRIVATE Qt5::Widgets
    Qt5::Sql
    Qt5::Charts)
