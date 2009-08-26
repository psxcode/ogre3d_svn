# Configure SkeletalAnimation demo build

set(HEADER_FILES include/SkeletalAnimation.h)
set(SOURCE_FILES src/SkeletalAnimation.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_SkeletalAnimation SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(Sample_SkeletalAnimation ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_SkeletalAnimation)

add_dependencies(Sample_SkeletalAnimation SampleBrowser)
