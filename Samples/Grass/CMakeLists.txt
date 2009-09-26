# Configure Bezier demo build

set(HEADER_FILES include/Grass.h)
set(SOURCE_FILES src/Grass.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_Grass SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(Sample_Grass ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_Grass)

add_dependencies(Sample_Grass SampleBrowser)

