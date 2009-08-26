# Configure FacialAnimation demo build

set(HEADER_FILES include/FacialAnimation.h)
set(SOURCE_FILES src/FacialAnimation.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_FacialAnimation SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_FacialAnimation ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_FacialAnimation)

add_dependencies(Sample_FacialAnimation SampleBrowser)
