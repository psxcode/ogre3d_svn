# Configure ParticleFX demo build

set(HEADER_FILES include/ParticleFX.h)
set(SOURCE_FILES src/ParticleFX.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_ParticleFX SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_ParticleFX ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_ParticleFX)

add_dependencies(Sample_ParticleFX SampleBrowser)
