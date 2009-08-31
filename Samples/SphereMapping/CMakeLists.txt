# Configure SphereMapping demo build

set(HEADER_FILES include/SphereMapping.h)
set(SOURCE_FILES src/SphereMapping.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_SphereMapping SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_SphereMapping ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_SphereMapping)

add_dependencies(Sample_SphereMapping SampleBrowser)
