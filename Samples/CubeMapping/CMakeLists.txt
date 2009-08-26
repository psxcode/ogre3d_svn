# Configure CubeMapping demo build

set(HEADER_FILES include/CubeMapping.h)
set(SOURCE_FILES src/CubeMapping.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_CubeMapping SHARED ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(Sample_CubeMapping ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_CubeMapping)

add_dependencies(Sample_CubeMapping SampleBrowser)
