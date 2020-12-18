cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./bin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project( Computer_Networking_Project )

include_directories( includes )

set( src_dir "src")

set( web_srcs   ${src_dir}/web_main.cpp
                ${src_dir}/http.cpp 
                ${src_dir}/IO.cpp 
                ${src_dir}/tcp_server.cpp 
                ${src_dir}/web_server.cpp 
                ${src_dir}/web_main.cpp)

add_executable( web_main ${web_srcs} )

find_package(CURL)

if (CURL_FOUND)
    include_directories( ${CURL_INCLUDE_DIR} )
    target_link_libraries( web_main ${CURL_LIBRARY} )
endif ()

find_package(PkgConfig QUIET)
pkg_check_modules(SQLITE3 sqlite3)

if (SQLITE3_FOUND)
    include_directories(${SQLITE3_INCLUDE_DIRs})
    target_link_libraries(web_main ${SQLITE3_LIBRARIES})
endif ()

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(web_main PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(web_main "${CMAKE_THREAD_LIBS_INIT}")
endif()

target_compile_definitions(web_main PUBLIC -DSQLITE_THREADSAFE=2)



set( test_srcs  ${src_dir}/test.cpp
                ${src_dir}/webcam.cpp
                ${src_dir}/stream_server.cpp
                ${src_dir}/stream_client.cpp)

add_executable(test_main ${test_srcs})

find_package( OpenCV 4.5 REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS} )

target_link_libraries( test_main ${OpenCV_LIBS} )

pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

include_directories(${LIBAV_INCLUDE_DIRS})
target_link_libraries(test_main PkgConfig::LIBAV)

