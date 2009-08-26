# Configure SkyDome demo build

set(HEADER_FILES include/SkyDome.h)
set(SOURCE_FILES src/SkyDome.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_SkyDome SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_SkyDome ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_SkyDome)

add_dependencies(Sample_SkyDome SampleBrowser)
