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

namespace Ogre {


	MotionGraph::TriggerType MotionGraph::TriggerNameToTriggerType(const String& triggertype)
	{
		if ( !strcmp(triggertype.c_str(),"AnimationEnd"))
		{
			return ANIMATION_END;
		}

		return NON_TRIGGER;
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
				std::getline(ifMgScript,line);
				//now just go through, there is nothing in States
				MotionGraph::State* pState = new MotionGraph::State(i);

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

	void MotionGraph::Transition::SetTriggerType( const String& triggertype )
	{
		
			mTriggerType = TriggerNameToTriggerType(triggertype);

	}

	void MotionGraph::State::AddTrigger(Trigger* pTrigger)
	{
		mTriggers.push(pTrigger);
	}


	bool MotionGraph::Construct(const MotionGraphScript& mgScript)
	{

		if ( !mgScript.IsLoaded() )
			return false;

		for ( MotionGraphScript::StateMap::const_iterator it = mgScript.GetStates().begin();
			it != mgScript.GetStates().end(); it++)
		{
			State* pState = new State( it->first );
			mStates.insert(std::make_pair(pState->GetStateID(),pState));



		}
		for ( MotionGraphScript::TransitionArray::const_iterator it = mgScript.GetTransitions().begin();
			it != mgScript.GetTransitions().end(); it++)
		{
			static_cast<MotionGraph::Transition*>(*it);
			mTransitions.push_back(*it);

		}
		return true;
	}
}