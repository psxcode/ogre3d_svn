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
#include "OgreGLSLLinkProgramManager.h"
#include "OgreStringConverter.h"

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
		
		//Single pass only for now
		Ogre::Pass* r2vbPass = mMaterial->getBestTechnique()->getPass(0);
		sceneMgr->_setPass(r2vbPass);
		bindVerticesOutput(r2vbPass);

		RenderOperation renderOp;
		mSourceRenderable->getRenderOperation(renderOp);
		

		checkGLError();

		GLHardwareVertexBuffer* vertexBuffer = static_cast<GLHardwareVertexBuffer*>(mVertexBuffer.getPointer());
		GLuint bufferId = vertexBuffer->getGLBufferId();

		checkGLError();

		//Bind the target buffer
		glBindBufferOffsetNV(GL_TRANSFORM_FEEDBACK_BUFFER_NV, 0, bufferId, 0);

		glBeginTransformFeedbackNV(renderOperationTypeToGLGeometryPrimitiveType(mOperationType));
		glEnable(GL_RASTERIZER_DISCARD_NV);    // disable rasterization

		checkGLError();

		
		
		checkGLError();

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV, mPrimitivesDrawnQuery);

		checkGLError();

		RenderSystem* targetRenderSystem = Root::getSingleton().getRenderSystem();
		//Draw the object
		targetRenderSystem->_setWorldMatrix(Matrix4::IDENTITY);
		targetRenderSystem->_setViewMatrix(Matrix4::IDENTITY);
		targetRenderSystem->_setProjectionMatrix(Matrix4::IDENTITY);
		targetRenderSystem->_render(renderOp);
		
		checkGLError();

		//Finish the query
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV);
		glDisable(GL_RASTERIZER_DISCARD_NV);
		glEndTransformFeedbackNV();

		//read back query results
		GLuint primitivesWritten;
		glGetQueryObjectuiv(mPrimitivesDrawnQuery, GL_QUERY_RESULT, &primitivesWritten);
		mVertexData->vertexCount = primitivesWritten * getVertexCountPerPrimitive(mOperationType);

		//void* buffData = mVertexBuffer->lock(HardwareBuffer::HBL_READ_ONLY);
		//float* buffFloat = static_cast<float*>(buffData);
		//mVertexBuffer->unlock();

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
	String GLRenderToVertexBufferObject::getSemanticVaryingName(VertexElementSemantic semantic, unsigned short index)
	{
		String result;
		switch (semantic)
		{
		case VES_POSITION:
			result = "gl_Position";
		case VES_TEXTURE_COORDINATES:
			result = String("gl_TexCoord[") + StringConverter::toString(index) + "]";
		case VES_DIFFUSE:
			result = "gl_Color";
		//TODO : Implement more
		default:
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Unsupported vertex element sematic in render to vertex buffer", 
				"OgreGLRenderToVertexBufferObject::getSemanticVaryingName");
		}

		return result;
	}
//-----------------------------------------------------------------------------
	GLint GLRenderToVertexBufferObject::getGLSemanticType(VertexElementSemantic semantic)
	{
		switch (semantic)
		{
		case VES_POSITION:
			return GL_POSITION;
		case VES_TEXTURE_COORDINATES:
			return GL_TEXTURE_COORD_NV;
		case VES_DIFFUSE:
			return GL_PRIMARY_COLOR;
		//TODO : Implement more
		default:
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Unsupported vertex element sematic in render to vertex buffer", 
				"OgreGLRenderToVertexBufferObject::getGLSemanticType");
			
		}
	}
//-----------------------------------------------------------------------------
	void GLRenderToVertexBufferObject::bindVerticesOutput(Pass* pass)
	{
		VertexDeclaration* declaration = mVertexData->vertexDeclaration;
		bool useVaryingAttributes = false;
		
		//Check if we are FixedFunc/ASM shaders (Static attributes) or GLSL (Varying attributes)
		//We assume that there isn't a mix of GLSL and ASM as this is illegal
		GpuProgram* sampleProgram = 0;
		if (pass->hasVertexProgram())
		{
			sampleProgram = pass->getVertexProgram().getPointer();
		}
		else if (pass->hasGeometryProgram())
		{
			sampleProgram = pass->getGeometryProgram().getPointer();
		}
		if ((sampleProgram != 0) && (sampleProgram->getLanguage() == "glsl"))
		{
			useVaryingAttributes = true;
		}

		if (useVaryingAttributes)
		{
			//Have GLSL shaders, using varying attributes
			GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
			GLhandleARB linkProgramId = linkProgram->getGLHandle();
			
			std::vector<GLint> locations;
			for (unsigned short e=0; e < declaration->getElementCount(); e++)
			{
				const VertexElement* element =declaration->getElement(e);
				String varyingName = getSemanticVaryingName(element->getSemantic(), element->getIndex());
				GLint location = glGetVaryingLocationNV(linkProgramId, varyingName.c_str());
				locations.push_back(location);
			}
			glTransformFeedbackVaryingsNV(linkProgramId, locations.size(), &locations[0], GL_SEPARATE_ATTRIBS_NV);
		}
		else
		{
			//Either fixed function or assembly (CG = assembly) shaders
			std::vector<GLint> attribs;
			for (unsigned short e=0; e < declaration->getElementCount(); e++)
			{
				const VertexElement* element = declaration->getElement(e);
				//Type
				attribs.push_back(getGLSemanticType(element->getSemantic()));
				//Number of components
				attribs.push_back(VertexElement::getTypeCount(element->getType()));
				//Index
				attribs.push_back(element->getIndex());
			}
			
			glTransformFeedbackAttribsNV(declaration->getElementCount(), &attribs[0] ,GL_SEPARATE_ATTRIBS_NV);
		}
	}
}