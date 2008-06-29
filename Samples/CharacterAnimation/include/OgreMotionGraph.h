
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