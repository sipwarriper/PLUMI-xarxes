cmake_minimum_required(VERSION 3.9)
project(PLUMI-xarxes)

set(CMAKE_C_STANDARD 99)

add_executable(PLUMI-server MIp2-nodelumi.c MIp2-lumi.c MIp2-lumi.h)
add_executable(PLUMI-client MIp2-p2p.c MIp1-mi.c MIp1-mi.h MIp2-lumi.c MIp2-lumi.h)