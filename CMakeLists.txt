# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS components/common/app_wifi components/common/app_reset)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(PROJECT_VER "5.0")
project(garage)
