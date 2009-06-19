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

#ifndef _DEFERREDSHADINGLOGIC_H
#define _DEFERREDSHADINGLOGIC_H

#include <OgreCompositorInstance.h>
#include <OgreCompositorLogic.h>
#include <map>

class DeferredShadingLogic : public Ogre::CompositorLogic
{
public:

	virtual void compositorInstanceCreated(Ogre::CompositorInstance* newInstance);

	virtual void compositorInstanceDestroyed(Ogre::CompositorInstance* destroyedInstance);
private:
	typedef std::map<Ogre::CompositorInstance*, Ogre::CompositorInstance::Listener*> ListenerMap;
	ListenerMap mListeners;

};

#endif
