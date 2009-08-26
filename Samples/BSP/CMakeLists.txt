# Configure BSP demo build

set(SOURCE_FILES src/BSP.cpp)
set(HEADER_FILES include/BSP.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_BSP SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_BSP ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_BSP)

add_dependencies(Sample_BSP SampleBrowser)

