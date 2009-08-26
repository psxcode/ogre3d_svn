# Configure SkyBox demo build

set(HEADER_FILES include/SkyBox.h)
set(SOURCE_FILES src/SkyBox.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_SkyBox SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_SkyBox ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_SkyBox)

add_dependencies(Sample_SkyBox SampleBrowser)
