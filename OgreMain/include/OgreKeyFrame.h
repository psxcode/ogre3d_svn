/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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

#ifndef __KeyFrame_H__
#define __KeyFrame_H__

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"
#include "OgreAny.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreIteratorWrappers.h"

namespace Ogre 
{

    /** A key frame in an animation sequence defined by an AnimationTrack.
    @remarks
        This class can be used as a basis for all kinds of key frames. 
        The unifying principle is that multiple KeyFrames define an 
        animation sequence, with the exact state of the animation being an 
        interpolation between these key frames. 
    */
    class _OgreExport KeyFrame
    {
    public:

        /** Default constructor, you should not call this but use AnimationTrack::createKeyFrame instead. */
        KeyFrame(const AnimationTrack* parent, Real time);

		virtual ~KeyFrame() {}

        /** Gets the time of this keyframe in the animation sequence. */
        virtual Real getTime(void) const;


    protected:
        Real mTime;
        const AnimationTrack* mParentTrack;
    };


	/** Specialised KeyFrame which stores any numeric value.
	*/
	class _OgreExport NumericKeyFrame : public KeyFrame
	{
	public:
		/** Default constructor, you should not call this but use AnimationTrack::createKeyFrame instead. */
		NumericKeyFrame(const AnimationTrack* parent, Real time);
		~NumericKeyFrame() {}

		/** Get the value at this keyframe. */
		virtual const AnyNumeric& getValue(void) const;
		/** Set the value at this keyframe.
		@remarks
			All keyframe values must have a consistent type. 
		*/
		virtual void setValue(const AnyNumeric& val);

	protected:
		AnyNumeric mValue;
	};


	/** Specialised KeyFrame which stores a full transform. */
	class _OgreExport TransformKeyFrame : public KeyFrame
	{
	public:
		/** Default constructor, you should not call this but use AnimationTrack::createKeyFrame instead. */
		TransformKeyFrame(const AnimationTrack* parent, Real time);
		~TransformKeyFrame() {}
		/** Sets the translation associated with this keyframe. 
		@remarks    
		The translation factor affects how much the keyframe translates (moves) it's animable
		object at it's time index.
		@param trans The vector to translate by
		*/
		virtual void setTranslate(const Vector3& trans);

		/** Gets the translation applied by this keyframe. */
		const Vector3& getTranslate(void) const;

		/** Sets the scaling factor applied by this keyframe to the animable
		object at it's time index.
		@param scale The vector to scale by (beware of supplying zero values for any component of this
		vector, it will scale the object to zero dimensions)
		*/
		virtual void setScale(const Vector3& scale);

		/** Gets the scaling factor applied by this keyframe. */
		virtual const Vector3& getScale(void) const;

		/** Sets the rotation applied by this keyframe.
		@param rot The rotation applied; use Quaternion methods to convert from angle/axis or Matrix3 if
		you don't like using Quaternions directly.
		*/
		virtual void setRotation(const Quaternion& rot);

		/** Gets the rotation applied by this keyframe. */
		virtual const Quaternion& getRotation(void) const;
	protected:
		Vector3 mTranslate;
		Vector3 mScale;
		Quaternion mRotate;


	};



	/** Specialised KeyFrame which stores absolute vertex positions for a complete
		buffer, designed to be interpolated with other keys in the same track. 
	*/
	class _OgreExport VertexMorphKeyFrame : public KeyFrame
	{
	public:
		/** Default constructor, you should not call this but use AnimationTrack::createKeyFrame instead. */
		VertexMorphKeyFrame(const AnimationTrack* parent, Real time);
		~VertexMorphKeyFrame() {}
		/** Sets the vertex buffer containing the source positions for this keyframe. 
		@remarks    
			We assume that positions are the first 3 float elements in this buffer,
			although we don't necessarily assume they're the only ones in there.
		@param buf Vertex buffer link; will not be modified so can be shared
			read-only data
		*/
		void setVertexBuffer(const HardwareVertexBufferSharedPtr& buf);

		/** Gets the vertex buffer containing positions for this keyframe. */
		const HardwareVertexBufferSharedPtr& getVertexBuffer(void) const;

	protected:
		HardwareVertexBufferSharedPtr mBuffer;

	};

	/** Specialised KeyFrame which stores offsets for a subset of the vertices 
		in a buffer to provide a blendable pose, designed to be interpolated with
		other animations.
	*/
	class _OgreExport VertexPoseKeyFrame : public KeyFrame
	{
	public:
		/** Default constructor, you should not call this but use AnimationTrack::createKeyFrame instead. */
		VertexPoseKeyFrame(const AnimationTrack* parent);
		~VertexPoseKeyFrame() {}

		/// A collection of vertex offsets based on the vertex index
		typedef std::map<size_t, Vector3> VertexOffsetMap;
		/// An iterator over the vertex offsets
		typedef MapIterator<VertexOffsetMap> VertexOffsetIterator;
		/// An iterator over the vertex offsets
		typedef ConstMapIterator<VertexOffsetMap> ConstVertexOffsetIterator;

		/** Adds an offset to a vertex for this pose. 
		@param index The vertex index
		@param offset The position offset for this pose
		*/
		void addVertex(size_t index, const Vector3& offset);

		/** Remove a vertex offset. */
		void removeVertex(size_t index);

		/** Clear all vertex offsets. */
		void clearVertexOffsets(void);

		/** Gets an iterator over all the vertex offsets. */
		ConstVertexOffsetIterator getVertexOffsetIterator(void) const;
		/** Gets an iterator over all the vertex offsets. */
		VertexOffsetIterator getVertexOffsetIterator(void);
		/** Gets a const reference to the vertex offsets. */
		const VertexOffsetMap& getVertexOffsets(void) const { return mVertexOffsetMap; }

		/** Get a hardware vertex buffer version of the vertex offsets. */
		const HardwareVertexBufferSharedPtr& _getHardwareVertexBuffer(size_t numVertices);

	protected:
		/// Primary storage, sparse vertex use
		VertexOffsetMap mVertexOffsetMap;
		/// Derived hardware buffer, covers all vertices
		HardwareVertexBufferSharedPtr mBuffer;

	};

}


#endif

