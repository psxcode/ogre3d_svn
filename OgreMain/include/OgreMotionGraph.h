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
#include "OgreAnimationState.h"
#include <fstream>
#include <deque>

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

	//class forward declaration
	class MotionGraphScript;


	class _OgreExport MotionGraph //: public AnimationStateSet
	{

		friend class State;
		friend class Transition;
	public:
//some common data structures used between motion graph and other program modules
		

		enum LocomotionSpeed
		{
			LOCOSPEED_IDLE = 0,
			LOCOSPEED_WALK,
			LOCOSPEED_RUN
		};
		enum LocomotionDirection
		{
			LOCODIRECTION_NON = -1,
			LOCODIRECTION_CENTER = 0,
			LOCODIRECTION_FORWARD, 
			LOCODIRECTION_BACKWARD, 
			LOCODIRECTION_LEFT,
			LOCODIRECTION_RIGHT
		};
		enum ActionType
		{
			ACTIONTYPE_LOCOMOTION = 0,
			ACTIONTYPE_JUMP,
			ACTIONTYPE_DANCE
		};

		//there maybe various of foot configuration
		// such as 
		// left foot stand or both feet stand
		// flight phase
		// left foot behind right foot  .etc
		enum FootType
		{
			FOOTTYPE_LF_STAND,
			FOOTTYPE_RF_STAND,
			FOOTTYPE_BOTHF_STAND,
			FOOTTYPE_FLIGHT
		};

		//control command package to transfer interactive control
		// commands to motion graph 
		struct InteractiveControlInfo
		{
			ActionType type;
			LocomotionSpeed speed;
			LocomotionDirection direct;
			InteractiveControlInfo():type(ACTIONTYPE_LOCOMOTION),speed(LOCOSPEED_IDLE),
			direct(LOCODIRECTION_CENTER){}
		};

		struct FootStepDirection
		{
			Ogre::Real rad;
			FootType foottype;
		};

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
		class Transition;
		/*
		Triggers are events that will cause a state transition.
		Triggers are designed to work as interrupts, and the there so must be
		a interrupt handling module.
		Many events can create Trigger, and assign them to States, just like PC's devices
		shot interrupts and these interrupts are sent to Interrupt Controller, the Controller
		select the highest priority level interrupt, then CPU, in Ogre is the MotionGraph,
		take action based on current state status.
		*/
		enum TriggerType
		{
			NON_TRIGGER,
			ANIMATION_END,
			DIRECTION_CONTROL,
			SPEED_CHANGE,
			ACTION_IDLE
		};

		static TriggerType TriggerNameToTriggerType(const String& triggertype);

		class Trigger
		{
		public:
			Trigger(TriggerType type = NON_TRIGGER):mType(type){};
			Trigger(const Trigger& trigger):mType(trigger.mType)
			{
				CtrlInfo.direct = trigger.CtrlInfo.direct;
				CtrlInfo.speed = trigger.CtrlInfo.speed;
				CtrlInfo.type = trigger.CtrlInfo.type;
			}
			virtual ~Trigger(){};
			TriggerType mType;
			InteractiveControlInfo CtrlInfo;

		};

		class State //: public AnimationState
		{
		public:
			State(int stateid,MotionGraph* pMg,const String& statename = ""):mStateID(stateid),mStateName(statename),
				mStartFrameTranslation(Ogre::Vector3::ZERO),mStartFrameRotation(Ogre::Quaternion::ZERO),
			mOwnerMotionGraph(pMg){};
			// for motion graph script state constructor
			State(int stateid,const String& statename = ""):mStateID(stateid),mStateName(statename),
				mStartFrameTranslation(Ogre::Vector3::ZERO),mStartFrameRotation(Ogre::Quaternion::ZERO),
				mOwnerMotionGraph(0){};
			virtual ~State(){};
			typedef std::queue<Trigger*> TriggerQueue;
			typedef std::map<float,Transition*> TransitionMap;
			int GetStateID(void) const { return mStateID; }
			void AddTrigger( Trigger* pTrigger);
			void AddTransition( Transition* pTran );
			/** When an animation arrives its end, call this 
			*/
			void ProcessAnimationEnded(const Entity* pEntity);
			/** When new State is transited to, its animation is enabled by call this
			*/
			void EnableAnimation(const Entity* pEntity); 

			/** When a new footstep is ready to carry, its animation state is set
			@remarks
			footindex is the frame index in animation track,
			when animation state is set, footindex must be converted to time value in Ogre::Real
			*/
			void SetOneFootStep(const Entity* pEntity,unsigned short footindex,const FootStepDirection& footdirect); 

			/** Get the first trigger in the trigger queue
			*/
			Trigger* GetTrigger(void);
			/** @Remove a trigger from the trigger queue
			*/
			void RemoveTopTrigger(void);
			Transition* GetBestTransition(void);

			typedef std::set<String> ActionSet;
			String GetCurrentActionName(void) const { return mCurrentAction.actionName; }
			LocomotionDirection GetCurrentActionDirection(void) const
			{
				return mCurrentAction.direction;
			}
			void SetCurrentAction(const String& actionname) { mCurrentAction.actionName = actionname; }
			void StitchMotion(const Entity* pEntity, const Ogre::Vector3& StartFrameTranslation,const Ogre::Quaternion& StartFrameOrientation );
			/// Gets the time position in original motion for this state
			Ogre::Real GetEndTimePosition(void) const;

			/** Set state's end time pos in its associated animation 
			if end time pos has been reached,a trigger of type "
			*/
			void SetEndTimePos(Ogre::Real timepos) 
			{
				mEndTimePos = timepos;
			}

			/** Set state's start time pos in its associated animation 
			*/
			void SetStartTimePos(Ogre::Real timepos)
			{
				mStartTimePos = timepos;
			}

			Ogre::Real GetStartTimePos(void) const
			{
				return mStartTimePos;
			}

			Ogre::Real GetEndTimePos(void) const
			{
				return mEndTimePos;
			}

			struct Action
			{
				String actionName;
				LocomotionDirection direction;
				Action(void):actionName(""),direction(LOCODIRECTION_NON){}
			};
			
		protected:
			String  mStateName;
			ActionSet mActions;
			Action  mCurrentAction;
			int		mStateID;
			/** Every State starts from a TimePos in an Animation and ends at another TimePos in the same animation 
			@remarks
			mStartTimePos and mEndTimePos are determined by transition's 
			*/
			Ogre::Real mStartTimePos;
			Ogre::Real mEndTimePos;

			TriggerQueue mTriggers; // it is a priority queue
			TransitionMap mTransitions; // always select transition with the maximum probability,
			// the probability is refreshed every time the a trigger is fired
			/**  The first frame of this Motion Graph State should be aligned to the latest translation
			@remarks
			the following frames' global translations are calculated with
			(K' translation - 1st' translation)*mStartFrameRotation + mStartFrameTranslation 
			*/
			Ogre::Vector3 mStartFrameTranslation;
			/** The first frame of this Motion Graph State should be rotated to match the character face 
			direction
			@remarks
			this variable's value is the radian angle how this State's first frame needs to rotate to the latest 
			character's face direction
			*/
			Ogre::Quaternion mStartFrameRotation;
			MotionGraph* mOwnerMotionGraph;

		};

		class Transition
		{
		public:
			Transition():mProbability(1.){};
			Transition(const Transition& rhs);
			virtual ~Transition(){};
			void AddFromState( State* pState) { mFromState = pState; }
			void AddToState( State* pState ) { mToState = pState; }
			void SetActionName( const String& actionName ) { mActionName = actionName; }
			void SetTriggerType( const String& triggertype );
			float GetProbability() const { return mProbability; }
			State* GetToState() { return mToState; }
			State* GetFromState() { return mFromState; }
		protected:
			State* mFromState;
			State* mToState;
			String mActionName;
			TriggerType mTriggerType;
			float  mProbability; //the transition probability to select it as the next tranistion of
			// the state

		};

		MotionGraph(const String& mgName):mMotionGraphName(mgName),mCurrentState(0){};


		/** Construct a motion graph using a motion graph script
		@remarks
		There are a lot of methods to construct a motion graph, referencing to much literature
		@param
		mgscript is a script describing the schema of how a motion graph is to be constructed
		*/
		bool ConstructFromScript(const MotionGraphScript& mgScript);

		/** Construct a subgraph that can give directional control interactively 
		this capability is achieved by adding a motion asset including a lot of locomotion with
		frequent turns 
		@remarks 
		this step is processed after motion graph has constructed from a motion graph script already,
		and then the specified "wonder" motion will be checked whether DirectionalSubGraph can be constructed
		this step also requires CalcKinematics is done already, as directional information needs kinematics
		and foot contact specification
		*/
		bool ConstructDirectionalSubGraph(const Entity& entity);

		/** Motion Graph has its trigger lists, regardless where these triggers come from,
		it is allowed to check the motion graph's trigger list to see whether some triggers
		must be processed.
		*/
		void ProcessTrigger(const Entity* pEntity);

		/** Calculate the kinematics attributes of the entity this motion graph is bound to
		@remarks
		Now only using velocity and acceleration to assist in blending algorithm
		@param
		pEntity is the Entity owning all the animations this motion graph is used
		*/
		void CalcKinematics(Entity* pEntity); 

		/** Convert root animation track's translation from global to relative coordinate,
		the start frame's translation is set to zero, the following ones have offset calculated,
		so precede motion and the successive motion can stitch together
		@remarks
		only x,z coordinate is concerned, vertical value is also used new coming motion's to 
		avoid accumulated vertical artifact. So jerk will sometimes be seen, if transition blending
		is improved, this is sure to be improved
		@param
		pEntity is the Entity owning all the animations this motion graph is used
		*/
		void AlignAnimations(Entity* pEntity); 

		/** Transit from the current state to the specified state.
		@params stateName is the name of the state to be transited to
		@remarks
		if no matching name state exists, current state will loop
		*/
		bool TransitToState(const String& stateName);

		/** Transit from the current state to the next state.
		@remarks
		some status variables are set, some triggers are fired.
		*/
		void Transit(const Entity* entity);

		State* GetCurrentState(void) { return mCurrentState; }

		/** Look for the best matching step to take.
		@remarks
		The lookuping process if based on the current feet configuration 
		and current direction and speed
		
		*/
		int LookForFootStep(const FootStepDirection& footdirect) const;


		~MotionGraph();
		typedef std::map<int,State*> StateMap;
		typedef std::vector<Transition*> TransitionArray;
		struct KinematicElem
		{
			Ogre::Vector3 HipTranslation;
			Ogre::Vector3 LeftFootTranslation;
			Ogre::Vector3 RightFootTranslation;
			Ogre::Quaternion orientation;
			Ogre::Vector3 velocity;       //this velocity is temporarily used for calculating any joint's velocity
			Ogre::Vector3 acceleration;   //this acceleration is temporarily used for any joint to 
			// determine the acceleration zero-crossings, it only cares about magnitude of velocity
		};
		struct MotionAnnotation
		{
			bool bLeftFootContact;
			bool bRightFootContact;

		};
		typedef std::map< std::string, std::map<float, KinematicElem*> > Kinematic; 
		typedef std::map< std::string, std::vector<MotionAnnotation*> > Annotations;
		/**  The key element is the orientation of a foot after one step has been just taken
			 the range of key value is 0 to 2*PI
			 the value element is the time pos of an animation at which a foot will step at a position
			 rotating this value of radian from the character's face direction
			 @remakes
			 the direction table is based totally on one animation, its is assumed to be the animation named
			 "wonder" for script usage.
		*/
		typedef std::map< Ogre::Real, unsigned short > DirectionLookupTable;
		typedef std::set< unsigned short > FootStandList;
	protected:
		void CalcBoneNodeKinematic(const NodeAnimationTrack* track,std::map<float, KinematicElem*>& Kinemap, const Bone* bone);

	protected:
		StateMap mStates;
		TransitionArray mTransitions;
		Kinematic mKinematicData;
		Annotations mAnnotationData;
		String mMotionGraphName;
		/// the state this motion graph is currently in
		State*	mCurrentState;
		DirectionLookupTable mLFDirectTable;
		DirectionLookupTable mRFDirectTable;
		FootStandList mLFStandPoints;
		FootStandList mRFStandPoints;

		static const int FOUND_NON;


	};

	/** A motion graph script is somewhat a Finite State Machine
	It defines a semantic as following
	States:
	1,[default action, accepted input triggers,...]
	2,[default action, accepted input triggers,...]
	3,[default action, accepted input triggers,...]
	...
	Transitions:
	1,'AnimationEnd',walk,2
	2,'AnimationEnd',run,3
	3,'AnimationEnd',shot,1

	The States section is a list of States.
	The Transitions section is a Finite State Machine Table
	CurrentState  InputTrigger   Action    NextState
	1         'turnleft'    turn left     2
	@remarks
	The first version of motion graph script is a native file that
	read directly using standardized fstream, once it works, a Resource
	version of motion graph script is to be implemented
	@note
	Now this script is only for motion graph construction usage.
	More functionalities are to be added to AI script.
	And the script now is loaded when its parent skeleton is loaded synchronously,

	The final goal of using script is to integrate a script language virtual machine
	into Ogre, e.g. Python, then adding new AI scripts and modifying exsiting scripts
	don't need to rebuild the source code of Ogre or its applications
	/************************************************************************/

	class _OgreExport MotionGraphScript //: public Resource
	{
	public:
		MotionGraphScript():mScriptName(""){};
		virtual ~MotionGraphScript();
		// this is an alpha version of script reader
		bool Load(const String& MgScriptName);
		bool IsLoaded() const;
		typedef std::map<int,MotionGraph::State*> StateMap;
		typedef std::vector<MotionGraph::Transition*> TransitionArray;
		const StateMap& GetStates() const { return mStates; }
		const TransitionArray& GetTransitions() const { return mTransitions; }
	protected:
		String mScriptName;
		StateMap mStates;
		TransitionArray mTransitions;



	};

	enum MotionGraphScriptChunkID
	{
		MG_HEADER      =   0x1000,
		MG_STATES      =   0x2000,
		//////////////////////////////////////////////////////////////////////////
		// repeated section of state
		//
		MG_TRANSITIONS  =   0x3000
	};


}

#endif