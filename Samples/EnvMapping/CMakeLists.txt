# Configure EnvMapping demo build

set(HEADER_FILES include/EnvMapping.h)
set(SOURCE_FILES src/EnvMapping.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_EnvMapping SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_EnvMapping ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_EnvMapping)

add_dependencies(Sample_EnvMapping SampleBrowser)
