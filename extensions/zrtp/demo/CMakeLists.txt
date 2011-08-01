########### next target ###############

add_executable(demo-zrtptest zrtptest.cpp)
target_link_libraries(demo-zrtptest zrtpcpp)
add_dependencies(demo-zrtptest zrtpcpp)

########### next target ###############

add_executable(demo-zrtpMulti zrtptestMulti.cpp)
target_link_libraries(demo-zrtpMulti zrtpcpp)
add_dependencies(demo-zrtpMulti zrtpcpp)

########### next target ###############

#add_executable(wrappertest wrappertest.c)
#target_link_libraries(wrappertest zrtpcpp)

########### install files ###############
# None
