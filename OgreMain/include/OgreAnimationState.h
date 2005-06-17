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

#ifndef __AnimationSet_H__
#define __AnimationSet_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreController.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

	// Forward decl
	class AnimationStateSet;

    /** Represents the state of an animation and the weight of it's influence. 
    @remarks
        Other classes can hold instances of this class to store the state of any animations
        they are using.
    */
    class _OgreExport AnimationState 
    {
    public:
        /// Normal constructor with all params supplied
        AnimationState(const String& animName, AnimationStateSet *parent, 
			Real timePos, Real length, Real weight = 1.0, bool enabled = false);
		/// constructor to copy from an existing state with new parent
		AnimationState(AnimationStateSet* parent, const AnimationState &rhs);
		/** Destructor - is here because class has virtual functions and some compilers 
			would whine if it won't exist.
		*/
		virtual ~AnimationState();
        
        /// Gets the name of the animation to which this state applies
        const String& getAnimationName() const;
        /// Gets the time position for this animation
        Real getTimePosition(void) const;
        /// Sets the time position for this animation
        void setTimePosition(Real timePos);
        /// Gets the total length of this animation (may be shorter than whole animation)
        Real getLength() const;
        /// Sets the total length of this animation (may be shorter than whole animation)
        void setLength(Real len);
        /// Gets the weight (influence) of this animation
        Real getWeight(void) const;
        /// Sets the weight (influence) of this animation
        void setWeight(Real weight);
        /** Modifies the time position, adjusting for animation length
        @remarks
            This method loops at the edges if animation looping is enabled.
        */
        void addTime(Real offset);

        /// Returns true if this animation is currently enabled
        bool getEnabled(void) const;
        /// Sets whether this animation is enabled
        void setEnabled(bool enabled);

        /// Equality operator
        bool operator==(const AnimationState& rhs) const;
        // Inequality operator
        bool operator!=(const AnimationState& rhs) const;

        /** Sets whether or not an animation loops at the start and end of
            the animation if the time continues to be altered.
        */
        void setLoop(bool loop) { mLoop = loop; }
        /// Gets whether or not this animation loops            
        bool getLoop(void) const { return mLoop; }
     
        /** Copies the states from another animation state, preserving the animation name
        (unlike operator=) but copying everything else.
        @param animState Reference to animation state which will use as source.
        */
        void copyStateFrom(const AnimationState& animState);

		/// Get the parent animation state set
		AnimationStateSet* getParent(void) const { return mParent; }

    protected:
        String mAnimationName;
		AnimationStateSet* mParent;
        Real mTimePos;
        Real mLength;
        Real mInvLength;
        Real mWeight;
        bool mEnabled;
        bool mLoop;

    };

	// A map of animation states
	typedef std::map<String, AnimationState*> AnimationStateMap;
	typedef MapIterator<AnimationStateMap> AnimationStateIterator;
	typedef ConstMapIterator<AnimationStateMap> ConstAnimationStateIterator;

	/** Class encapsulating a set of AnimationState objects.
	*/
	class _OgreExport AnimationStateSet
	{
	public:
		/// Create a blank animation state set
		AnimationStateSet();
		/// Create an animation set by copying the contents of another
		AnimationStateSet(const AnimationStateSet& rhs);

		~AnimationStateSet();

		/** Create a new AnimationState instance. 
		@param animName The name of the animation
		@param timePos Starting time position
		@param length Length of the animation to play
		@param weight Weight to apply the animation with 
		@param enabled Whether the animation is enabled
		*/
		AnimationState* createAnimationState(const String& animName,  
			Real timePos, Real length, Real weight = 1.0, bool enabled = false);
		/// Get an animation state by the name of the animation
		AnimationState* getAnimationState(const String& name) const;
		/// Tests if state for the named animation is present
		bool hasAnimationState(const String& name) const;
		/// Remove animation state with the given name
		void removeAnimationState(const String& name);
		/// Remove all animation states
		void removeAllAnimationStates(void);

		/// Get an iterator over all the animation states in this set
		AnimationStateIterator getAnimationStateIterator(void);
		/// Get an iterator over all the animation states in this set
		ConstAnimationStateIterator getAnimationStateIterator(void) const;
		/// Copy the state of any matching animation states from this to another
		void copyMatchingState(AnimationStateSet* target);
		/// Has any animation state been altered since the last time resetDirty was called?
		bool isDirty(void) const { return mDirty; }
		/// Reset the dirty flag on this state set
		void resetDirty(void) { mDirty = false; }
		/// Set the dirty flag on this state set
		void _notifyDirty(void) { mDirty = true; }

	protected:
		bool mDirty;
		AnimationStateMap mAnimationStates;


	};


}

#endif

