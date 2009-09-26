# Configure Smoke demo build

set(HEADER_FILES include/Smoke.h)
set(SOURCE_FILES src/Smoke.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_Smoke SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(Sample_Smoke ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_Smoke)

add_dependencies(Sample_Smoke SampleBrowser)

