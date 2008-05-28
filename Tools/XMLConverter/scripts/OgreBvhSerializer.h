/*
-----------------------------------------------------------------------------
  2008-05-27 created
  Lucas Westine
-----------------------------------------------------------------------------
*/

#ifndef __BvhSerializer_H__
#define __BvhSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreSkeleton.h"
#include "OgreSerializer.h"

namespace Ogre {

	/** Class for serialising bvh , which is added into Ogre .skeleton as animation section
	@par
	To add a bvh:<OL>
	<LI>Create a Bvh object and populate it using it's methods.</LI>
	<LI>Call the exportSkeleton method</LI>
	</OL>
	*/
	class _OgreExport BvhSerializer : public Serializer
	{
	public:
		SkeletonSerializer();
		virtual ~SkeletonSerializer();


		/** Exports a skeleton to the file specified. 
		@remarks
		This method takes an externally created Skeleton object, and exports both it
		and animations it uses to a .skeleton file.
		@param pSkeleton Weak reference to the Skeleton to export
		@param filename The destination filename
		@param endianMode The endian mode to write in
		*/
		void exportSkeleton(const Skeleton* pSkeleton, const String& filename,
			Endian endianMode = ENDIAN_NATIVE);

		/** Imports Skeleton and animation data from a .skeleton file DataStream.
		@remarks
		This method imports data from a DataStream opened from a .skeleton file and places it's
		contents into the Skeleton object which is passed in. 
		@param stream The DataStream holding the .skeleton data. Must be initialised (pos at the start of the buffer).
		@param pDest Weak reference to the Skeleton object which will receive the data. Should be blank already.
		*/
		void importSkeleton(DataStreamPtr& stream, Skeleton* pDest);

		// TODO: provide Cal3D importer?

	protected:
		// Internal export methods
		void writeSkeleton(const Skeleton* pSkel);
		void writeBone(const Skeleton* pSkel, const Bone* pBone);
		void writeBoneParent(const Skeleton* pSkel, unsigned short boneId, unsigned short parentId);
		void writeAnimation(const Skeleton* pSkel, const Animation* anim);
		void writeAnimationTrack(const Skeleton* pSkel, const NodeAnimationTrack* track);
		void writeKeyFrame(const Skeleton* pSkel, const TransformKeyFrame* key);
		void writeSkeletonAnimationLink(const Skeleton* pSkel, 
			const LinkedSkeletonAnimationSource& link);

		// Internal import methods
		void readBone(DataStreamPtr& stream, Skeleton* pSkel);
		void readBoneParent(DataStreamPtr& stream, Skeleton* pSkel);
		void readAnimation(DataStreamPtr& stream, Skeleton* pSkel);
		void readAnimationTrack(DataStreamPtr& stream, Animation* anim, Skeleton* pSkel);
		void readKeyFrame(DataStreamPtr& stream, NodeAnimationTrack* track, Skeleton* pSkel);
		void readSkeletonAnimationLink(DataStreamPtr& stream, Skeleton* pSkel);

		size_t calcBoneSize(const Skeleton* pSkel, const Bone* pBone);
		size_t calcBoneSizeWithoutScale(const Skeleton* pSkel, const Bone* pBone);
		size_t calcBoneParentSize(const Skeleton* pSkel);
		size_t calcAnimationSize(const Skeleton* pSkel, const Animation* pAnim);
		size_t calcAnimationTrackSize(const Skeleton* pSkel, const NodeAnimationTrack* pTrack);
		size_t calcKeyFrameSize(const Skeleton* pSkel, const TransformKeyFrame* pKey);
		size_t calcKeyFrameSizeWithoutScale(const Skeleton* pSkel, const TransformKeyFrame* pKey);
		size_t calcSkeletonAnimationLinkSize(const Skeleton* pSkel, 
			const LinkedSkeletonAnimationSource& link);




	};

}


#endif
