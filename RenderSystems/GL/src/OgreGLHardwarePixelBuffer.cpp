/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2005 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#include "OgreGLHardwarePixelBuffer.h"
#include "OgreGLTexture.h"
#include "OgreGLSupport.h"
#include "OgreGLPixelFormat.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

namespace Ogre {
//----------------------------------------------------------------------------- 
GLHardwarePixelBuffer::GLHardwarePixelBuffer(GLenum target, GLuint id, GLint face, GLint level, Usage usage, bool crappyCard):
	HardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, usage, false, false),
	mType(TYPE_TEXTURE),
	mTarget(target), mTextureID(id), mFace(face), mLevel(level), mSoftwareMipmap(crappyCard)
{
	// devise mWidth, mHeight and mDepth and mFormat
	GLint value;
	
	glBindTexture( mTarget, mTextureID );
	
	// Get width
	glGetTexLevelParameteriv(mTarget, level, GL_TEXTURE_WIDTH, &value);
	mWidth = value;
	
	// Get height
	if(target == GL_TEXTURE_1D)
		value = 1;	// Height always 1 for 1D textures
	else
		glGetTexLevelParameteriv(mTarget, level, GL_TEXTURE_HEIGHT, &value);
	mHeight = value;
	
	// Get depth
	if(target != GL_TEXTURE_3D)
		value = 1; // Depth always 1 for non-3D textures
	else
		glGetTexLevelParameteriv(mTarget, level, GL_TEXTURE_DEPTH, &value);
	mDepth = value;
	
	// Get format
	glGetTexLevelParameteriv(mTarget, level, GL_TEXTURE_INTERNAL_FORMAT, &value);
	int mGLInternalFormat = value;
	mFormat = GLPixelUtil::getClosestOGREFormat(value);
	
	// Default
	mRowPitch = mWidth;
	mSlicePitch = mHeight*mWidth;
	mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
	
	// Log a message
	/*
	std::stringstream str;
	str << "GLHardwarePixelBuffer constructed for texture " << mTextureID 
		<< " face " << mFace << " level " << mLevel << ": "
		<< "width=" << mWidth << " height="<< mHeight << " depth=" << mDepth
		<< "format=" << PixelUtil::getFormatName(mFormat) << "(internal 0x"
		<< std::hex << value << ")";
	LogManager::getSingleton().logMessage( 
                LML_NORMAL, str.str());
	*/
	// Set up pixel box
	mBuffer = PixelBox(mWidth, mHeight, mDepth, mFormat);
				
	// Allocate buffer
	if(mUsage != HBU_STATIC)
		allocateBuffer();
}

//-----------------------------------------------------------------------------  
GLHardwarePixelBuffer::~GLHardwarePixelBuffer()
{
	freeBuffer();
	// Emit message
	/*
	std::stringstream str;
	str << "GLHardwarePixelBuffer destructed for texture " << mTextureID 
		<< " face " << mFace << " level " << mLevel;
	LogManager::getSingleton().logMessage( 
                LML_NORMAL, str.str());
	*/
	
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::allocateBuffer()
{
	if(mBuffer.data)
		// Already allocated
		return;
	mBuffer.data = new uint8[mSizeInBytes];
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::freeBuffer()
{
	delete [] (uint8*)mBuffer.data;
	mBuffer.data = 0;
}
//-----------------------------------------------------------------------------  
PixelBox GLHardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
{
	allocateBuffer();
	return mBuffer.getSubVolume(lockBox);
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::unlockImpl(void)
{
	// From buffer to card
	upload(mCurrentLock);
	
	// deallocate or keep buffer depending on usage
	if(mUsage == HBU_STATIC)
		freeBuffer();
}
//-----------------------------------------------------------------------------
void GLHardwarePixelBuffer::upload(PixelBox &data)
{
	glBindTexture( mTarget, mTextureID );
	if(PixelUtil::isCompressed(mFormat))
	{
		GLenum format = GLPixelUtil::getClosestGLInternalFormat(mFormat);
		// Data must be consecutive and at beginning of buffer as PixelStorei not allowed
		// for compressed formate
		switch(mTarget) {
			case GL_TEXTURE_1D:
				glCompressedTexSubImage1DARB_ptr(GL_TEXTURE_1D, mLevel, 
					data.left,
					data.getWidth(),
					format, data.getConsecutiveSize(),
					data.data);
				break;
			case GL_TEXTURE_2D:
				glCompressedTexSubImage2DARB_ptr(GL_TEXTURE_2D, mLevel, 
					data.left, data.top, 
					data.getWidth(), data.getHeight(),
					format, data.getConsecutiveSize(),
					data.data);
				break;
			case GL_TEXTURE_3D:
				glCompressedTexSubImage3DARB_ptr(GL_TEXTURE_3D, mLevel, 
					data.left, data.top, data.front,
					data.getWidth(), data.getHeight(), data.getDepth(),
					format, data.getConsecutiveSize(),
					data.data);
				break;
			case GL_TEXTURE_CUBE_MAP:
				glCompressedTexSubImage2DARB_ptr(GL_TEXTURE_CUBE_MAP_POSITIVE_X + mFace, mLevel, 
					data.left, data.top, 
					data.getWidth(), data.getHeight(),
					format, data.getConsecutiveSize(),
					data.data);
				break;
		}
		
	} 
	else if(mSoftwareMipmap)
	{
		GLint internalFormat;
		glGetTexLevelParameteriv(mTarget, mLevel, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
		if(data.getWidth() != data.rowPitch)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, data.rowPitch);
		if(data.getHeight()*data.getWidth() != data.slicePitch)
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		switch(mTarget)
		{
		case GL_TEXTURE_1D:
			gluBuild1DMipmaps(
				GL_TEXTURE_1D, internalFormat,
				data.getWidth(),
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
				data.data);
			break;
		case GL_TEXTURE_3D:
			/* Requires GLU 1.3 which is harder to come by than cards doing hardware mipmapping
				Most 3D textures don't need mipmaps?
			gluBuild3DMipmaps(
				GL_TEXTURE_3D, internalFormat, 
				data.getWidth(), data.getHeight(), data.getDepth(),
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
				data.data);
			*/
			glTexImage3D(
				GL_TEXTURE_3D, 0, internalFormat, 
				data.getWidth(), data.getHeight(), data.getDepth(), 0, 
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
				data.data );
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:
			gluBuild2DMipmaps(
				mTarget==GL_TEXTURE_CUBE_MAP? 
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + mFace : 
					GL_TEXTURE_2D,
				internalFormat, data.getWidth(), data.getHeight(), 
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format), 
				data.data);
			break;
		}
	} 
	else
	{
		if(data.getWidth() != data.rowPitch)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, data.rowPitch);
		if(data.getHeight()*data.getWidth() != data.slicePitch)
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
		if((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3) {
			// Standard alignment of 4 is not right
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
		switch(mTarget) {
			case GL_TEXTURE_1D:
				glTexSubImage1D(GL_TEXTURE_1D, mLevel, 
					data.left,
					data.getWidth(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
			case GL_TEXTURE_2D:
				glTexSubImage2D(GL_TEXTURE_2D, mLevel, 
					data.left, data.top, 
					data.getWidth(), data.getHeight(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
			case GL_TEXTURE_3D:
				glTexSubImage3DEXT(GL_TEXTURE_3D, mLevel, 
					data.left, data.top, data.front,
					data.getWidth(), data.getHeight(), data.getDepth(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
			case GL_TEXTURE_CUBE_MAP:
				glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + mFace, mLevel, 
					data.left, data.top, 
					data.getWidth(), data.getHeight(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
		}	
	}
	// Restore defaults
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::download(PixelBox &data)
{
	// TODO
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blit(HardwarePixelBuffer *src, const Image::Box &srcBox, const Image::Box &dstBox)
{
	// TODO
	// this can be speeded up with some copy pixels primitive, sometimes
	HardwarePixelBuffer::blit(src, srcBox, dstBox);

}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
{
	if(!mBuffer.contains(dstBox))
		Except(Exception::ERR_INVALIDPARAMS, "destination box out of range",
		 "GLHardwarePixelBuffer::blitFromMemory");
	PixelBox scaled;
	// for scoped deletion of conversion buffer
	MemoryDataStreamPtr buf;
	
	if(src.getWidth() != dstBox.getWidth() ||
		src.getHeight() != dstBox.getHeight() ||
		src.getDepth() != dstBox.getDepth())
	{
		// Scale to destination size. Use DevIL and not iluScale because ILU screws up for 
		// floating point textures and cannot cope with 3D images.
		// This also does pixel format conversion if needed
		buf.bind(new MemoryDataStream(
			PixelUtil::getMemorySize(dstBox.getWidth(), 
				dstBox.getHeight(), dstBox.getDepth(),
				mFormat)));
		scaled = PixelBox(dstBox, mFormat, buf->getPtr());
		Image::scale(src, scaled, Image::FILTER_BILINEAR);
	}
	else if(GLPixelUtil::getGLOriginFormat(src.format) == 0 ||
			GLPixelUtil::getGLOriginDataType(src.format) == 0)
	{
		// Extents match, but format is not accepted as valid source format for GL
		// do conversion in temporary buffer
		buf.bind(new MemoryDataStream(
			PixelUtil::getMemorySize(dstBox.getWidth(), 
				dstBox.getHeight(), dstBox.getDepth(),
				mFormat)));
		scaled = PixelBox(dstBox, mFormat, buf->getPtr());
		PixelUtil::bulkPixelConversion(src, scaled);
	}
	else
	{
		// No scaling or conversion needed
		scaled = src;
		// Set extents for upload
		scaled.left = dstBox.left;
		scaled.right = dstBox.right;
		scaled.top = dstBox.top;
		scaled.bottom = dstBox.bottom;
		scaled.front = dstBox.front;
		scaled.back = dstBox.back;
	}
	
	upload(scaled);
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
{
	// TODO
	// glGetTexImageXD
}


};
