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

#ifndef __PROCEDURAL_MANUAL_OBJECT_H__
#define __PROCEDURAL_MANUAL_OBJECT_H__

#include <OgreManualObject.h>
#include <OgreRenderToVertexBufferObject.h>

namespace Ogre
{
	class ProceduralManualObject : public ManualObject
	{
	public:
		ProceduralManualObject(const String& name);
		virtual ~ProceduralManualObject();

		void setRenderToVertexBufferObject(RenderToVertexBufferObjectSharedPtr r2vbObject);
		const RenderToVertexBufferObjectSharedPtr& getRenderToVertexBufferObject();

		/** @copydoc ManualObject::_updateRenderQueue. */
		void _updateRenderQueue(RenderQueue* queue);
		/** @copydoc ManualObject::getMovableType. */
		const String& getMovableType(void) const;
	};

	class ProceduralManualObjectFactory : public MovableObjectFactory
	{
	protected:
			MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params);
		public:
			ProceduralManualObjectFactory() {}
			~ProceduralManualObjectFactory() {}

			static String FACTORY_TYPE_NAME;

			const String& getType(void) const;
			void destroyInstance( MovableObject* obj);  
	};

}
#endif