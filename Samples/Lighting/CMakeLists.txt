# Configure Lighting demo build

set(HEADER_FILES include/Lighting.h)
set(SOURCE_FILES src/Lighting.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_Lighting SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(Sample_Lighting ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_Lighting)

add_dependencies(Sample_Lighting SampleBrowser)
