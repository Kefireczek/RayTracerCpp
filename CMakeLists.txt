cmake_minimum_required(VERSION 3.10)
project(RayTracerC++)

# Ustaw œcie¿kê do vcpkg
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Patryk/vcpkg/scripts/buildsystems/vcpkg.cmake")

# W³¹cz C++17
set(CMAKE_CXX_STANDARD 17)

# ZnajdŸ pakiet SFML
find_package(SFML COMPONENTS graphics window system REQUIRED)

# Dodaj executable
add_executable(RayTracerC++ RayTracerC++.cpp)

# Podlinkuj SFML
target_link_libraries(RayTracerC++ sfml-graphics sfml-window sfml-system)
