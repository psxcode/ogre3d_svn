/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
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
#include "OgreStableHeaders.h"

#include "OgreMeshSerializerImpl.h"
#include "OgreMeshFileFormat.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreException.h"
#include "OgreMaterialManager.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"
#include "OgreHardwareBufferManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

namespace Ogre {

    /// stream overhead = ID + size
    const unsigned long stream_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
    //---------------------------------------------------------------------
    MeshSerializerImpl::MeshSerializerImpl()
    {

        // Version number
        mVersion = "[MeshSerializer_v1.30]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl::~MeshSerializerImpl()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::exportMesh(const Mesh* pMesh, const String& filename)
    {
        LogManager::getSingleton().logMessage("MeshSerializer writing mesh data to " + filename + "...");

        // Check that the mesh has it's bounds set
        if (pMesh->getBounds().isNull() || pMesh->getBoundingSphereRadius() == 0.0f)
        {
            Except(Exception::ERR_INVALIDPARAMS, "The Mesh you have supplied does not have its"
                " bounds completely defined. Define them first before exporting.", 
                "MeshSerializerImpl::exportMesh");
        }
        mpfFile = fopen(filename.c_str(), "wb");

        writeFileHeader();
        LogManager::getSingleton().logMessage("File header written.");


        LogManager::getSingleton().logMessage("Writing mesh data...");
        writeMesh(pMesh);
        LogManager::getSingleton().logMessage("Mesh data exported.");

        fclose(mpfFile);
        LogManager::getSingleton().logMessage("MeshSerializer export successful.");
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::importMesh(DataStreamPtr& stream, Mesh* pMesh)
    {

        // Check header
        readFileHeader(stream);

        unsigned short streamID;
        while(!stream.eof())
        {
            streamID = readChunk(stream);
            switch (streamID)
            {
            case M_MESH:
                readMesh(stream, pMesh);
                break;
			}

        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeMesh(const Mesh* pMesh)
    {
        // Header
        writeChunkHeader(M_MESH, calcMeshSize(pMesh));

		// bool skeletallyAnimated
		bool skelAnim = pMesh->hasSkeleton();
		writeBools(&skelAnim, 1);

        // Write shared geometry
        if (pMesh->sharedVertexData)
            writeGeometry(pMesh->sharedVertexData);

        // Write Submeshes
        for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            LogManager::getSingleton().logMessage("Writing submesh...");
            writeSubMesh(pMesh->getSubMesh(i));
            LogManager::getSingleton().logMessage("Submesh exported.");
        }

        // Write skeleton info if required
        if (pMesh->hasSkeleton())
        {
            LogManager::getSingleton().logMessage("Exporting skeleton link...");
            // Write skeleton link
            writeSkeletonLink(pMesh->getSkeletonName());
            LogManager::getSingleton().logMessage("Skeleton link exported.");

            // Write bone assignments
            if (!pMesh->mBoneAssignments.empty())
            {
                LogManager::getSingleton().logMessage("Exporting shared geometry bone assignments...");

                Mesh::VertexBoneAssignmentList::const_iterator vi;
                for (vi = pMesh->mBoneAssignments.begin(); 
                vi != pMesh->mBoneAssignments.end(); ++vi)
                {
                    writeMeshBoneAssignment(&(vi->second));
                }

                LogManager::getSingleton().logMessage("Shared geometry bone assignments exported.");
            }
        }

        // Write LOD data if any
        if (pMesh->getNumLodLevels() > 1)
        {
            LogManager::getSingleton().logMessage("Exporting LOD information....");
            writeLodInfo(pMesh);
            LogManager::getSingleton().logMessage("LOD information exported.");
            
        }
        // Write bounds information
        LogManager::getSingleton().logMessage("Exporting bounds information....");
        writeBoundsInfo(pMesh);
        LogManager::getSingleton().logMessage("Bounds information exported.");

		// Write submesh name table
		LogManager::getSingleton().logMessage("Exporting submesh name table...");
		writeSubMeshNameTable(pMesh);
		LogManager::getSingleton().logMessage("Submesh name table exported.");
		
		// Write edge lists
		if (pMesh->isEdgeListBuilt())
		{
			LogManager::getSingleton().logMessage("Exporting edge lists...");
			writeEdgeList(pMesh);
			LogManager::getSingleton().logMessage("Edge lists exported");
		}

    }
    //---------------------------------------------------------------------
	// Added by DrEvil
	void MeshSerializerImpl::writeSubMeshNameTable(const Mesh* pMesh)
	{
		// Header
		writeChunkHeader(M_SUBMESH_NAME_TABLE, calcSubMeshNameTableSize(pMesh));

		// Loop through and save out the index and names.
		Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();

		while(it != pMesh->mSubMeshNameMap.end())
		{
			// Header
			writeChunkHeader(M_SUBMESH_NAME_TABLE_ELEMENT, stream_OVERHEAD_SIZE + 
				sizeof(unsigned short) + (unsigned long)it->first.length() + 1);

			// write the index
			writeShorts(&it->second, 1);
			// name
	        writeString(it->first);

			++it;
		}
	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMesh(const SubMesh* s)
    {
        // Header
        writeChunkHeader(M_SUBMESH, calcSubMeshSize(s));

        // char* materialName
        writeString(s->getMaterialName());

        // bool useSharedVertices
        writeBools(&s->useSharedVertices, 1);

		// unsigned int indexCount
        writeInts(&s->indexData->indexCount, 1);

        // bool indexes32Bit
        bool idx32bit = (s->indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);
        writeBools(&idx32bit, 1);

        // unsigned short* faceVertexIndices ((indexCount)
        HardwareIndexBufferSharedPtr ibuf = s->indexData->indexBuffer;
        void* pIdx = ibuf->lock(HardwareBuffer::HBL_READ_ONLY);
        if (idx32bit)
        {
            unsigned int* pIdx32 = static_cast<unsigned int*>(pIdx);
            writeInts(pIdx32, s->indexData->indexCount);
        }
        else
        {
            unsigned short* pIdx16 = static_cast<unsigned short*>(pIdx);
            writeShorts(pIdx16, s->indexData->indexCount);
        }
        ibuf->unlock();

        // M_GEOMETRY stream (Optional: present only if useSharedVertices = false)
        if (!s->useSharedVertices)
        {
            writeGeometry(s->vertexData);
        }
        
        // Operation type
        writeSubMeshOperation(s);

        // Bone assignments
        if (!s->mBoneAssignments.empty())
        {
            LogManager::getSingleton().logMessage("Exporting dedicated geometry bone assignments...");

            SubMesh::VertexBoneAssignmentList::const_iterator vi;
            for (vi = s->mBoneAssignments.begin(); 
            vi != s->mBoneAssignments.end(); ++vi)
            {
                writeSubMeshBoneAssignment(&(vi->second));
            }

            LogManager::getSingleton().logMessage("Dedicated geometry bone assignments exported.");
        }


    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMeshOperation(const SubMesh* sm)
    {
        // Header
        writeChunkHeader(M_SUBMESH_OPERATION, calcSubMeshOperationSize(sm));

        // unsigned short operationType
        unsigned short opType = static_cast<unsigned short>(sm->operationType);
        writeShorts(&opType, 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeGeometry(const VertexData* vertexData)
    {
		// calc size
        const VertexDeclaration::VertexElementList& elemList = 
            vertexData->vertexDeclaration->getElements();
        const VertexBufferBinding::VertexBufferBindingMap& bindings = 
            vertexData->vertexBufferBinding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator vbi, vbiend;

		size_t size = stream_OVERHEAD_SIZE + sizeof(unsigned int) + // base
			(stream_OVERHEAD_SIZE + elemList.size() * (stream_OVERHEAD_SIZE + sizeof(unsigned short) * 5)); // elements
        vbiend = bindings.end();
		for (vbi = bindings.begin(); vbi != vbiend; ++vbi)
		{
			const HardwareVertexBufferSharedPtr& vbuf = vbi->second;
			size += (stream_OVERHEAD_SIZE * 2) + (sizeof(unsigned short) * 2) + vbuf->getSizeInBytes();
		}

		// Header
        writeChunkHeader(M_GEOMETRY, size);

        // unsigned int numVertices
        writeInts(&vertexData->vertexCount, 1);

		// Vertex declaration
		size = stream_OVERHEAD_SIZE + elemList.size() * (stream_OVERHEAD_SIZE + sizeof(unsigned short) * 5);
		writeChunkHeader(M_GEOMETRY_VERTEX_DECLARATION, size);
		
        VertexDeclaration::VertexElementList::const_iterator vei, veiend;
		veiend = elemList.end();
		unsigned short tmp;
		size = stream_OVERHEAD_SIZE + sizeof(unsigned short) * 5;
		for (vei = elemList.begin(); vei != veiend; ++vei)
		{
			const VertexElement& elem = *vei;
			writeChunkHeader(M_GEOMETRY_VERTEX_ELEMENT, size);
			// unsigned short source;  	// buffer bind source
			tmp = elem.getSource();
			writeShorts(&tmp, 1);
			// unsigned short type;    	// VertexElementType
			tmp = static_cast<unsigned short>(elem.getType());
			writeShorts(&tmp, 1);
			// unsigned short semantic; // VertexElementSemantic
			tmp = static_cast<unsigned short>(elem.getSemantic());
			writeShorts(&tmp, 1);
			// unsigned short offset;	// start offset in buffer in bytes
			tmp = static_cast<unsigned short>(elem.getOffset());
			writeShorts(&tmp, 1);
			// unsigned short index;	// index of the semantic (for colours and texture coords)
			tmp = elem.getIndex();
			writeShorts(&tmp, 1);

		}

		// Buffers and bindings
		vbiend = bindings.end();
		for (vbi = bindings.begin(); vbi != vbiend; ++vbi)
		{
			const HardwareVertexBufferSharedPtr& vbuf = vbi->second;
			size = (stream_OVERHEAD_SIZE * 2) + (sizeof(unsigned short) * 2) + vbuf->getSizeInBytes();
			writeChunkHeader(M_GEOMETRY_VERTEX_BUFFER,  size);
			// unsigned short bindIndex;	// Index to bind this buffer to
			tmp = vbi->first;
			writeShorts(&tmp, 1);
			// unsigned short vertexSize;	// Per-vertex size, must agree with declaration at this index
			tmp = (unsigned short)vbuf->getVertexSize();
			writeShorts(&tmp, 1);
			
			// Data
			size = stream_OVERHEAD_SIZE + vbuf->getSizeInBytes();
			writeChunkHeader(M_GEOMETRY_VERTEX_BUFFER_DATA, size);
			void* pBuf = vbuf->lock(HardwareBuffer::HBL_READ_ONLY);
#		if OGRE_ENDIAN == ENDIAN_BIG
			// endian conversion for OSX
			// Copy data
			unsigned char* tempData = new unsigned char[vbuf->getSizeInBytes()];
			memcpy(tempData, pBuf, vbuf->getSizeInBytes());
			flipToLittleEndian(
				tempData, 
				vertexData->vertexCount,
				vbuf->getVertexSize(),
				vertexData->vertexDeclaration->findElementsBySource(vbi->first));
			writeData(tempData, vbuf->getVertexSize(), vertexData->vertexCount);
			delete [] tempData;
#		else
			writeData(pBuf, vbuf->getVertexSize(), vertexData->vertexCount);
#		endif
		}


    }
    //---------------------------------------------------------------------
	unsigned long MeshSerializerImpl::calcSubMeshNameTableSize(const Mesh* pMesh)
	{
		size_t size = stream_OVERHEAD_SIZE;
		// Figure out the size of the Name table.
		// Iterate through the subMeshList & add up the size of the indexes and names.
		Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();		
		while(it != pMesh->mSubMeshNameMap.end())
		{
			// size of the index
			size += sizeof(unsigned short);
			// name
			size += (unsigned long)it->first.length() + 1;

			++it;
		}        		

		// size of the sub-mesh name table.
		return (unsigned long)size;
	}
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcMeshSize(const Mesh* pMesh)
    {
        unsigned long size = stream_OVERHEAD_SIZE;

        // Num shared vertices
        size += sizeof(unsigned int);

        // Geometry
        if (pMesh->sharedVertexData && pMesh->sharedVertexData->vertexCount > 0)
        {
            size += calcGeometrySize(pMesh->sharedVertexData);
        }

        // Submeshes
        for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            size += calcSubMeshSize(pMesh->getSubMesh(i));
        }

        // Skeleton link
        if (pMesh->hasSkeleton())
        {
            size += calcSkeletonLinkSize(pMesh->getSkeletonName());
        }

		// Submesh name table
		size += calcSubMeshNameTableSize(pMesh);

		// Edge list
		if (pMesh->isEdgeListBuilt())
		{
			size += calcEdgeListSize(pMesh);
		}
		
		return size;

    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcSubMeshSize(const SubMesh* pSub)
    {
        size_t size = stream_OVERHEAD_SIZE;

        // Material name
        size += (unsigned long)pSub->getMaterialName().length() + 1;

        // bool useSharedVertices
        size += sizeof(bool);
        // unsigned int indexCount
        size += sizeof(unsigned int);
        // bool indexes32bit
        size += sizeof(bool);
        // unsigned int* faceVertexIndices 
        size += sizeof(unsigned int) * pSub->indexData->indexCount;

        // Geometry
        if (!pSub->useSharedVertices)
        {
            size += calcGeometrySize(pSub->vertexData);
        }

        return static_cast<unsigned long>(size);
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcSubMeshOperationSize(const SubMesh* pSub)
    {
        return stream_OVERHEAD_SIZE + sizeof(unsigned short);
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcGeometrySize(const VertexData* vertexData)
    {
        size_t size = stream_OVERHEAD_SIZE;

        // Num vertices
        size += sizeof(unsigned int);

        const VertexDeclaration::VertexElementList& elems = 
            vertexData->vertexDeclaration->getElements();

        VertexDeclaration::VertexElementList::const_iterator i, iend;
        iend = elems.end();
        for (i = elems.begin(); i != iend; ++i)
        {
            const VertexElement& elem = *i;
            // Vertex element
            size += VertexElement::getTypeSize(elem.getType()) * vertexData->vertexCount;
        }
        return static_cast<unsigned long>(size);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometry(DataStreamPtr& stream, Mesh* pMesh, 
        VertexData* dest)
    {

        dest->vertexStart = 0;

        // unsigned int numVertices
        readInts(stream, &dest->vertexCount, 1);

        // Find optional geometry streams
        if (!stream.eof())
        {
            unsigned short streamID = readChunk(stream);
            while(!stream.eof() && 
                (streamID == M_GEOMETRY_VERTEX_DECLARATION || 
                 streamID == M_GEOMETRY_VERTEX_BUFFER ))
            {
                switch (streamID)
                {
                case M_GEOMETRY_VERTEX_DECLARATION:
                    readGeometryVertexDeclaration(stream, pMesh, dest);
                    break;
                case M_GEOMETRY_VERTEX_BUFFER:
                    readGeometryVertexBuffer(stream, pMesh, dest);
                    break;
                }
                // Get next stream
                if (!stream.eof())
                {
                    streamID = readChunk(stream);
                }
            }
            if (!stream.eof())
            {
                // Backpedal back to start of non-submesh stream
                stream.skip(-(long)stream_OVERHEAD_SIZE);
            }
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometryVertexDeclaration(DataStreamPtr& stream, 
        Mesh* pMesh, VertexData* dest)
    {
        // Find optional geometry streams
        if (!stream.eof())
        {
            unsigned short streamID = readChunk(stream);
            while(!stream.eof() && 
                (streamID == M_GEOMETRY_VERTEX_ELEMENT ))
            {
                switch (streamID)
                {
                case M_GEOMETRY_VERTEX_ELEMENT:
                    readGeometryVertexElement(stream, pMesh, dest);
                    break;
                }
                // Get next stream
                if (!stream.eof())
                {
                    streamID = readChunk(stream);
                }
            }
            if (!stream.eof())
            {
                // Backpedal back to start of non-submesh stream
                stream.skip(-(long)stream_OVERHEAD_SIZE);
            }
        }
		
	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometryVertexElement(DataStreamPtr& stream, 
        Mesh* pMesh, VertexData* dest)
    {
		unsigned short source, offset, index, tmp;
		VertexElementType vType;
		VertexElementSemantic vSemantic;
		// unsigned short source;  	// buffer bind source
		readShorts(stream, &source, 1);
		// unsigned short type;    	// VertexElementType
		readShorts(stream, &tmp, 1);
		vType = static_cast<VertexElementType>(tmp);
		// unsigned short semantic; // VertexElementSemantic
		readShorts(stream, &tmp, 1);
		vSemantic = static_cast<VertexElementSemantic>(tmp);
		// unsigned short offset;	// start offset in buffer in bytes
		readShorts(stream, &offset, 1);
		// unsigned short index;	// index of the semantic
		readShorts(stream, &index, 1);

		dest->vertexDeclaration->addElement(source, offset, vType, vSemantic, index);

	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometryVertexBuffer(DataStreamPtr& stream, 
        Mesh* pMesh, VertexData* dest)
    {
		unsigned short bindIndex, vertexSize;
		// unsigned short bindIndex;	// Index to bind this buffer to
		readShorts(stream, &bindIndex, 1);
		// unsigned short vertexSize;	// Per-vertex size, must agree with declaration at this index
		readShorts(stream, &vertexSize, 1);

		// Check for vertex data header
		unsigned short headerID;
		headerID = readChunk(stream);
		if (headerID != M_GEOMETRY_VERTEX_BUFFER_DATA)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, "Can't find vertex buffer data area",
            	"MeshSerializerImpl::readGeometryVertexBuffer");
		}
		// Check that vertex size agrees
		if (dest->vertexDeclaration->getVertexSize(bindIndex) != vertexSize)
		{
			Except(Exception::ERR_INTERNAL_ERROR, "Buffer vertex size does not agree with vertex declaration",
            	"MeshSerializerImpl::readGeometryVertexBuffer");
		}
		
		// Create / populate vertex buffer
		HardwareVertexBufferSharedPtr vbuf;
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            vertexSize,
            dest->vertexCount,
            pMesh->mVertexBufferUsage, 
			pMesh->mVertexBufferShadowBuffer);
        void* pBuf = vbuf->lock(HardwareBuffer::HBL_DISCARD);
        stream.read(pBuf, dest->vertexCount * vertexSize);

		// endian conversion for OSX
		flipFromLittleEndian(
			pBuf, 
			dest->vertexCount, 
			vertexSize,
			dest->vertexDeclaration->findElementsBySource(bindIndex));
        vbuf->unlock();
		
		// Set binding
        dest->vertexBufferBinding->setBinding(bindIndex, vbuf);

	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readSubMeshNameTable(DataStreamPtr& stream, Mesh* pMesh)
	{
		// The map for
		std::map<unsigned short, String> subMeshNames;
		unsigned short streamID, subMeshIndex;

		// Need something to store the index, and the objects name
		// This table is a method that imported meshes can retain their naming
		// so that the names established in the modelling software can be used
		// to get the sub-meshes by name. The exporter must support exporting
		// the optional stream M_SUBMESH_NAME_TABLE.

        // Read in all the sub-streams. Each sub-stream should contain an index and Ogre::String for the name.
		if (!stream.eof())
		{
			streamID = readChunk(stream);
			while(!stream.eof() && (streamID == M_SUBMESH_NAME_TABLE_ELEMENT ))
			{
				// Read in the index of the submesh.
				readShorts(stream, &subMeshIndex, 1);
				// Read in the String and map it to its index.
				subMeshNames[subMeshIndex] = readString(stream);					

				// If we're not end of file get the next stream ID
				if (!stream.eof())
					streamID = readChunk(stream);
			}
			if (!stream.eof())
			{
				// Backpedal back to start of stream
				stream.skip(-(long)stream_OVERHEAD_SIZE);
			}
		}

		// Set all the submeshes names
		// ?

		// Loop through and save out the index and names.
		std::map<unsigned short, String>::const_iterator it = subMeshNames.begin();

		while(it != subMeshNames.end())
		{			
			// Name this submesh to the stored name.
			pMesh->nameSubMesh(it->second, it->first);
			++it;
		}



	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readMesh(DataStreamPtr& stream, Mesh* pMesh)
    {
        unsigned short streamID;

        // Never automatically build edge lists for this version
        // expect them in the file or not at all
        pMesh->mAutoBuildEdgeLists = false;

		// bool skeletallyAnimated
		readBools(stream, &mIsSkeletallyAnimated, 1);

        // Find all substreams 
        if (!stream.eof())
        {
            streamID = readChunk(stream);
            while(!stream.eof() &&
                (streamID == M_GEOMETRY ||
				 streamID == M_SUBMESH ||
                 streamID == M_MESH_SKELETON_LINK ||
                 streamID == M_MESH_BONE_ASSIGNMENT ||
				 streamID == M_MESH_LOD ||
                 streamID == M_MESH_BOUNDS ||
				 streamID == M_SUBMESH_NAME_TABLE ||
				 streamID == M_EDGE_LISTS))
            {
                switch(streamID)
                {
				case M_GEOMETRY:
					pMesh->sharedVertexData = new VertexData();
					try {
						readGeometry(stream, pMesh, pMesh->sharedVertexData);
					}
					catch (Exception& e)
					{
						if (e.getNumber() == Exception::ERR_ITEM_NOT_FOUND)
						{
							// duff geometry data entry with 0 vertices
							delete pMesh->sharedVertexData;
							pMesh->sharedVertexData = 0;
							// Skip this stream (pointer will have been returned to just after header)
							stream.skip(mCurrentstreamLen - stream_OVERHEAD_SIZE);
						}
						else
						{
							throw;
						}
					}
					break;
                case M_SUBMESH:
                    readSubMesh(stream, pMesh);
                    break;
                case M_MESH_SKELETON_LINK:
                    readSkeletonLink(stream, pMesh);
                    break;
                case M_MESH_BONE_ASSIGNMENT:
                    readMeshBoneAssignment(stream, pMesh);
                    break;
                case M_MESH_LOD:
					readMeshLodInfo(stream, pMesh);
					break;
                case M_MESH_BOUNDS:
                    readBoundsInfo(stream, pMesh);
                    break;
				case M_SUBMESH_NAME_TABLE:
    	            readSubMeshNameTable(stream, pMesh);
					break;
                case M_EDGE_LISTS:
                    readEdgeList(stream, pMesh);
                    break;
					
                }

                if (!stream.eof())
                {
                    streamID = readChunk(stream);
                }

            }
            if (!stream.eof())
            {
                // Backpedal back to start of stream
                stream.skip(-(long)stream_OVERHEAD_SIZE);
            }
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMesh(DataStreamPtr& stream, Mesh* pMesh)
    {
        unsigned short streamID;

        SubMesh* sm = pMesh->createSubMesh();
        // char* materialName
        String materialName = readString(stream);
        sm->setMaterialName(materialName);

        // bool useSharedVertices
        readBools(stream,&sm->useSharedVertices, 1);

        // unsigned int indexCount
        sm->indexData->indexStart = 0;
        readInts(stream, &sm->indexData->indexCount, 1);

        HardwareIndexBufferSharedPtr ibuf;
        // bool indexes32Bit
        bool idx32bit;
        readBools(stream, &idx32bit, 1);
        if (idx32bit)
        {
            ibuf = HardwareBufferManager::getSingleton().
                createIndexBuffer(
                    HardwareIndexBuffer::IT_32BIT, 
                    sm->indexData->indexCount, 
                    pMesh->mIndexBufferUsage,
					pMesh->mIndexBufferShadowBuffer);
            // unsigned int* faceVertexIndices 
            unsigned int* pIdx = static_cast<unsigned int*>(
                ibuf->lock(HardwareBuffer::HBL_DISCARD)
                );
            readInts(stream, pIdx, sm->indexData->indexCount);
            ibuf->unlock();

        }
        else // 16-bit
        {
            ibuf = HardwareBufferManager::getSingleton().
                createIndexBuffer(
                    HardwareIndexBuffer::IT_16BIT, 
                    sm->indexData->indexCount, 
                    pMesh->mIndexBufferUsage,
					pMesh->mIndexBufferShadowBuffer);
            // unsigned short* faceVertexIndices 
            unsigned short* pIdx = static_cast<unsigned short*>(
                ibuf->lock(HardwareBuffer::HBL_DISCARD)
                );
            readShorts(stream, pIdx, sm->indexData->indexCount);
            ibuf->unlock();
        }
        sm->indexData->indexBuffer = ibuf;

        // M_GEOMETRY stream (Optional: present only if useSharedVertices = false)
        if (!sm->useSharedVertices)
        {
            streamID = readChunk(stream);
            if (streamID != M_GEOMETRY)
            {
                Except(Exception::ERR_INTERNAL_ERROR, "Missing geometry data in mesh file", 
                    "MeshSerializerImpl::readSubMesh");
            }
            sm->vertexData = new VertexData();
            readGeometry(stream, pMesh, sm->vertexData);
        }


        // Find all bone assignments (if present) 
        if (!stream.eof())
        {
            streamID = readChunk(stream);
            while(!stream.eof() &&
                (streamID == M_SUBMESH_BONE_ASSIGNMENT ||
                 streamID == M_SUBMESH_OPERATION))
            {
                switch(streamID)
                {
                case M_SUBMESH_OPERATION:
                    readSubMeshOperation(stream, pMesh, sm);
                    break;
                case M_SUBMESH_BONE_ASSIGNMENT:
                    readSubMeshBoneAssignment(stream, pMesh, sm);
                    break;
                }

                if (!stream.eof())
                {
                    streamID = readChunk(stream);
                }

            }
            if (!stream.eof())
            {
                // Backpedal back to start of stream
                stream.skip(-(long)stream_OVERHEAD_SIZE);
            }
        }
	

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMeshOperation(DataStreamPtr& stream, 
        Mesh* pMesh, SubMesh* sm)
    {
        // unsigned short operationType
        unsigned short opType;
        readShorts(stream, &opType, 1);
        sm->operationType = static_cast<RenderOperation::OperationType>(opType);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSkeletonLink(const String& skelName)
    {
        writeChunkHeader(M_MESH_SKELETON_LINK, calcSkeletonLinkSize(skelName));

        writeString(skelName);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSkeletonLink(DataStream &stream, Mesh* pMesh)
    {
        String skelName = readString(stream);
        pMesh->setSkeletonName(skelName);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readTextureLayer(DataStreamPtr& stream, Mesh* pMesh,
        MaterialPtr& pMat)
    {
        // Material definition section phased out of 1.1
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcSkeletonLinkSize(const String& skelName)
    {
        unsigned long size = stream_OVERHEAD_SIZE;

        size += (unsigned long)skelName.length() + 1;

        return size;

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeMeshBoneAssignment(const VertexBoneAssignment* assign)
    {
        writeChunkHeader(M_MESH_BONE_ASSIGNMENT, calcBoneAssignmentSize());

        // unsigned int vertexIndex;
        writeInts(&(assign->vertexIndex), 1);
        // unsigned short boneIndex;
        writeShorts(&(assign->boneIndex), 1);
        // Real weight;
        writeReals(&(assign->weight), 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMeshBoneAssignment(const VertexBoneAssignment* assign)
    {
        writeChunkHeader(M_SUBMESH_BONE_ASSIGNMENT, calcBoneAssignmentSize());

        // unsigned int vertexIndex;
        writeInts(&(assign->vertexIndex), 1);
        // unsigned short boneIndex;
        writeShorts(&(assign->boneIndex), 1);
        // Real weight;
        writeReals(&(assign->weight), 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readMeshBoneAssignment(DataStreamPtr& stream, Mesh* pMesh)
    {
        VertexBoneAssignment assign;

        // unsigned int vertexIndex;
        readInts(stream, &(assign.vertexIndex),1);
        // unsigned short boneIndex;
        readShorts(stream, &(assign.boneIndex),1);
        // Real weight;
        readReals(stream, &(assign.weight), 1);

        pMesh->addBoneAssignment(assign);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMeshBoneAssignment(DataStreamPtr& stream, 
        Mesh* pMesh, SubMesh* sub)
    {
        VertexBoneAssignment assign;

        // unsigned int vertexIndex;
        readInts(stream, &(assign.vertexIndex),1);
        // unsigned short boneIndex;
        readShorts(stream, &(assign.boneIndex),1);
        // Real weight;
        readReals(stream, &(assign.weight), 1);

        sub->addBoneAssignment(assign);

    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcBoneAssignmentSize(void)
    {
        unsigned long size;

        size = stream_OVERHEAD_SIZE;

        // Vert index
        size += sizeof(unsigned int);
        // Bone index
        size += sizeof(unsigned short);
        // weight
        size += sizeof(Real);

        return size;
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodInfo(const Mesh* pMesh)
    {
        unsigned short numLods = pMesh->getNumLodLevels();
        bool manual = pMesh->isLodManual();
        writeLodSummary(numLods, manual);

		// Loop from LOD 1 (not 0, this is full detail)
        for (unsigned short i = 1; i < numLods; ++i)
        {
			const Mesh::MeshLodUsage& usage = pMesh->getLodLevel(i);
			if (manual)
			{
				writeLodUsageManual(usage);
			}
			else
			{
				writeLodUsageGenerated(pMesh, usage, i);
			}
            
        }
        

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodSummary(unsigned short numLevels, bool manual)
    {
        // Header
        unsigned long size = stream_OVERHEAD_SIZE;
        // unsigned short numLevels;
        size += sizeof(unsigned short);
        // bool manual;  (true for manual alternate meshes, false for generated)
        size += sizeof(bool);
        writeChunkHeader(M_MESH_LOD, size);

        // Details
        // unsigned short numLevels;
        writeShorts(&numLevels, 1);
        // bool manual;  (true for manual alternate meshes, false for generated)
        writeBools(&manual, 1);

        
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodUsageManual(const Mesh::MeshLodUsage& usage)
    {
        // Header
        unsigned long size = stream_OVERHEAD_SIZE;
        unsigned long manualSize = stream_OVERHEAD_SIZE;
        // Real fromDepthSquared;
        size += sizeof(Real);
        // Manual part size

        // String manualMeshName;
        manualSize += static_cast<unsigned long>(usage.manualName.length() + 1);

        size += manualSize;

        writeChunkHeader(M_MESH_LOD_USAGE, size);
        writeReals(&(usage.fromDepthSquared), 1);

        writeChunkHeader(M_MESH_LOD_MANUAL, manualSize);
        writeString(usage.manualName);
        

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodUsageGenerated(const Mesh* pMesh, const Mesh::MeshLodUsage& usage,
		unsigned short lodNum)
    {
		// Usage Header
        unsigned long size = stream_OVERHEAD_SIZE;
		unsigned short subidx;

        // Real fromDepthSquared;
        size += sizeof(Real);

        // Calc generated SubMesh sections size
		for(subidx = 0; subidx < pMesh->getNumSubMeshes(); ++subidx)
		{
			// header
			size += stream_OVERHEAD_SIZE;
			// unsigned int numFaces;
			size += sizeof(unsigned int);
			SubMesh* sm = pMesh->getSubMesh(subidx);
            const IndexData* indexData = sm->mLodFaceList[lodNum - 1];

            // bool indexes32Bit
			size += sizeof(bool);
			// unsigned short*/int* faceIndexes;  
            if (indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned int) * indexData->indexCount);
            }
            else
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned short) * indexData->indexCount);
            }

		}

        writeChunkHeader(M_MESH_LOD_USAGE, size);
        writeReals(&(usage.fromDepthSquared), 1);

		// Now write sections
        // Calc generated SubMesh sections size
		for(subidx = 0; subidx < pMesh->getNumSubMeshes(); ++subidx)
		{
			size = stream_OVERHEAD_SIZE;
			// unsigned int numFaces;
			size += sizeof(unsigned int);
			SubMesh* sm = pMesh->getSubMesh(subidx);
            const IndexData* indexData = sm->mLodFaceList[lodNum - 1];
            // bool indexes32Bit
			size += sizeof(bool);
			// unsigned short*/int* faceIndexes;  
            if (indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned int) * indexData->indexCount);
            }
            else
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned short) * indexData->indexCount);
            }

			writeChunkHeader(M_MESH_LOD_GENERATED, size);
			unsigned int idxCount = static_cast<unsigned int>(indexData->indexCount);
			writeInts(&idxCount, 1);
            // Lock index buffer to write
            HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;
			// bool indexes32bit
			bool idx32 = (ibuf->getType() == HardwareIndexBuffer::IT_32BIT);
			writeBools(&idx32, 1);
            if (idx32)
            {
                unsigned int* pIdx = static_cast<unsigned int*>(
                    ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
			    writeInts(pIdx, indexData->indexCount);
                ibuf->unlock();
            }
            else
            {
                unsigned short* pIdx = static_cast<unsigned short*>(
                    ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
			    writeShorts(pIdx, indexData->indexCount);
                ibuf->unlock();
            }
		}
	

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeBoundsInfo(const Mesh* pMesh)
    {
		// Usage Header
        unsigned long size = stream_OVERHEAD_SIZE;

        size += sizeof(Real) * 7;
        writeChunkHeader(M_MESH_BOUNDS, size);

        // Real minx, miny, minz
        const Vector3& min = pMesh->mAABB.getMinimum();
        const Vector3& max = pMesh->mAABB.getMaximum();
        writeReals(&min.x, 1);
        writeReals(&min.y, 1);
        writeReals(&min.z, 1);
        // Real maxx, maxy, maxz
        writeReals(&max.x, 1);
        writeReals(&max.y, 1);
        writeReals(&max.z, 1);
        // Real radius
        writeReals(&pMesh->mBoundRadius, 1);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readBoundsInfo(DataStreamPtr& stream, Mesh* pMesh)
    {
        Vector3 min, max;
        // Real minx, miny, minz
        readReals(stream, &min.x, 1);
        readReals(stream, &min.y, 1);
        readReals(stream, &min.z, 1);
        // Real maxx, maxy, maxz
        readReals(stream, &max.x, 1);
        readReals(stream, &max.y, 1);
        readReals(stream, &max.z, 1);
        AxisAlignedBox box(min, max);
        pMesh->_setBounds(box, true);
        // Real radius
        Real radius;
        readReals(stream, &radius, 1);
        pMesh->_setBoundingSphereRadius(radius);



    }
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readMeshLodInfo(DataStreamPtr& stream, Mesh* pMesh)
	{
		unsigned short streamID, i;

        // unsigned short numLevels;
		readShorts(stream, &(pMesh->mNumLods), 1);
        // bool manual;  (true for manual alternate meshes, false for generated)
		readBools(stream, &(pMesh->mIsLodManual), 1);

		// Preallocate submesh lod face data if not manual
		if (!pMesh->mIsLodManual)
		{
			unsigned short numsubs = pMesh->getNumSubMeshes();
			for (i = 0; i < numsubs; ++i)
			{
				SubMesh* sm = pMesh->getSubMesh(i);
				sm->mLodFaceList.resize(pMesh->mNumLods-1);
			}
		}

		// Loop from 1 rather than 0 (full detail index is not in file)
		for (i = 1; i < pMesh->mNumLods; ++i)
		{
			streamID = readChunk(stream);
			if (streamID != M_MESH_LOD_USAGE)
			{
				Except(Exception::ERR_ITEM_NOT_FOUND, 
					"Missing M_MESH_LOD_USAGE stream in " + pMesh->getName(), 
					"MeshSerializerImpl::readMeshLodInfo");
			}
			// Read depth
			Mesh::MeshLodUsage usage;
			readReals(stream, &(usage.fromDepthSquared), 1);

			if (pMesh->isLodManual())
			{
				readMeshLodUsageManual(stream, pMesh, i, usage);
			}
			else //(!pMesh->isLodManual)
			{
				readMeshLodUsageGenerated(stream, pMesh, i, usage);
			}
            usage.edgeData = NULL;

			// Save usage
			pMesh->mMeshLodUsageList.push_back(usage);
		}


	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readMeshLodUsageManual(DataStreamPtr& stream, 
        Mesh* pMesh, unsigned short lodNum, Mesh::MeshLodUsage& usage)
	{
		unsigned long streamID;
		// Read detail stream
		streamID = readChunk(stream);
		if (streamID != M_MESH_LOD_MANUAL)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Missing M_MESH_LOD_MANUAL stream in " + pMesh->getName(),
				"MeshSerializerImpl::readMeshLodUsageManual");
		}

		usage.manualName = readString(stream);
		usage.manualMesh = NULL; // will trigger load later
	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readMeshLodUsageGenerated(DataStreamPtr& stream, 
        Mesh* pMesh, unsigned short lodNum, Mesh::MeshLodUsage& usage)
	{
		usage.manualName = "";
		usage.manualMesh = 0;

		// Get one set of detail per SubMesh
		unsigned short numSubs, i;
		unsigned long streamID;
		numSubs = pMesh->getNumSubMeshes();
		for (i = 0; i < numSubs; ++i)
		{
			streamID = readChunk(stream);
			if (streamID != M_MESH_LOD_GENERATED)
			{
				Except(Exception::ERR_ITEM_NOT_FOUND, 
					"Missing M_MESH_LOD_GENERATED stream in " + pMesh->getName(),
					"MeshSerializerImpl::readMeshLodUsageGenerated");
			}

			SubMesh* sm = pMesh->getSubMesh(i);
			// lodNum - 1 because SubMesh doesn't store full detail LOD
            sm->mLodFaceList[lodNum - 1] = new IndexData();
			IndexData* indexData = sm->mLodFaceList[lodNum - 1];
            // unsigned int numIndexes
            unsigned int numIndexes;
			readInts(stream, &numIndexes, 1);
            indexData->indexCount = static_cast<size_t>(numIndexes);
            // bool indexes32Bit
            bool idx32Bit;
            readBools(stream, &idx32Bit, 1);
            // unsigned short*/int* faceIndexes;  ((v1, v2, v3) * numFaces)
            if (idx32Bit)
            {
                indexData->indexBuffer = HardwareBufferManager::getSingleton().
                    createIndexBuffer(HardwareIndexBuffer::IT_32BIT, indexData->indexCount,
                    pMesh->mIndexBufferUsage, pMesh->mIndexBufferShadowBuffer);
                unsigned int* pIdx = static_cast<unsigned int*>(
                    indexData->indexBuffer->lock(
                        0, 
                        indexData->indexBuffer->getSizeInBytes(), 
                        HardwareBuffer::HBL_DISCARD) );

			    readInts(stream, pIdx, indexData->indexCount);
                indexData->indexBuffer->unlock();

            }
            else
            {
                indexData->indexBuffer = HardwareBufferManager::getSingleton().
                    createIndexBuffer(HardwareIndexBuffer::IT_16BIT, indexData->indexCount,
                    pMesh->mIndexBufferUsage, pMesh->mIndexBufferShadowBuffer);
                unsigned short* pIdx = static_cast<unsigned short*>(
                    indexData->indexBuffer->lock(
                        0, 
                        indexData->indexBuffer->getSizeInBytes(), 
                        HardwareBuffer::HBL_DISCARD) );
			    readShorts(stream, pIdx, indexData->indexCount);
                indexData->indexBuffer->unlock();

            }

		}
	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipFromLittleEndian(void* pData, size_t vertexCount, 
        size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
	{
#	if OGRE_ENDIAN == ENDIAN_BIG
        flipEndian(pData, vertexCount, vertexSize, elems);
#	endif	
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipToLittleEndian(void* pData, size_t vertexCount, 
			size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
	{
#	if OGRE_ENDIAN == ENDIAN_BIG
        flipEndian(pData, vertexCount, vertexSize, elems);
#	endif	
	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipEndian(void* pData, size_t vertexCount, 
        size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
	{
		void *pBase = pData;
		for (size_t v = 0; v < vertexCount; ++v)
		{
			VertexDeclaration::VertexElementList::const_iterator ei, eiend;
			eiend = elems.end();
			for (ei = elems.begin(); ei != eiend; ++ei)
			{
				void *pElem;
				// re-base pointer to the element
				(*ei).baseVertexPointerToElement(pBase, &pElem);
				// Flip the endian based on the type
				size_t typeSize = 0;
				switch (VertexElement::getBaseType((*ei).getType()))
				{
					case VET_FLOAT1:
						typeSize = sizeof(Real);
						break;
					case VET_SHORT1:
						typeSize = sizeof(short);
						break;
					case VET_COLOUR:
						typeSize = sizeof(RGBA);
						break;
					case VET_UBYTE4:
						typeSize = 0; // NO FLIPPING
						break;
				};
                Serializer::flipEndian(pElem, typeSize, 
					VertexElement::getTypeCount((*ei).getType()));
				
			}

			pBase = static_cast<void*>(
				static_cast<unsigned char*>(pBase) + vertexSize);
			
		}
	}
    //---------------------------------------------------------------------
	unsigned long MeshSerializerImpl::calcEdgeListSize(const Mesh* pMesh)
	{
        size_t size = stream_OVERHEAD_SIZE;

        for (ushort i = 0; i < pMesh->getNumLodLevels(); ++i)
        {
            
            const EdgeData* edgeData = pMesh->getEdgeList(i);
            bool isManual = pMesh->isLodManual() && (i > 0);

            size += calcEdgeListLodSize(edgeData, isManual);

        }

        return size;
	}
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcEdgeListLodSize(const EdgeData* edgeData, bool isManual)
    {
        size_t size = stream_OVERHEAD_SIZE;

        // unsigned short lodIndex
        size += sizeof(unsigned short);

        // bool isManual			// If manual, no edge data here, loaded from manual mesh
        size += sizeof(bool);
        if (!isManual)
        {
            // unsigned long numTriangles
            size += sizeof(unsigned long);
            // unsigned long numEdgeGroups
            size += sizeof(unsigned long);
            // Triangle* triangleList
            size_t triSize = 0;
            // unsigned long indexSet
            // unsigned long vertexSet
            // unsigned long vertIndex[3]
            // unsigned long sharedVertIndex[3] 
            // Real normal[4] 
            triSize += sizeof(unsigned long) * 8 
                    + sizeof(Real) * 4;

            size += triSize * edgeData->triangles.size();
            // Write the groups
            for (EdgeData::EdgeGroupList::const_iterator gi = edgeData->edgeGroups.begin();
                gi != edgeData->edgeGroups.end(); ++gi)
            {
                const EdgeData::EdgeGroup& edgeGroup = *gi;
                size += calcEdgeGroupSize(edgeGroup);
            }

        }

        return size;
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcEdgeGroupSize(const EdgeData::EdgeGroup& group)
    {
        size_t size = stream_OVERHEAD_SIZE;

        // unsigned long vertexSet
        size += sizeof(unsigned long);
        // unsigned long numEdges
        size += sizeof(unsigned long);
        // Edge* edgeList
        size_t edgeSize = 0;
        // unsigned long  triIndex[2]
        // unsigned long  vertIndex[2]
        // unsigned long  sharedVertIndex[2]
        // bool degenerate
        edgeSize += sizeof(unsigned long) * 6 + sizeof(bool);
        size += edgeSize * group.edges.size();

        return size;
    }
    //---------------------------------------------------------------------
	void MeshSerializerImpl::writeEdgeList(const Mesh* pMesh)
	{
        writeChunkHeader(M_EDGE_LISTS, calcEdgeListSize(pMesh));

        for (ushort i = 0; i < pMesh->getNumLodLevels(); ++i)
        {
            const EdgeData* edgeData = pMesh->getEdgeList(i);
            bool isManual = pMesh->isLodManual() && (i > 0);
            writeChunkHeader(M_EDGE_LIST_LOD, calcEdgeListLodSize(edgeData, isManual));

            // unsigned short lodIndex
            writeShorts(&i, 1);

            // bool isManual			// If manual, no edge data here, loaded from manual mesh
            writeBools(&isManual, 1);
            if (!isManual)
            {
                // unsigned long  numTriangles
                unsigned long count = static_cast<unsigned long>(edgeData->triangles.size());
                writeLongs(&count, 1);
                // unsigned long numEdgeGroups
                count = static_cast<unsigned long>(edgeData->edgeGroups.size());
                writeLongs(&count, 1);
                // Triangle* triangleList
                // Iterate rather than writing en-masse to allow endian conversion
                for (EdgeData::TriangleList::const_iterator t = edgeData->triangles.begin();
                    t != edgeData->triangles.end(); ++t)
                {
                    const EdgeData::Triangle& tri = *t;
                    // unsigned long indexSet; 
                    unsigned long tmp[3];
                    tmp[0] = tri.indexSet;
                    writeLongs(tmp, 1);
                    // unsigned long vertexSet;
                    tmp[0] = tri.vertexSet;
                    writeLongs(tmp, 1);
                    // unsigned long vertIndex[3];
                    tmp[0] = tri.vertIndex[0];
                    tmp[1] = tri.vertIndex[1];
                    tmp[2] = tri.vertIndex[2];
                    writeLongs(tmp, 3);
                    // unsigned long sharedVertIndex[3]; 
                    tmp[0] = tri.sharedVertIndex[0];
                    tmp[1] = tri.sharedVertIndex[1];
                    tmp[2] = tri.sharedVertIndex[2];
                    writeLongs(tmp, 3);
                    // Real normal[4];   
                    writeReals(&(tri.normal.x), 4);

                }
                // Write the groups
                for (EdgeData::EdgeGroupList::const_iterator gi = edgeData->edgeGroups.begin();
                    gi != edgeData->edgeGroups.end(); ++gi)
                {
                    const EdgeData::EdgeGroup& edgeGroup = *gi;
                    writeChunkHeader(M_EDGE_GROUP, calcEdgeGroupSize(edgeGroup));
                    // unsigned long vertexSet
                    unsigned long vertexSet = static_cast<unsigned long>(edgeGroup.vertexSet);
                    writeLongs(&vertexSet, 1);
                    // unsigned long numEdges
                    count = static_cast<unsigned long>(edgeGroup.edges.size());
                    writeLongs(&count, 1);
                    // Edge* edgeList
                    // Iterate rather than writing en-masse to allow endian conversion
                    for (EdgeData::EdgeList::const_iterator ei = edgeGroup.edges.begin();
                        ei != edgeGroup.edges.end(); ++ei)
                    {
                        const EdgeData::Edge& edge = *ei;
                        unsigned long tmp[2];
                        // unsigned long  triIndex[2]
                        tmp[0] = edge.triIndex[0];
                        tmp[1] = edge.triIndex[1];
                        writeLongs(tmp, 2);
                        // unsigned long  vertIndex[2]
                        tmp[0] = edge.vertIndex[0];
                        tmp[1] = edge.vertIndex[1];
                        writeLongs(tmp, 2);
                        // unsigned long  sharedVertIndex[2]
                        tmp[0] = edge.sharedVertIndex[0];
                        tmp[1] = edge.sharedVertIndex[1];
                        writeLongs(tmp, 2);
                        // bool degenerate
                        writeBools(&(edge.degenerate), 1);
                    }

                }

            }

        }
	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readEdgeList(DataStreamPtr& stream, Mesh* pMesh)
	{
        unsigned short streamID;

        if (!stream.eof())
        {
            streamID = readChunk(stream);
            while(!stream.eof() &&
                streamID == M_EDGE_LIST_LOD)
            {
                // Process single LOD

                // unsigned short lodIndex
                unsigned short lodIndex;
                readShorts(stream, &lodIndex, 1);

                // bool isManual			// If manual, no edge data here, loaded from manual mesh
                bool isManual;
                readBools(stream, &isManual, 1);
                // Only load in non-manual levels; others will be connected up by Mesh on demand
                if (!isManual)
                {
                    Mesh::MeshLodUsage& usage = const_cast<Mesh::MeshLodUsage&>(pMesh->getLodLevel(lodIndex));

                    usage.edgeData = new EdgeData();
                    // unsigned long numTriangles
                    unsigned long numTriangles;
                    readLongs(stream, &numTriangles, 1);
                    // Allocate correct amount of memory
                    usage.edgeData->triangles.resize(numTriangles);
                    // unsigned long numEdgeGroups
                    unsigned long numEdgeGroups;
                    readLongs(stream, &numEdgeGroups, 1);
                    // Allocate correct amount of memory
                    usage.edgeData->edgeGroups.resize(numEdgeGroups);
                    // Triangle* triangleList
                    unsigned long tmp[3];
                    for (size_t t = 0; t < numTriangles; ++t)
                    {
                        EdgeData::Triangle& tri = usage.edgeData->triangles[t];
                        // unsigned long indexSet
                        readLongs(stream, tmp, 1);
                        tri.indexSet = tmp[0];
                        // unsigned long vertexSet
                        readLongs(stream, tmp, 1);
                        tri.vertexSet = tmp[0];
                        // unsigned long vertIndex[3]
                        readLongs(stream, tmp, 3);
                        tri.vertIndex[0] = tmp[0];
                        tri.vertIndex[1] = tmp[1];
                        tri.vertIndex[2] = tmp[2];
                        // unsigned long sharedVertIndex[3] 
                        readLongs(stream, tmp, 3);
                        tri.sharedVertIndex[0] = tmp[0];
                        tri.sharedVertIndex[1] = tmp[1];
                        tri.sharedVertIndex[2] = tmp[2];
                        // Real normal[4] 
                        readReals(stream, &(tri.normal.x), 4);

                    }

                    for (unsigned long eg = 0; eg < numEdgeGroups; ++eg)
                    {
                        streamID = readChunk(stream);
                        if (streamID != M_EDGE_GROUP)
                        {
                            Except(Exception::ERR_INTERNAL_ERROR, 
                                "Missing M_EDGE_GROUP stream", 
                                "MeshSerializerImpl::readEdgeList");
                        }
                        EdgeData::EdgeGroup& edgeGroup = usage.edgeData->edgeGroups[eg];

                        // unsigned long vertexSet
                        readLongs(stream, tmp, 1);
                        edgeGroup.vertexSet = tmp[0];
                        // unsigned long numEdges
                        unsigned long numEdges;
                        readLongs(stream, &numEdges, 1);
                        edgeGroup.edges.resize(numEdges);
                        // Edge* edgeList
                        for (unsigned long e = 0; e < numEdges; ++e)
                        {
                            EdgeData::Edge& edge = edgeGroup.edges[e];
                            // unsigned long  triIndex[2]
                            readLongs(stream, tmp, 2);
                            edge.triIndex[0] = tmp[0];
                            edge.triIndex[1] = tmp[1];
                            // unsigned long  vertIndex[2]
                            readLongs(stream, tmp, 2);
                            edge.vertIndex[0] = tmp[0];
                            edge.vertIndex[1] = tmp[1];
                            // unsigned long  sharedVertIndex[2]
                            readLongs(stream, tmp, 2);
                            edge.sharedVertIndex[0] = tmp[0];
                            edge.sharedVertIndex[1] = tmp[1];
                            // bool degenerate
                            readBools(stream, &(edge.degenerate), 1);
                        }
                        // Populate edgeGroup.vertexData pointers
                        // If there is shared vertex data, vertexSet 0 is that, 
                        // otherwise 0 is first dedicated
                        if (pMesh->sharedVertexData)
                        {
                            if (edgeGroup.vertexSet == 0)
                            {
                                edgeGroup.vertexData = pMesh->sharedVertexData;
                            }
                            else
                            {
                                edgeGroup.vertexData = pMesh->getSubMesh(
                                    edgeGroup.vertexSet-1)->vertexData;
                            }
                        }
                        else
                        {
                            edgeGroup.vertexData = pMesh->getSubMesh(
                                edgeGroup.vertexSet)->vertexData;
                        }
                    }
                    
                }

                if (!stream.eof())
                {
                    streamID = readChunk(stream);
                }

            }
            if (!stream.eof())
            {
                // Backpedal back to start of stream
                stream.skip(-(long)stream_OVERHEAD_SIZE);
            }
        }



        pMesh->mEdgeListsBuilt = true;
	}
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_2::MeshSerializerImpl_v1_2()
    {
        // Version number
        mVersion = "[MeshSerializer_v1.20]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_2::~MeshSerializerImpl_v1_2()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readMesh(DataStreamPtr& stream, Mesh* pMesh)
    {
        MeshSerializerImpl::readMesh(stream, pMesh);
        // Always automatically build edge lists for this version
        pMesh->mAutoBuildEdgeLists = true;
        
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometry(DataStreamPtr& stream, Mesh* pMesh, 
        VertexData* dest)
    {
        unsigned short texCoordSet = 0;
        
        unsigned short bindIdx = 0;

        dest->vertexStart = 0;

        // unsigned int numVertices
        readInts(stream, &dest->vertexCount, 1);

        // Vertex buffers

        readGeometryPositions(bindIdx, stream, pMesh, dest);
        ++bindIdx;

        // Find optional geometry streams
        if (!stream.eof())
        {
            unsigned short streamID = readChunk(stream);
            while(!stream.eof() && 
                (streamID == M_GEOMETRY_NORMALS || 
                 streamID == M_GEOMETRY_COLOURS ||
                 streamID == M_GEOMETRY_TEXCOORDS ))
            {
                switch (streamID)
                {
                case M_GEOMETRY_NORMALS:
                    readGeometryNormals(bindIdx++, stream, pMesh, dest);
                    break;
                case M_GEOMETRY_COLOURS:
                    readGeometryColours(bindIdx++, stream, pMesh, dest);
                    break;
                case M_GEOMETRY_TEXCOORDS:
                    readGeometryTexCoords(bindIdx++, stream, pMesh, dest, texCoordSet++);
                    break;
                }
                // Get next stream
                if (!stream.eof())
                {
                    streamID = readChunk(stream);
                }
            }
            if (!stream.eof())
            {
                // Backpedal back to start of non-submesh stream
                stream.skip(-(long)stream_OVERHEAD_SIZE);
            }
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryPositions(unsigned short bindIdx, 
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest)
    {
        Real *pReal = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // Real* pVertices (x, y, z order x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_FLOAT3, VES_POSITION);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->mVertexBufferUsage, 
			pMesh->mIndexBufferShadowBuffer);
        pReal = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readReals(stream, pReal, dest->vertexCount * 3);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryNormals(unsigned short bindIdx, 
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest)
    {
        Real *pReal = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // Real* pNormals (x, y, z order x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_FLOAT3, VES_NORMAL);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->mVertexBufferUsage,
			pMesh->mVertexBufferShadowBuffer);
        pReal = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readReals(stream, pReal, dest->vertexCount * 3);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryColours(unsigned short bindIdx, 
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest)
    {
        RGBA* pRGBA = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // unsigned long* pColours (RGBA 8888 format x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_COLOUR, VES_DIFFUSE);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->mVertexBufferUsage,
			pMesh->mVertexBufferShadowBuffer);
        pRGBA = static_cast<RGBA*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readLongs(stream, pRGBA, dest->vertexCount);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryTexCoords(unsigned short bindIdx, 
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest, unsigned short texCoordSet)
    {
        Real *pReal = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // unsigned short dimensions    (1 for 1D, 2 for 2D, 3 for 3D)
        unsigned short dim;
        readShorts(stream, &dim, 1);
        // Real* pTexCoords  (u [v] [w] order, dimensions x numVertices)
        dest->vertexDeclaration->addElement(
            bindIdx, 
            0, 
            VertexElement::multiplyTypeCount(VET_FLOAT1, dim), 
            VES_TEXTURE_COORDINATES,
            texCoordSet);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->mVertexBufferUsage,
			pMesh->mVertexBufferShadowBuffer);
        pReal = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readReals(stream, pReal, dest->vertexCount * dim);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_1::MeshSerializerImpl_v1_1()
    {
        // Version number
        mVersion = "[MeshSerializer_v1.10]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_1::~MeshSerializerImpl_v1_1()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_1::readGeometryTexCoords(unsigned short bindIdx, 
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest, unsigned short texCoordSet)
    {
        Real *pReal = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // unsigned short dimensions    (1 for 1D, 2 for 2D, 3 for 3D)
        unsigned short dim;
        readShorts(stream, &dim, 1);
        // Real* pTexCoords  (u [v] [w] order, dimensions x numVertices)
        dest->vertexDeclaration->addElement(
            bindIdx, 
            0, 
            VertexElement::multiplyTypeCount(VET_FLOAT1, dim), 
            VES_TEXTURE_COORDINATES,
            texCoordSet);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->getVertexBufferUsage(),
			pMesh->isVertexBufferShadowed());
        pReal = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readReals(stream, pReal, dest->vertexCount * dim);

        // Adjust individual v values to (1 - v)
        if (dim == 2)
        {
            for (size_t i = 0; i < dest->vertexCount; ++i)
            {
                ++pReal; // skip u
                *pReal = 1.0 - *pReal; // v = 1 - v
                ++pReal;
            }
            
        }
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------




}

