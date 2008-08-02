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

#include "OgreGLRenderToVertexBufferObject.h"
#include "OgreHardwareBufferManager.h"
#include "OgreGLHardwareVertexBuffer.h"
#include "OgreRenderable.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
//-----------------------------------------------------------------------------
	static GLint renderOperationTypeToGLGeometryPrimitiveType(RenderOperation::OperationType operationType)
	{
		switch (operationType)
		{
		case RenderOperation::OT_POINT_LIST:
			return GL_POINTS;
		case RenderOperation::OT_LINE_LIST:
			return GL_LINES;
		case RenderOperation::OT_LINE_STRIP:
			return GL_LINE_STRIP;
		default:
		case RenderOperation::OT_TRIANGLE_LIST:
			return GL_TRIANGLES;
		case RenderOperation::OT_TRIANGLE_STRIP:
			return GL_TRIANGLE_STRIP;
		case RenderOperation::OT_TRIANGLE_FAN:
			return GL_TRIANGLE_FAN;
		}
	}
//-----------------------------------------------------------------------------
	static GLint getVertexCountPerPrimitive(RenderOperation::OperationType operationType)
	{
		//Not fully correct, but does the job
		switch (operationType)
		{
		case RenderOperation::OT_POINT_LIST:
			return 1;
		case RenderOperation::OT_LINE_LIST:
		case RenderOperation::OT_LINE_STRIP:
			return 2;
		default:
		case RenderOperation::OT_TRIANGLE_LIST:
		case RenderOperation::OT_TRIANGLE_STRIP:
		case RenderOperation::OT_TRIANGLE_FAN:
			return 3;
		}
	}
//-----------------------------------------------------------------------------
	void checkGLError()
	{
		String msg;
		bool foundError = false;

		// get all the GL errors
		GLenum glErr = glGetError();
		while (glErr != GL_NO_ERROR)
        {
			const char* glerrStr = (const char*)gluErrorString(glErr);
			if (glerrStr)
			{
				msg += String(glerrStr);
			}
			glErr = glGetError();
			foundError = true;	
        }

		if (foundError)
		{
			bool debug = true;
		}
	}
//-----------------------------------------------------------------------------
	GLRenderToVertexBufferObject::GLRenderToVertexBufferObject()
	{
		mVertexBuffer.setNull();

		 // create query objects
		glGenQueries(1, &mPrimitivesDrawnQuery);
	}
//-----------------------------------------------------------------------------
	GLRenderToVertexBufferObject::~GLRenderToVertexBufferObject()
	{
		glDeleteQueries(1, &mPrimitivesDrawnQuery);
		//TODO : Implement
	}
//-----------------------------------------------------------------------------
	void GLRenderToVertexBufferObject::getRenderOperation(RenderOperation& op)
	{
		op.operationType = mOperationType;
		op.useIndexes = false;
		op.vertexData = mVertexData;
	}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	void GLRenderToVertexBufferObject::update(SceneManager* sceneMgr)
	{
		size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
		if (mVertexBuffer.isNull() || mVertexBuffer->getSizeInBytes() != bufSize)
		{
			reallocateBuffer();
			mResetRequested = true;
		}
		
		RenderOperation renderOp;
		mSourceRenderable->getRenderOperation(renderOp);
		bindVerticesOutput();

		checkGLError();

		GLHardwareVertexBuffer* vertexBuffer = static_cast<GLHardwareVertexBuffer*>(mVertexBuffer.getPointer());
		GLuint bufferId = vertexBuffer->getGLBufferId();

		checkGLError();

		//Bind the target buffer
		glBindBufferOffsetNV(GL_TRANSFORM_FEEDBACK_BUFFER_NV, 0, bufferId, 0);

		glBeginTransformFeedbackNV(renderOperationTypeToGLGeometryPrimitiveType(mOperationType));
		glEnable(GL_RASTERIZER_DISCARD_NV);    // disable rasterization

		checkGLError();

		//Single pass only for now
		sceneMgr->_setPass(mMaterial->getBestTechnique()->getPass(0));
		
		checkGLError();

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV, mPrimitivesDrawnQuery);

		checkGLError();

		//Draw the object
		Root::getSingleton().getRenderSystem()->_render(renderOp);
		
		checkGLError();

		//Finish the query
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV);
		glDisable(GL_RASTERIZER_DISCARD_NV);
		glEndTransformFeedbackNV();

		//read back query results
		GLuint primitivesWritten;
		glGetQueryObjectuiv(mPrimitivesDrawnQuery, GL_QUERY_RESULT, &primitivesWritten);
		mVertexData->vertexCount = primitivesWritten * getVertexCountPerPrimitive(mOperationType);

		checkGLError();
	}
//-----------------------------------------------------------------------------
	void GLRenderToVertexBufferObject::reallocateBuffer()
	{
		if (!mVertexBuffer.isNull())
		{
			mVertexBuffer.setNull();
		}		
		mVertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
			mVertexData->vertexDeclaration->getVertexSize(0), mMaxVertexCount, HardwareBuffer::HBU_DYNAMIC);
		
		mVertexData->vertexBufferBinding->unsetAllBindings();
		mVertexData->vertexBufferBinding->setBinding(0, mVertexBuffer);
	}
//-----------------------------------------------------------------------------
	void GLRenderToVertexBufferObject::bindVerticesOutput()
	{
		// specify which attributes to store
		GLint attribs[] = { GL_POSITION, 4, 0 };
		glTransformFeedbackAttribsNV(1, attribs, GL_SEPARATE_ATTRIBS_NV);
		//TODO : Implement real
	}
}