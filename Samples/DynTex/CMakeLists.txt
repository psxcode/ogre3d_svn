# Configure DynTex demo build

set(HEADER_FILES include/DynTex.h)
set(SOURCE_FILES src/DynTex.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_DynTex SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_DynTex ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_DynTex)

add_dependencies(Sample_DynTex SampleBrowser)
