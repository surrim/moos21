set(CMAKE_CXX_STANDARD 20)

add_executable(moos21 base.cpp connection.cpp helpmenu.cpp infodialog.cpp inputdialog.cpp moosmenu.cpp settingsmenu.cpp tools.cpp usermenu.cpp viewmenu.cpp whoisdialog.cpp)
find_package(wxWidgets REQUIRED gl core base net)
find_package(ZLIB)
include(${wxWidgets_USE_FILE})
# and for each of your dependent executable/library targets:
target_link_libraries(moos21 ${wxWidgets_LIBRARIES} ${ZLIB_LIBRARIES})
