# Configure Bezier demo build

set(HEADER_FILES include/CelShading.h)
set(SOURCE_FILES src/CelShading.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_CelShading SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(Sample_CelShading ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_CelShading)

add_dependencies(Sample_CelShading SampleBrowser)

