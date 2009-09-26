# Configure TextureFX demo build

set(HEADER_FILES include/TextureFX.h)
set(SOURCE_FILES src/TextureFX.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Sample_TextureFX SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(Sample_TextureFX ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_TextureFX)

add_dependencies(Sample_TextureFX SampleBrowser)

