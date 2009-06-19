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

#ifndef __SHAREDDATA_H
#define __SHAREDDATA_H

#include <Ogre.h>
#include "MLight.h"
#include "DeferredShading.h"

class SharedData : public Ogre::Singleton<SharedData> {

public:

	SharedData()
		: iRoot(0),
		  iCamera(0),
		  iWindow(0),
		  mAnimState(0),
		  mMLAnimState(0),
		  iMainLight(0)
	{
		iActivate = false;
	}

		~SharedData() {}

		// shared data across the application
		Ogre::Real iLastFrameTime;
		Ogre::Root *iRoot;
		Ogre::Camera *iCamera;
		Ogre::RenderWindow *iWindow;

		DeferredShadingSystem *iSystem;
		bool iActivate;
		bool iGlobalActivate;

		// Animation state for big lights
		Ogre::AnimationState* mAnimState;
		// Animation state for light swarm
		Ogre::AnimationState* mMLAnimState;

		MLight *iMainLight;

		Ogre::vector<Ogre::Node*>::type mLightNodes;

};

#endif