/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __D3D10PREREQUISITES_H__
#define __D3D10PREREQUISITES_H__

#include "OgrePrerequisites.h"

// Define versions for if DirectX is in use (Win32 only)
#define DIRECT3D_VERSION 0x0900

// some D3D commonly used macros
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


#include "OgreNoMemoryMacros.h"
#undef NOMINMAX
#define NOMINMAX // required to stop windows.h screwing up std::min definition
#include <d3d10.h>
#include <d3dx10.h>
#include <dxerr.h>
// only for D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY
#include <d3dx9shader.h>
#include "OgreMemoryMacros.h"


namespace Ogre
{
	// Predefine classes
	class D3D10RenderSystem;
	class D3D10RenderWindow;
	class D3D10Texture;
	class D3D10TextureManager;
	class D3D10Driver;
	class D3D10DriverList;
	class D3D10VideoMode;
	class D3D10VideoModeList;
	class D3D10GpuProgram;
	class D3D10GpuProgramManager;
    class D3D10HardwareBufferManager;
    class D3D10HardwareIndexBuffer;
    class D3D10HLSLProgramFactory;
    class D3D10HLSLProgram;
    class D3D10VertexDeclaration;

// Should we ask D3D to manage vertex/index buffers automatically?
// Doing so avoids lost devices, but also has a performance impact
// which is unacceptably bad when using very large buffers
#define OGRE_D3D_MANAGE_BUFFERS 1

    //-------------------------------------------
	// Windows setttings
	//-------------------------------------------
#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(OGRE_STATIC_LIB)
#	ifdef OGRED3DENGINEDLL_EXPORTS
#		define _OgreD3D10Export __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define _OgreD3D10Export
#       else
#    		define _OgreD3D10Export __declspec(dllimport)
#       endif
#	endif
#else
#	define _OgreD3D10Export
#endif	// OGRE_WIN32
}
#endif