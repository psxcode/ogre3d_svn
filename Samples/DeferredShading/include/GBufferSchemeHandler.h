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

#ifndef _GBUFFERSCHEMEHANDLER_H
#define _GBUFFERSCHEMEHANDLER_H

#include <OgreMaterialManager.h>
#include "GBufferMaterialGenerator.h"

class GBufferSchemeHandler : public Ogre::MaterialManager::Listener
{
public:
	/** @copydoc MaterialManager::Listener::handleSchemeNotFound */
	virtual Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex, 
		const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
		const Ogre::Renderable* rend);
protected:
	GBufferMaterialGenerator mMaterialGenerator;

	static const Ogre::String NORMAL_MAP_PATTERN;
	struct MaterialProperties {
		MaterialProperties() : normalMap(0), isSkinned(false), vertexColourType(0) {}
		Ogre::vector<Ogre::TextureUnitState*>::type regularTextures;
		Ogre::TextureUnitState* normalMap;
		bool isSkinned;
		Ogre::TrackVertexColourType vertexColourType;
	};

	MaterialProperties inspectMaterial(Ogre::Technique* technique, unsigned short lodIndex, const Ogre::Renderable* rend);
	MaterialGenerator::Perm getPermutation(const MaterialProperties& props);
	void fillPass(Ogre::Pass* gBufferPass, Ogre::Pass* originalPass, const MaterialProperties& props);
	bool checkNormalMap(Ogre::TextureUnitState* tus, MaterialProperties& props);
	
	
};

#endif
