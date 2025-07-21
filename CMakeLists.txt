cmake_minimum_required(VERSION 3.10)
project(SimpleWindow C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add the executable
add_executable(simple_window src/simple_window.c src/conway.c)

# Link against the GDI library
target_link_libraries(simple_window PRIVATE gdi32)
