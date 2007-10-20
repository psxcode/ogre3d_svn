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
#include "OgreD3D10HardwareIndexBuffer.h"
#include "OgreD3D10Mappings.h"
#include "OgreException.h"
#include "OgreD3D10HardwareBufferManager.h"

namespace Ogre {

	//---------------------------------------------------------------------
    D3D10HardwareIndexBuffer::D3D10HardwareIndexBuffer(HardwareIndexBuffer::IndexType idxType, 
        size_t numIndexes, HardwareBuffer::Usage usage, ID3D10Device * pDev, 
        bool useSystemMemory, bool useShadowBuffer)
        : HardwareIndexBuffer(idxType, numIndexes, usage, useSystemMemory, useShadowBuffer)
    {
		/*
#if OGRE_D3D_MANAGE_BUFFERS
		mD3DPool = useSystemMemory? D3DPOOL_SYSTEMMEM : 
			// If not system mem, use managed pool UNLESS buffer is discardable
			// if discardable, keeping the software backing is expensive
			(usage & HardwareBuffer::HBU_DISCARDABLE)? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
#else
		mD3DPool = useSystemMemory? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;
#endif
        // Create the Index buffer
        HRESULT hr = pDev->CreateIndexBuffer(
            static_cast<UINT>(mSizeInBytes),
            D3D10Mappings::get(mUsage),
            D3D10Mappings::get(mIndexType),
			mD3DPool,
            &mlpD3DBuffer,
            NULL
            );
            
        if (FAILED(hr))
        {
			String msg = DXGetErrorDescription9(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot create D3D10 Index buffer: " + msg, 
                "D3D10HardwareIndexBuffer::D3D10HardwareIndexBuffer");
        }
*/
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10Mappings::get(usage);
		bd.ByteWidth = (UINT)mSizeInBytes;
		bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags =D3D10_CPU_ACCESS_WRITE;
		
		if(!(usage & HBU_WRITE_ONLY 
			|| usage & HBU_STATIC_WRITE_ONLY 
			|| usage & HBU_DYNAMIC_WRITE_ONLY
			|| usage & HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE ))
		{
			bd.CPUAccessFlags =D3D10_CPU_ACCESS_READ ;
		}

		bd.MiscFlags = 0;
		HRESULT hr = pDev->CreateBuffer( &bd, 0, &mlpD3DBuffer );
		if( FAILED(hr) )
		{
			String msg = DXGetErrorDescription(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot create D3D10 vertex buffer: " + msg, 
                "D3D10HardwareVertexBuffer::D3D10HardwareVertexBuffer");
       }
    }
	//---------------------------------------------------------------------
    D3D10HardwareIndexBuffer::~D3D10HardwareIndexBuffer()
    {
        SAFE_RELEASE(mlpD3DBuffer);
    }
	//---------------------------------------------------------------------
    void* D3D10HardwareIndexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
        char* pBuf;
		// TODO deal with locking modes not supported by dx10 because of initial usage
		// use temporary staging buffer
        HRESULT hr = mlpD3DBuffer->Map(
           D3D10Mappings::get(options, mUsage),
			0,
			(void**)&pBuf 
		);

        if (FAILED(hr))
        {
			String msg = DXGetErrorDescription(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot lock D3D10 Index buffer: " + msg, 
                "D3D10HardwareIndexBuffer::lock");
        }
		pBuf+=offset;

        return pBuf;


    }
	//---------------------------------------------------------------------
	void D3D10HardwareIndexBuffer::unlockImpl(void)
    {
        mlpD3DBuffer->Unmap();
    }
	//---------------------------------------------------------------------
    void D3D10HardwareIndexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
       // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pSrc = this->lock(offset, length, HardwareBuffer::HBL_READ_ONLY);
        memcpy(pDest, pSrc, length);
        this->unlock();

    }
	//---------------------------------------------------------------------
    void D3D10HardwareIndexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource,
			bool discardWholeBuffer)
    {
       // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pDst = this->lock(offset, length, 
            discardWholeBuffer ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
        memcpy(pDst, pSource, length);
        this->unlock();    }
	//---------------------------------------------------------------------
	bool D3D10HardwareIndexBuffer::releaseIfDefaultPool(void)
	{
/*		if (mD3DPool == D3DPOOL_DEFAULT)
		{
			SAFE_RELEASE(mlpD3DBuffer);
			return true;
		}
		return false;
*/
		return true;
	}
	//---------------------------------------------------------------------
	bool D3D10HardwareIndexBuffer::recreateIfDefaultPool(ID3D10Device * pDev)
	{
	/*	if (mD3DPool == D3DPOOL_DEFAULT)
		{
			// Create the Index buffer
			HRESULT hr = pDev->CreateIndexBuffer(
				static_cast<UINT>(mSizeInBytes),
				D3D10Mappings::get(mUsage),
				D3D10Mappings::get(mIndexType),
				mD3DPool,
				&mlpD3DBuffer,
				NULL
				);

			if (FAILED(hr))
			{
				String msg = DXGetErrorDescription9(hr);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot create D3D10 Index buffer: " + msg, 
					"D3D10HardwareIndexBuffer::D3D10HardwareIndexBuffer");
			}

			return true;
		}
		return false;
	*/
		return true;
	}
	//---------------------------------------------------------------------

}