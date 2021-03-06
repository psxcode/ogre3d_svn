#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL ES 1.x RenderSystem build

set(HEADER_FILES
  include/OgreGLESContext.h
  include/OgreGLESDefaultHardwareBufferManager.h
  include/OgreGLESDepthBuffer.h
  include/OgreGLESFBOMultiRenderTarget.h
  include/OgreGLESFBORenderTexture.h
  include/OgreGLESFrameBufferObject.h
  include/OgreGLESGpuProgramManager.h
  include/OgreGLESHardwareBufferManager.h
  include/OgreGLESHardwareIndexBuffer.h
  include/OgreGLESHardwarePixelBuffer.h
  include/OgreGLESHardwareVertexBuffer.h
  include/OgreGLESPBRenderTexture.h
  include/OgreGLESPBuffer.h
  include/OgreGLESPixelFormat.h
  include/OgreGLESPlugin.h
  include/OgreGLESPrerequisites.h
  include/OgreGLESRenderSystem.h
  include/OgreGLESRenderTexture.h
  include/OgreGLESSupport.h
  include/OgreGLESTexture.h
  include/OgreGLESTextureManager.h
)


set(SOURCE_FILES
  src/OgreGLESContext.cpp
  src/OgreGLESDefaultHardwareBufferManager.cpp
  src/OgreGLESDepthBuffer.cpp
  src/OgreGLESEngineDll.cpp
  src/OgreGLESFBOMultiRenderTarget.cpp
  src/OgreGLESFBORenderTexture.cpp
  src/OgreGLESFrameBufferObject.cpp
  src/OgreGLESGpuProgramManager.cpp
  src/OgreGLESHardwareBufferManager.cpp
  src/OgreGLESHardwareIndexBuffer.cpp
  src/OgreGLESHardwarePixelBuffer.cpp
  src/OgreGLESHardwareVertexBuffer.cpp
  src/OgreGLESPBRenderTexture.cpp
  src/OgreGLESPixelFormat.cpp
  src/OgreGLESPlugin.cpp
  src/OgreGLESRenderSystem.cpp
  src/OgreGLESRenderTexture.cpp
  src/OgreGLESSupport.cpp
  src/OgreGLESTexture.cpp
  src/OgreGLESTextureManager.cpp
)

# Add system specific settings
if (WIN32)
  set(PLATFORM_HEADERS 
    include/EGL/WIN32/OgreWin32EGLContext.h
    include/EGL/WIN32/OgreWin32EGLSupport.h
    include/EGL/WIN32/OgreWin32EGLRenderTexture.h
    include/EGL/WIN32/OgreWin32EGLWindow.h
    include/EGL/WIN32/OgreGLESUtil.h
    include/EGL/OgreEGLContext.h
    include/EGL/OgreEGLRenderTexture.h
    include/EGL/OgreEGLSupport.h
    include/EGL/OgreEGLWindow.h
  )
  set(PLATFORM_SOURCES
    src/EGL/WIN32/OgreWin32EGLSupport.cpp
    src/EGL/WIN32/OgreWin32EGLRenderTexture.cpp
    src/EGL/WIN32/OgreWin32EGLWindow.cpp
    src/EGL/WIN32/OgreWin32EGLContext.cpp
    src/EGL/OgreEGLContext.cpp
    src/EGL/OgreEGLRenderTexture.cpp
    src/EGL/OgreEGLSupport.cpp
    src/EGL/OgreEGLWindow.cpp
  )
  include_directories(src/EGL/WIN32 include/EGL/WIN32)
  link_directories(${OPENGLES_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "WIN32")
# Add system specific settings
elseif (SYMBIAN)
  set(PLATFORM_HEADERS 
    include/EGL/SYMBIAN/OgreSYMBIANEGLContext.h
    include/EGL/SYMBIAN/OgreSYMBIANEGLSupport.h
    include/EGL/SYMBIAN/OgreSYMBIANEGLRenderTexture.h
    include/EGL/SYMBIAN/OgreSYMBIANEGLWindow.h
    include/EGL/SYMBIAN/OgreGLESUtil.h
    include/EGL/OgreEGLContext.h
    include/EGL/OgreEGLRenderTexture.h
    include/EGL/OgreEGLSupport.h
    include/EGL/OgreEGLWindow.h
  )
  set(PLATFORM_SOURCES
    src/EGL/SYMBIAN/OgreSYMBIANEGLSupport.cpp
    src/EGL/SYMBIAN/OgreSYMBIANEGLRenderTexture.cpp
    src/EGL/SYMBIAN/OgreSYMBIANEGLWindow.cpp
    src/EGL/SYMBIAN/OgreSYMBIANEGLContext.cpp
    src/EGL/OgreEGLContext.cpp
    src/EGL/OgreEGLRenderTexture.cpp
    src/EGL/OgreEGLSupport.cpp
    src/EGL/OgreEGLWindow.cpp
  )
  include_directories(src/EGL/SYMBIAN include/EGL/SYMBIAN)
  link_directories(${OPENGLES_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "SYMBIAN")
elseif (APPLE)
  set(PLATFORM_HEADERS 
    include/EAGL/OgreEAGLESContext.h
    include/EAGL/OgreEAGLSupport.h
    include/EAGL/OgreEAGLRenderTexture.h
    include/EAGL/OgreEAGLWindow.h
    include/EAGL/OgreGLESUtil.h
  )
  set(PLATFORM_SOURCES
    src/EAGL/OgreEAGLSupport.mm
    src/EAGL/OgreEAGLRenderTexture.mm
    src/EAGL/OgreEAGLWindow.mm
    src/EAGL/OgreEAGLESContext.mm
  )
  include_directories(src/EAGL include/EAGL)
  link_directories(${OPENGLES_LIBRARY_PATH})
  set(PLATFORM_HEADER_INSTALL "APPLE")
elseif (UNIX)
  set(PLATFORM_HEADERS
    include/EGL/X11/OgreX11EGLContext.h
    include/EGL/X11/OgreX11EGLSupport.h
    include/EGL/X11/OgreX11EGLRenderTexture.h
    include/EGL/X11/OgreX11EGLWindow.h
    include/EGL/X11/OgreGLESUtil.h
    include/EGL/OgreEGLContext.h
    include/EGL/OgreEGLRenderTexture.h
    include/EGL/OgreEGLSupport.h
    include/EGL/OgreEGLWindow.h
  )
  set(PLATFORM_SOURCES
    src/EGL/X11/OgreX11EGLContext.cpp
    src/EGL/X11/OgreX11EGLSupport.cpp
    src/EGL/X11/OgreX11EGLRenderTexture.cpp
    src/EGL/X11/OgreX11EGLWindow.cpp
    src/EGL/OgreEGLContext.cpp
    src/EGL/OgreEGLRenderTexture.cpp
    src/EGL/OgreEGLSupport.cpp
    src/EGL/OgreEGLWindow.cpp
  )
  include_directories(src/EGL/X11 include/EGL/X11)
  set(PLATFORM_HEADER_INSTALL "X11")
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
endif ()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
  ${CMAKE_CURRENT_SOURCE_DIR}/include/EAGL
  ${OPENGLES_INCLUDE_DIR}
)

add_library(RenderSystem_GLES ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
target_link_libraries(RenderSystem_GLES OgreMain ${OPENGLES_LIBRARIES} ${PLATFORM_LIBS})

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_GLES PROPERTIES
    COMPILE_DEFINITIONS OGRE_GLESPLUGIN_EXPORTS
  )
endif ()

if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_GLES ${Boost_LIBRARIES})
endif ()

ogre_config_plugin(RenderSystem_GLES)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/GLES)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GLES/${PLATFORM_HEADER_INSTALL})
