/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

#ifndef _GBUFFER_MATERIAL_GENERATOR_H_
#define _GBUFFER_MATERIAL_GENERATOR_H_

#include "MaterialGenerator.h"

class GBufferMaterialGenerator : public MaterialGenerator
{
public:
	
	enum GBufferPermutations 
	{
		//(Regular) Textures
		GBP_NO_TEXTURES =		0x00000000,
		GBP_ONE_TEXTURE =		0x00000001,
		GBP_TWO_TEXTURES =		0x00000002,
		GBP_THREE_TEXTURES =	0x00000003,
		GBP_TEXTURE_MASK =		0x0000000F,

		GBP_NORMAL_MAP =		0x00000010,
		
		GBP_NO_TEXCOORDS =		0x00000000,
		GBP_ONE_TEXCOORD =		0x00000100,
		GBP_TWO_TEXCOORDS =		0x00000200,
		GBP_TEXCOORD_MASK =		0x00000F00,

		GBP_SKINNED =			0x00010000
	};
	
	static const Ogre::uint32 FS_MASK =		0x0000FFFF;
	static const Ogre::uint32 VS_MASK =		0x00FFFF00;
	static const Ogre::uint32 MAT_MASK =	0xFF00FFFF;

	GBufferMaterialGenerator();
};

#endif