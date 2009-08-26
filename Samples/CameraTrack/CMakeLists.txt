# Configure CameraTrack demo build

set(SOURCE_FILES src/CameraTrack.cpp)
set(HEADER_FILES include/CameraTrack.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_CameraTrack SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(Sample_CameraTrack ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_CameraTrack)

add_dependencies(Sample_CameraTrack SampleBrowser)
