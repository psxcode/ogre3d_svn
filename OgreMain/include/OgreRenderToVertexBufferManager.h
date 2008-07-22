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
#ifndef __RenderToVertexBufferManager_H__
#define __RenderToVertexBufferManager_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreRenderToVertexBufferObject.h"

namespace Ogre {
	
	/** Shared pointer implementation used to have the destruction callback the manager. */
	class RenderToVertexBufferObjectSharedPtr : public SharedPtr<RenderToVertexBufferObject>
	{
		RenderToVertexBufferObjectSharedPtr() : SharedPtr<RenderToVertexBufferObject>() {}
		virtual ~RenderToVertexBufferObjectSharedPtr();
	};

	class RenderToVertexBufferManager : public Singleton<RenderToVertexBufferManager>
	{
		friend class RenderToVertexBufferObjectSharedPtr;
	public:
		/** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static GpuProgramManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static GpuProgramManager* getSingletonPtr(void);

		/**
			Create a render to vertex buffer object.
		*/
		virtual RenderToVertexBufferObjectSharedPtr createRenderToVertexBufferObject() = 0;

		/**
			Update all the RenderToVertexBufferObjects which have signed up to
			automatically update themselves.
		*/
		void updateAutoUpdatedObjects();
	protected:
		/**
			Unregister a render to vertex buffer object from the manager.
			@param deadObject the object about to be destroyed
		*/
		virtual unregisterRenderToVertexBufferObject(RenderToVertexBufferObject* deadObject);
	};
}

#endif