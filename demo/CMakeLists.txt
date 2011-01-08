
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(rtpsend_SRCS rtpsend.cpp)
add_executable(rtpsend ${rtpsend_SRCS})
target_link_libraries(rtpsend ccrtp1)


########### next target ###############

set(rtplisten_SRCS rtplisten.cpp)
add_executable(rtplisten ${rtplisten_SRCS})
target_link_libraries(rtplisten ccrtp1)


########### next target ###############

set(rtphello_SRCS rtphello.cpp)
add_executable(rtphello ${rtphello_SRCS})
target_link_libraries(rtphello ${KDE4_KDECORE_LIBS} ccrtp1)


########### next target ###############

set(rtpduphello_SRCS rtpduphello.cpp)
add_executable(rtpduphello ${rtpduphello_SRCS})
target_link_libraries(rtpduphello ccrtp1)


########### next target ###############

set(audiorx_SRCS audiorx.cpp audio.h)
add_executable(audiorx ${audiorx_SRCS})
target_link_libraries(audiorx ccrtp1)


########### next target ###############

set(audiotx_SRCS audiotx.cpp)
add_executable(audiotx ${audiotx_SRCS})
target_link_libraries(audiotx ccrtp1)

########### next target ###############

set(ccrtptest_SRCS ccrtptest.cpp)
add_executable(ccrtptest ${ccrtptest_SRCS})
target_link_libraries(ccrtptest ccrtp1)

########### next target ###############

if (SRTP_SUPPORT)
  set(ccsrtptest_SRCS ccsrtptest.cpp)
  add_executable(ccsrtptest ${ccsrtptest_SRCS})
  target_link_libraries(ccsrtptest ccrtp1)
endif()
