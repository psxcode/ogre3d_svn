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
#ifndef __MotionGraph_H__
#define __MotionGraph_H__

#include "OgrePrerequisites.h"

namespace Ogre {

	/** A proxy class of Actor's behavior policy set .
	@remarks
	This class defines the interface an actor can access to determine how it Act(). 

	Actor acts as the AI tells, commonly  via controlling the state of Actor's m_Entity and m_SceneNode,
	especially is to set AnimationState.

	@note
	It is an abstract class, you should subclass it to create your instance class.

	It is designed to utilize the following features:
	Motion graphs and variants        
	Motion interpolation and blending
	Behavior-based graphs and search trees (similar to move trees)
	Motion controllers learnt from existing data or based on physics 
	Finite State Machine based AI

	*/



	class _OgreExport MotionGraph : public Resource
	{
	public:
		/** A State is a character action unit, it keeps steady until some trigger pulses.
		p.s. Trigger will be another class not created at this level of character animation.
		@remarks
		State can be reference to node in classical motion graph implementation,
		but it extends that. State can have more character information, e.g. position,
		orientation,velocity, acceleration,height and so on, if dynaimc is added, force will be
		another varible of state.
		@note
		The differences of State in Ogre compared to other motion graph nodes
		State's basic function is handling AnimationState in Ogre, animations will be
		loaded in preprocess or on demand in unitive ResourceManager.

		/************************************************************************/
		class State
		{
		public:
			State();
			~State();


		};


		class Transition
		{
		public:
			Transition();
			~Transition();

		};

		MotionGraph();
		~MotionGraph();


	};

}

#endif