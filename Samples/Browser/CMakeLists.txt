#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Browser demo build

set(SOURCE_FILES src/SampleBrowser.cpp)

if (OGRE_BUILD_PLATFORM_IPHONE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/SampleBrowser.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
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

if (SAMPLE_DEPENDENCIES)
  add_dependencies(SampleBrowser ${SAMPLE_DEPENDENCIES})
endif ()


# configure SampleBrowser build

ogre_config_common(SampleBrowser)

# set install RPATH for Unix systems
if (UNIX AND OGRE_FULL_RPATH)
	set_property(TARGET SampleBrowser APPEND PROPERTY
		INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
	set_property(TARGET SampleBrowser PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

if (APPLE)
	# On OS X, create .app bundle
	set_property(TARGET SampleBrowser PROPERTY MACOSX_BUNDLE TRUE)
	# also, symlink frameworks so .app is standalone
	# NOTE: $(CONFIGURATION) is not resolvable at CMake run time, it's only 
	# valid at build time (hence parenthesis rather than braces)
	set (OGRE_SAMPLE_CONTENTS_PATH 
		${CMAKE_BINARY_DIR}/bin/$(CONFIGURATION)/SampleBrowser.app/Contents)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Ogre.framework 
		${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		COMMAND ln ARGS -s -f ${CMAKE_SOURCE_DIR}/Dependencies/Cg.framework 
		${OGRE_SAMPLE_CONTENTS_PATH}/Frameworks/
		)
	# now cfg files
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Resources
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/plugins.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/resources.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/media.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/bin/samples.cfg 
		${OGRE_SAMPLE_CONTENTS_PATH}/Resources/
		)

	# now plugins
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_SAMPLE_CONTENTS_PATH}/Plugins)
	if (OGRE_BUILD_RENDERSYSTEM_GL)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/RenderSystem_GL.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif ()
	if (OGRE_BUILD_PLUGIN_BSP)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_BSPSceneManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_CG)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_CgProgramManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_OCTREE)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_OctreeSceneManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_PCZ)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_PCZSceneManager.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/    
			)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_OctreeZone.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_PFX)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Plugin_ParticleFX.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()

	# Components
	if (OGRE_BUILD_COMPONENT_PAGING)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/libOgrePaging.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_PROPERTY)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/libOgreProperty.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/libOgreRTShaderSystem.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_TERRAIN)
		add_custom_command(TARGET SampleBrowser POST_BUILD
			COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/libOgreTerrain.dylib 
			${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
			)
	endif()

	# Add links for the sample plugins
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_BezierPatch.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_BSP.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_CameraTrack.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_CelShading.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_CubeMapping.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_Dot3Bump.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_DynTex.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_FacialAnimation.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_Fresnel.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_Grass.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_Lighting.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_ParticleFX.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_SkeletalAnimation.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_SkyBox.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_SkyDome.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_SkyPlane.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_Smoke.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_SphereMapping.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_Terrain.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_TextureFX.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
	add_custom_command(TARGET SampleBrowser POST_BUILD
		COMMAND ln ARGS -s -f ${CMAKE_BINARY_DIR}/lib/$(CONFIGURATION)/Sample_Transparency.dylib 
		${OGRE_SAMPLE_CONTENTS_PATH}/Plugins/
		)
endif (APPLE)

if (OGRE_INSTALL_SAMPLES)
	ogre_install_target(SampleBrowser "")
	if (OGRE_INSTALL_PDB)
		# install debug pdb files
		install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_DEBUG_PATH}/SampleBrowser.pdb
			DESTINATION bin${OGRE_DEBUG_PATH} CONFIGURATIONS Debug
			)
		install(FILES ${OGRE_BINARY_DIR}/bin${OGRE_RELWDBG_PATH}/SampleBrowser.pdb
			DESTINATION bin${OGRE_RELWDBG_PATH} CONFIGURATIONS RelWithDebInfo
			)
	endif ()
endif ()	

