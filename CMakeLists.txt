cmake_minimum_required(VERSION 4.1)
project(hw_1_escape_the_upside_down_maze_z_chapman_cub C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(starter_code)
include_directories(part2)

add_executable(hw_1_escape_the_upside_down_maze_z_chapman_cub
        starter_code/Game.h
        starter_code/Entity.h

        game/Game.h
        game/Entity.h
        game/main.cpp
        game/Game.cpp
        game/Entity.cpp
)
