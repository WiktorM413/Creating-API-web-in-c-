cmake_minimum_required(VERSION 3.16)

project(Creating_API LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.0+1
)
FetchContent_MakeAvailable(Crow)

add_executable(Creating_API main.cpp
)

target_link_libraries(Creating_API PRIVATE Crow)

include(GNUInstallDirs)
install(TARGETS Creating_API
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
