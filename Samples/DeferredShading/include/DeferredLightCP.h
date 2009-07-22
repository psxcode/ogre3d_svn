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

class DeferredLightRenderOperation : public Ogre::CompositorInstance::RenderSystemOperation
{
public:	
	/// Set state to SceneManager and RenderSystem
	virtual void execute(SceneManager *sm, RenderSystem *rs) 
	{
		bool debug = true;
	}

	virtual ~DeferredLightRenderOperation() {}
};

class DeferredLightCompositionPass : public Ogre::CustomCompositionPass
{
public:
	virtual Ogre::CompositorInstance::RenderSystemOperation* createOperation(const CompositionPass* pass)
	{
		return OGRE_NEW DeferredLightRenderOperation;
	}

protected:
	virtual ~DeferredLightCompositionPass() {}
};

#endif
