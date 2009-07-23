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

#ifndef _DEFERRED_LIGHT_CP_H
#define _DEFERRED_LIGHT_CP_H

#include "OgreCompositorInstance.h"
#include "OgreCustomCompositionPass.h"

#include "DLight.h"
#include "MaterialGenerator.h"
#include "AmbientLight.h"

class DeferredLightRenderOperation : public Ogre::CompositorInstance::RenderSystemOperation
{
public:
	DeferredLightRenderOperation(Ogre::CompositorInstance* instance, const Ogre::CompositionPass* pass);
	/// Set state to SceneManager and RenderSystem
	virtual void execute(Ogre::SceneManager *sm, Ogre::RenderSystem *rs);

	virtual ~DeferredLightRenderOperation();
private:

	/** Create a new MiniLight 
	 */
	DLight *createDLight(Ogre::Light* light);

	/** Destroy a MiniLight
	 */
	void destroyDLight(Ogre::Light* light);

	Ogre::String mTexName0;
	Ogre::String mTexName1;

	MaterialGenerator* mLightMaterialGenerator;

	typedef std::map<Ogre::Light*, DLight*> LightsMap;
	LightsMap mLights;
	AmbientLight* mAmbientLight;
};

class DeferredLightCompositionPass : public Ogre::CustomCompositionPass
{
public:
	virtual Ogre::CompositorInstance::RenderSystemOperation* createOperation(
		Ogre::CompositorInstance* instance, const Ogre::CompositionPass* pass)
	{
		return OGRE_NEW DeferredLightRenderOperation(instance, pass);
	}

protected:
	virtual ~DeferredLightCompositionPass() {}
};

#endif
