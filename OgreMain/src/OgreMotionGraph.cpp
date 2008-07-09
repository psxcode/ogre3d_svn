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

#include "OgreStableHeaders.h"
#include "OgreMotionGraph.h"
#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include "OgreStringConverter.h"
#include "OgreEntity.h"

namespace Ogre {


	MotionGraph::TriggerType MotionGraph::TriggerNameToTriggerType(const String& triggertype)
	{
		if ( !strcmp(triggertype.c_str(),"AnimationEnd"))
		{
			return ANIMATION_END;
		}

		return NON_TRIGGER;
	}

	MotionGraphScript::~MotionGraphScript()
	{
		for ( StateMap::iterator it = mStates.begin(); it != mStates.end(); it++)
		{
			if ( it->second)
			{
				delete it->second;
			}
		}
		for (TransitionArray::iterator it = mTransitions.begin(); it != mTransitions.end(); it++)
		{
			if ( *it )
			{
				delete *it;
			}
		}

	}

	bool MotionGraphScript::Load(const String& MgScriptName)
	{

		std::ifstream ifMgScript(MgScriptName.c_str(),std::ios::in);
		if ( !ifMgScript.is_open())
		{
			return false;
		}
		// Already loaded a script, now is reload
		if ( strcmp(mScriptName.c_str(),"") )
		{
			mStates.clear();
			mTransitions.clear();
		}

		mScriptName = MgScriptName;
		std::string line;

		std::getline(ifMgScript,line);
		std::stringstream ss(line);
		std::string str;
		ss >> str;
		if( !strcmp(str.c_str(),"States:") )
		{
			int stateCount;
			ss >> stateCount;
			for ( int i = 0; i < stateCount; i++)
			{
				int StateId;
				std::getline(ifMgScript,line);
				std::vector<String> stateParams = StringUtil::split(line,",\n");
				std::vector<String>::iterator it = stateParams.begin();
				StateId = StringConverter::parseInt(*it++);
				String actionname = *it++;
				//now just go through, there is nothing in States
				MotionGraph::State* pState = new MotionGraph::State(StateId);
				pState->SetCurrentAction(actionname);

				mStates.insert(std::make_pair<int,MotionGraph::State*>(pState->GetStateID(),pState));
			}			
		}
		else
			return false;

		std::getline(ifMgScript,line);
		ss.clear();
		ss.str(line);
		ss >> str;
		if ( !strcmp(str.c_str(),"Transitions:") )
		{
			int TransitionCount;
			ss >> TransitionCount;
			for ( int i = 0; i < TransitionCount; i++)
			{
				std::getline(ifMgScript,line);
				MotionGraph::Transition* pTran = new MotionGraph::Transition;

				std::vector<String> transitionParam = StringUtil::split(line,",\n");
				int FromStateID,ToStateID;
				std::vector<String>::iterator it = transitionParam.begin();
				FromStateID = StringConverter::parseInt(*it++);
				String triggername = *it++;
				pTran->SetTriggerType(triggername);
				pTran->SetActionName(*it++);
				ToStateID = StringConverter::parseInt(*it++);

				MotionGraph::State* pState = (*mStates.find(FromStateID)).second;
				MotionGraph::TriggerType triggertype = MotionGraph::TriggerNameToTriggerType(triggername);
				MotionGraph::Trigger* pTrigger = new MotionGraph::Trigger(triggertype);

				pState->AddTrigger(pTrigger);
				pTran->AddFromState((*mStates.find(FromStateID)).second);
				pTran->AddToState((*mStates.find(ToStateID)).second);
				mTransitions.push_back(pTran);


			}


		}
		return true;

	}

	MotionGraph::Trigger* MotionGraph::State::GetTrigger()
	{
		if ( !mTriggers.empty() )
			return mTriggers.front();
		else
			return 0;
	}

	void MotionGraph::State::RemoveTrigger()
	{
		if ( !mTriggers.empty())
			mTriggers.pop();
	}


	MotionGraph::Transition::Transition(const Ogre::MotionGraph::Transition &rhs):
	mFromState(rhs.mFromState),mToState(rhs.mToState),mActionName(rhs.mActionName),
		mTriggerType(rhs.mTriggerType),mProbability(rhs.mProbability)
	{

	}

	void MotionGraph::Transition::SetTriggerType( const String& triggertype )
	{

		mTriggerType = TriggerNameToTriggerType(triggertype);

	}

	void MotionGraph::State::AddTrigger(Trigger* pTrigger)
	{
		mTriggers.push(pTrigger);
	}

	void MotionGraph::State::AddTransition( Transition* pTran )
	{

		if ( pTran )
			mTransitions.insert(std::make_pair(pTran->GetProbability(),pTran));
	}

	bool MotionGraph::Construct(const MotionGraphScript& mgScript)
	{

		if ( !mgScript.IsLoaded() )
			return false;

		for ( MotionGraphScript::StateMap::const_iterator it = mgScript.GetStates().begin();
			it != mgScript.GetStates().end(); it++)
		{
			State* pState = new State( it->first );
			pState->SetCurrentAction(it->second->GetCurrentActionName());
			mStates.insert(std::make_pair(pState->GetStateID(),pState));



		}
		mCurrentState = mStates.rbegin()->second;
		for ( MotionGraphScript::TransitionArray::const_iterator it = mgScript.GetTransitions().begin();
			it != mgScript.GetTransitions().end(); it++)
		{
			//copy constructor
			Transition* pTran = new MotionGraph::Transition(*static_cast<MotionGraph::Transition*>(*it));
			mTransitions.push_back(pTran);
			// use new Transitions and new States
			pTran->AddFromState(mStates.find(pTran->GetFromState()->GetStateID())->second);
			pTran->AddToState(mStates.find(pTran->GetToState()->GetStateID())->second);
			pTran->GetFromState()->AddTransition(pTran);


		}
		return true;
	}

	void MotionGraph::ProcessTrigger(const Entity* pEntity)
	{
		Trigger* trigger;
		if ( mCurrentState )
		{
			trigger = mCurrentState->GetTrigger();
			if ( trigger)
			{
				switch ( trigger->mType )
				{
				case NON_TRIGGER:
					break;
				case ANIMATION_END:
					{
						String ActionName = mCurrentState->GetCurrentActionName();
						AnimationState* pAnimState = pEntity->getAnimationState(ActionName);
						pAnimState->setTimePosition(0.0);//restart from the beginning
						pAnimState->setEnabled(false);
						mCurrentState->RemoveTrigger();
						Transit();
						ActionName = mCurrentState->GetCurrentActionName();
						pAnimState = pEntity->getAnimationState(ActionName);
						pAnimState->setEnabled(true);
						pAnimState->setLoop(false);
						break;
					}
				case DIRECTION_CONTROL:
					// Do motion interpolation in the same state
					break;

				}
			}
		}
	}

	MotionGraph::Transition* MotionGraph::State::GetBestTransition()
	{
		if ( mTransitions.empty() )
		{
			return 0;
		}
		else
		{
			return mTransitions.rbegin()->second;
		}

	}

	void MotionGraph::Transit()
	{
		Transition* pTran =	mCurrentState->GetBestTransition();
		if ( pTran )
		{
			mCurrentState = pTran->GetToState();
		}


	}

	bool MotionGraphScript::IsLoaded() const
	{
		return (strcmp(mScriptName.c_str(),""))?true:false;
	}

}