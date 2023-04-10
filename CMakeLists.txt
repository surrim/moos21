cmake_minimum_required(VERSION 3.17)
project(moos VERSION 2.2)

find_package(wxWidgets REQUIRED gl core base net)
find_package(ZLIB)

file(GLOB moos_src "./src/*.h" "./src/*.cpp")
add_executable(${PROJECT_NAME} ${moos_src})

include(${wxWidgets_USE_FILE})
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} ${ZLIB_LIBRARIES})
