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
			DIRECTION_CONTROL
		};

		static TriggerType TriggerNameToTriggerType(const String& triggertype);

		class Trigger
		{
		public:
			Trigger(TriggerType type = NON_TRIGGER):mType(type){};
			virtual ~Trigger(){};

		protected:
			TriggerType mType;

		};

		class State //: public AnimationState
		{
		public:
			State(int stateid, const String& statename = ""):mStateID(stateid),mStateName(statename){};
			virtual ~State(){};
			typedef std::queue<Trigger*> TriggerQueue;
			typedef std::map<float,Transition*> TransitionMap;
			int GetStateID() const { return mStateID; }
			void AddTrigger( Trigger* pTrigger);

		protected:
			String  mStateName;
			int		mStateID;
			TriggerQueue mTriggers; // it is a priority queue
			TransitionMap mTransitions; // always select transition with the maximum probability,
			// the probability is refreshed every time the a trigger is fired
		};

		class Transition
		{
		public:
			Transition(){};
			virtual ~Transition(){};
			void AddFromState( State* pState) { mFromState = pState; }
			void AddToState( State* pState ) { mToState = pState; }
			void SetActionName( const String& actionName ) { mActionName = actionName; }
			void SetTriggerType( const String& triggertype );
		protected:
			State* mFromState;
			State* mToState;
			String mActionName;
			TriggerType mTriggerType;

		};
	
		MotionGraph(const String& mgName):mMotionGraphName(mgName){};


		/** Construct a motion graph using a motion graph script
		@remarks
		There are a lot of methods to construct a motion graph, referencing to much literature
		@par
		Note that this method automatically generates a handle for the bone, which you
		can retrieve using Bone::getHandle. If you wish the new Bone to have a specific
		handle, use the alternate form of this method which takes a handle as a parameter,
		although you should note the restrictions.
		*/
		bool Construct(const MotionGraphScript& mgScript);
		/** Motion Graph has its trigger lists, regardless where these triggers come from,
		it is allowed to check the motion graph's trigger list to see whether some triggers 
		must be processed.
		*/
		void CheckTrigger();
		/** Tranist from current state to next state.
		@remarks
		some status maybe set, some triggers maybe fired.
		*/
		void Transit();


		~MotionGraph();
		typedef std::map<int,State*> StateMap;
		typedef std::vector<Transition*> TransitionArray;
	protected:
		StateMap mStates;
		TransitionArray mTransitions;
		String mMotionGraphName;


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
		virtual ~MotionGraphScript(){};
		// this is an alpha version of script reader
		bool Load(const String& MgScriptName);
		bool IsLoaded() const { return (strcmp(mScriptName.c_str(),""))?false:true;}
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