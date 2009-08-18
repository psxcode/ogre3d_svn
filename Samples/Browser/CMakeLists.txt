#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Browser demo build

if (OGRE_BUILD_PLATFORM_IPHONE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/SampleBrowser.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/SampleBrowser.mm COPYONLY)
  set(SOURCE_FILES src/SampleBrowser.mm)
else ()
  set(SOURCE_FILES src/SampleBrowser.cpp)
endif ()

set (HEADER_FILES
	include/SampleBrowser.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/Sample.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SampleContext.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SamplePlugin.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SdkCameraMan.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SdkSample.h
	${CMAKE_SOURCE_DIR}/Samples/Common/include/SdkTrays.h
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(SampleBrowser WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

target_link_libraries(SampleBrowser ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample(SampleBrowser)

if (SAMPLE_DEPENDENCIES)
  add_dependencies(SampleBrowser ${SAMPLE_DEPENDENCIES})
endif ()
