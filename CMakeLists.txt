cmake_minimum_required(VERSION 3.3)
project(basehttpd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/common/bh_malloc.c
    src/common/bh_malloc.h
    src/core/basehttpd.c
    src/core/basehttpd.h
    src/event/bh_epoll.c
    src/event/bh_event.c
    src/event/bh_event.h
    src/event/bh_evport.c
    src/event/bh_kqueue.c
    src/event/bh_poll.c
    src/event/bh_select.c)

add_executable(basehttpd ${SOURCE_FILES})