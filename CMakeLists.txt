cmake_minimum_required(VERSION 3.16)

# Nom du projet
project(Basic55)

# Définir le type de projet ESP-IDF
set(CMAKE_TOOLCHAIN_FILE $ENV{IDF_PATH}/tools/cmake/toolchain-esp32.cmake)
set(IDF_TARGET esp32)

# Inclure les composants ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Basic55)

