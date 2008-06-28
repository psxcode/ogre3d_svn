#include "ProceduralTools.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>

using namespace Ogre;

//Constants (copied as is from sample)

// Grid sizes (in vertices)
#define X_SIZE_LOG2		6
#define Y_SIZE_LOG2		6
#define Z_SIZE_LOG2		6
#define TOTAL_POINTS	(1<<(X_SIZE_LOG2 + Y_SIZE_LOG2 + Z_SIZE_LOG2))
#define CELLS_COUNT		(((1<<X_SIZE_LOG2) - 1) * ((1<<Y_SIZE_LOG2) - 1) * ((1<<Z_SIZE_LOG2) - 1))

#define SWIZZLE	1

#define MAKE_INDEX(x, y, z, sizeLog2)	(int)((x) | ((y) << sizeLog2[0]) | ((z) << (sizeLog2[0] + sizeLog2[1])))

//--------------------------------------------------------------------------------------
// Fills pPos with x, y, z de-swizzled from index with bitsizes in sizeLog2
//
//  Traversing the grid in a swizzled fashion improves locality of reference,
// and this is very beneficial when sampling a texture.
//--------------------------------------------------------------------------------------
void UnSwizzle(uint index, uint sizeLog2[3], uint * pPos)
{

    // force index traversal to occur in 2x2x2 blocks by giving each of x, y, and z one
    // of the bottom 3 bits
	pPos[0] = index & 1;
    index >>= 1;
    pPos[1] = index & 1;
    index >>= 1;
    pPos[2] = index & 1;
    index >>= 1;

    // Treat the rest of the index like a row, collumn, depth array
    // Each dimension needs to grab sizeLog2 - 1 bits
    // This will make the blocks traverse the grid in a raster style order
    index <<= 1;
    pPos[0] = pPos[0] | (index &  ( (1 << sizeLog2[0]) - 2));
    index >>=  sizeLog2[0] - 1;
    pPos[1] = pPos[1] | ( index &  ( (1 << sizeLog2[1]) - 2));
    index >>= sizeLog2[1] - 1;
    pPos[2] = pPos[2] | ( index &  ( (1 << sizeLog2[2]) - 2));
}


ManualObject* ProceduralTools::generateTetrahedra(SceneManager* sceneManager)
{
	ManualObject* tetrahedra = sceneManager->createManualObject("Tetrahedra");

	uint sizeLog2[3] = { X_SIZE_LOG2, Y_SIZE_LOG2, Z_SIZE_LOG2 };
	uint nTotalBits = sizeLog2[0] + sizeLog2[1] + sizeLog2[2];
	uint nPointsTotal = 1 << nTotalBits;

	tetrahedra->begin("");
	tetrahedra->estimateVertexCount(nPointsTotal);
	
	//Generate positions
	for(uint i=0; i<nPointsTotal; i++) {
		uint pos[3];
		pos[0] = i & ((1<<X_SIZE_LOG2)-1);
		pos[1] = (i >> X_SIZE_LOG2) & ((1<<Y_SIZE_LOG2)-1);
		pos[2] = (i >> (X_SIZE_LOG2+Y_SIZE_LOG2)) & ((1<<Z_SIZE_LOG2)-1);

		Vector3 posVector;
		posVector.x = (Real(pos[0]) / Real(1<<X_SIZE_LOG2))*2.0-1.0;
		posVector.y = (Real(pos[1]) / Real(1<<Y_SIZE_LOG2))*2.0-1.0;
		posVector.z = (Real(pos[2]) / Real(1<<Z_SIZE_LOG2))*2.0-1.0;
		
		tetrahedra->position(posVector);
	}

	tetrahedra->estimateIndexCount(nPointsTotal * 6 * 4);
	//Generate indices
	for (uint i = 0; i<nPointsTotal; i++) {

		uint pos[3];
#if SWIZZLE
		UnSwizzle(i, sizeLog2, pos);	// un-swizzle current index to get x, y, z for the current sampling point
#else
		pos[0] = i & ((1<<X_SIZE_LOG2)-1);
		pos[1] = (i >> X_SIZE_LOG2) & ((1<<Y_SIZE_LOG2)-1);
		pos[2] = (i >> (X_SIZE_LOG2+Y_SIZE_LOG2)) & ((1<<Z_SIZE_LOG2)-1);
#endif
		if (pos[0] == (1 << sizeLog2[0]) - 1 || pos[1] == (1 << sizeLog2[1]) - 1 || pos[2] == (1 << sizeLog2[2]) - 1)
			continue;	// skip extra cells


		// NOTE: order of vertices matters! important for backface culling

		// T0
		uint16 indices[4];
		indices[0] = MAKE_INDEX(pos[0] + 1, pos[1], pos[2], sizeLog2);
		indices[1] = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		indices[2] = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2], sizeLog2);
		indices[3] = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);
		tetrahedra->quad(indices[0], indices[1], indices[2], indices[3]);

		// T1
		indices[0] = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);
		indices[1] = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		indices[2] = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2], sizeLog2);
		indices[3] = MAKE_INDEX(pos[0], pos[1] + 1, pos[2], sizeLog2);
		tetrahedra->quad(indices[0], indices[1], indices[2], indices[3]);

		// T2
		indices[0] = MAKE_INDEX(pos[0], pos[1] + 1, pos[2], sizeLog2);
		indices[1] = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		indices[2] = MAKE_INDEX(pos[0], pos[1] + 1, pos[2] + 1, sizeLog2);
		indices[3] = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);
		tetrahedra->quad(indices[0], indices[1], indices[2], indices[3]);

		// T3
		indices[0] = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		indices[1] = MAKE_INDEX(pos[0], pos[1], pos[2] + 1, sizeLog2);
		indices[2] = MAKE_INDEX(pos[0], pos[1] + 1, pos[2] + 1, sizeLog2);
		indices[3] = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);
		tetrahedra->quad(indices[0], indices[1], indices[2], indices[3]);

		// T4
		indices[0] = MAKE_INDEX(pos[0], pos[1], pos[2] + 1, sizeLog2);
		indices[1] = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		indices[2] = MAKE_INDEX(pos[0] + 1, pos[1], pos[2] + 1, sizeLog2);
		indices[3] = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);
		tetrahedra->quad(indices[0], indices[1], indices[2], indices[3]);

		// T5
		indices[0] = MAKE_INDEX(pos[0], pos[1], pos[2], sizeLog2);
		indices[1] = MAKE_INDEX(pos[0] + 1, pos[1], pos[2], sizeLog2);
		indices[2] = MAKE_INDEX(pos[0] + 1, pos[1], pos[2] + 1, sizeLog2);
		indices[3] = MAKE_INDEX(pos[0] + 1, pos[1] + 1, pos[2] + 1, sizeLog2);
		tetrahedra->quad(indices[0], indices[1], indices[2], indices[3]);
	}
	
	tetrahedra->end();
	return tetrahedra;
}