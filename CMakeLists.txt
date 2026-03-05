cmake_minimum_required(VERSION 3.16)

project(Medusa LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES)
  add_compile_options(/EHsc /permissive-)
else()
  add_compile_options(-fpermissive)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

# draw_* pull in cktypes/psoop/dynarray via includes in the source itself.
add_executable(draw_pb_sphere src/draw_pb_sphere.cc)
add_executable(drawpullback src/drawpullback.cc)

# find_matings and mate_interact need the medusa implementation units.
add_executable(find_matings
  src/find_matings.cc
  src/medusa_A.cc
  src/medusa_B.cc
)

add_executable(mate_interact
  src/mate_interact.cc
  src/medusa_A.cc
  src/medusa_B.cc
)
