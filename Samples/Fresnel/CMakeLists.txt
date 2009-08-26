# Configure Fresnel demo build

set(HEADER_FILES include/Fresnel.h)
set(SOURCE_FILES src/Fresnel.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Sample_Fresnel SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Sample_Fresnel ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(Sample_Fresnel)

add_dependencies(Sample_Fresnel SampleBrowser)
