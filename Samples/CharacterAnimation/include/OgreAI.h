
#ifndef __AI_H__
#define __AI_H__

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
	class _OgreExport AI_Base
	{
	public:
		AI_Base();
		virtual ~AI_Base();

		void Initialize() = 0;
		void Execute() = 0;
	};

	class _OgreExport AI_Motiongraph : public AI_Base
	{
	public:
		virtual ~AI_Motiongraph();
	protected:
		AI_Motiongraph():
	public:
		AI_Motiongraph(const SkeletonInstance& skelInstance);
	protected:
		MotionGraph* m_pMotionGraph;
	};

	AI_Motiongraph::AI_Motiongraph()
	{
		m_pMotionGraph = new MotionGraph(const SkeletonInstance& SkeletonInstance);
	}
}

#endif