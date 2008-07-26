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

#include "ProceduralManualObject.h"

namespace Ogre
{
	const String& ProceduralManualObject::getMovableType(void) const
	{
		return ProceduralManualObjectFactory::FACTORY_TYPE_NAME;
	}

	//-----------------------------------------------------------------------------
	String ProceduralManualObjectFactory::FACTORY_TYPE_NAME = "ProceduralManualObject";
	//-----------------------------------------------------------------------------
	const String& ProceduralManualObjectFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------------
	MovableObject* ProceduralManualObjectFactory::createInstanceImpl(
		const String& name, const NameValuePairList* params)
	{
		return new ProceduralManualObject(name);
	}
	//-----------------------------------------------------------------------------
	void ProceduralManualObjectFactory::destroyInstance( MovableObject* obj)
	{
		delete obj;
	}
}