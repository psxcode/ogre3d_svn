# Configure Bezier demo build

set(HEADER_FILES include/BezierPatch.h)
set(SOURCE_FILES src/BezierPatch.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_BezierPatch SHARED ${HEADER_FILES} ${SOURCE_FILES} )

target_link_libraries(Sample_BezierPatch ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_BezierPatch)

add_dependencies(Sample_BezierPatch SampleBrowser)

