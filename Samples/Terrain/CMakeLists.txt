# Configure Terrain demo build

set(HEADER_FILES include/Terrain.h)
set(SOURCE_FILES src/Terrain.cpp)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/Components/Terrain/include)

add_library(Sample_Terrain SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_Terrain ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_Terrain_LIBRARIES})
ogre_config_sample(Sample_Terrain)

add_dependencies(Sample_Terrain SampleBrowser)
