project(test-app C CXX)

set(PROJECT_TARGET gtk_curl_app)

# Set the minimum version of cmake required to build this project
cmake_minimum_required(VERSION 3.10)

set(THREADS_PREFER_PTHREAD_FLAG ON)

# Use the package PkgConfig to detect GTK+ headers/library files
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
if(WIN32)
    add_definitions("-DCURL_STATICLIB")
endif()
find_package(CURL REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

set (HEADERS
 window.h
)

set(SOURCES 
  main.cc
  window.cc  
  ${HEADERS}
)

if(WIN32)
    # Definition just in case
    target_compile_definitions(${PROJECT_TARGET} PRIVATE _WIN32)
   set(WINDOWS_CURL_LIBS idn2 unistring iconv charset ssh2 gcrypt gpg-error ws2_32 advapi32 crypt32 wldap32)
endif()

# Create the executable
add_executable(${PROJECT_TARGET} ${SOURCES})
target_link_libraries(${PROJECT_TARGET} PRIVATE ${GTKMM_LIBRARIES} ${CURL_LIBRARIES} ${WINDOWS_CURL_LIBS} Threads::Threads)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries

target_include_directories(${PROJECT_TARGET} PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(${PROJECT_TARGET} PRIVATE ${GTKMM_LIBRARY_DIRS} ${CURL_INCLUDE_DIRS})
target_compile_options(${PROJECT_TARGET} PRIVATE ${GTKMM_CFLAGS_OTHER})