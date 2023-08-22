cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#####################################

project(html-ui LANGUAGES CXX)

#####################################

option(HTML_UI_ENABLE_SAMPLE "Enable html-ui sample project" OFF)

#####################################

add_subdirectory(src)

if(HTML_UI_ENABLE_SAMPLE)
  add_subdirectory(sample)
endif()
