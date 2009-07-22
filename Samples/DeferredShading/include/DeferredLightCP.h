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

//Bad practice etc. This is a demo :)
using namespace Ogre;

class DeferredLightRenderOperation : public CompositorInstance::RenderSystemOperation
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
	virtual CompositorInstance::RenderSystemOperation* createOperation(
		CompositorInstance* instance, const CompositionPass* pass)
	{
		const CompositionPass::InputTex& input0 = pass->getInput(0);
		String texName0 = instance->getTextureInstanceName(input0.name, input0.mrtIndex);
		const CompositionPass::InputTex& input1 = pass->getInput(1);
		String texName1 = instance->getTextureInstanceName(input1.name, input1.mrtIndex);
		//TODO : Use these textures to do something good...
		return OGRE_NEW DeferredLightRenderOperation;
	}

protected:
	virtual ~DeferredLightCompositionPass() {}
};

#endif
