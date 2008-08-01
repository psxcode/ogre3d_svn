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
#include "OgreSkeletonInstance.h"
#include "OgreEntity.h"
#include "OgreAnimation.h"
#include "OgreBone.h"

namespace Ogre {


	MotionGraph::TriggerType MotionGraph::TriggerNameToTriggerType(const String& triggertype)
	{
		if ( !strcmp(triggertype.c_str(),"AnimationEnd"))
		{
			return ANIMATION_END;
		}else if ( !strcmp(triggertype.c_str(),"DirectionControl"))
		{
			return DIRECTION_CONTROL;
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

	void MotionGraph::State::RemoveTopTrigger()
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

	MotionGraph::~MotionGraph()
	{
		for ( Kinematic::iterator it = mKinematicData.begin(); it != mKinematicData.end(); it++)
		{
			for( std::map<float, KinematicElem*>::iterator iter = it->second.begin(); iter != it->second.end(); iter++ )
			{	
				if ( 0 != iter->second)
				{
					delete iter->second;
				}
			}
		}
		for ( Annotations::iterator it = mAnnotationData.begin(); it != mAnnotationData.end(); it++)
		{
			for ( std::vector<MotionAnnotation*>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
			{
				if ( 0 != *iter)
				{
					delete *iter;
				}
			}
		}
	}

	void MotionGraph::CalcKinematics(const Entity* pEntity)
	{
		std::ofstream kinefile;
		kinefile.open("acceleration",std::ios::out);
		if ( !kinefile.is_open())
		{
			exit(1);
		}
		else
			;

		AnimationStateSet* animations = pEntity->getAllAnimationStates();
		AnimationStateIterator it = animations->getAnimationStateIterator();
		while (it.hasMoreElements())
		{
			AnimationState* animstate= it.getNext();
			Animation* anim = pEntity->getSkeleton()->getAnimation(animstate->getAnimationName());
			Animation::NodeTrackIterator trackIter = anim->getNodeTrackIterator();
			std::map<float, KinematicElem*> Kinemap;
			KinematicElem* pkinematic = 0;
			bool bMapReserve = false;



			while (trackIter.hasMoreElements())
			{
				NodeAnimationTrack* track = trackIter.getNext();
				Node* boneNode = track->getAssociatedNode();


				//preserve Kinemap data
				if( false == bMapReserve )
				{
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++ )
					{
						pkinematic = new KinematicElem;
						Kinemap.insert(std::make_pair(i,pkinematic));
					}
					bMapReserve = true;
				}


				Bone* bone = pEntity->getSkeleton()->getBone(track->getHandle());
				//do calculation for this animation track


				TransformKeyFrame* CurrentKf = 0;


				if (bone->getName() == "hip")
				{

					//calculate velocity first
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++ )
					{
						pkinematic = Kinemap[i];
						CurrentKf = track->getNodeKeyFrame(i);
						TransformKeyFrame* NextKf = 0;
						TransformKeyFrame* ThirdKf = 0;
						if ( i < track->getNumKeyFrames() - 1 )
						{
							NextKf = track->getNodeKeyFrame(i+1);
							Ogre::Real t1 = NextKf->getTime() - CurrentKf->getTime();
							assert( t1 > 0.00001f );
							pkinematic->velocity = (NextKf->getTranslate() - CurrentKf->getTranslate())/t1;

						}
						else
						{
							pkinematic->velocity = Kinemap[i-1]->velocity;
						}

						pkinematic->HipTranslation = CurrentKf->getTranslate();
						pkinematic->orientation = CurrentKf->getRotation();

						/*	if ( i < track->getNumKeyFrames() - 2 )
						{
						NextKf = track->getNodeKeyFrame(i+1);
						ThirdKf = track->getNodeKeyFrame(i+2);
						Ogre::Real t1 = NextKf->getTime() - CurrentKf->getTime();
						Ogre::Real t2 = ThirdKf->getTime() - NextKf->getTime();
						assert( t1 > 0.00001f );
						assert( t2 > 0.00001f );
						pkinematic->velocity = (NextKf->getTranslate() - CurrentKf->getTranslate())/t1;

						}
						else
						{
						pkinematic->velocity = Kinemap[i-1]->velocity;
						}*/





					}
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++  )
					{

						pkinematic = Kinemap[i];
						if ( i < track->getNumKeyFrames() - 1 )
						{

							Ogre::Real t1 = track->getNodeKeyFrame(i+1)->getTime() - track->getNodeKeyFrame(i)->getTime();
							assert( t1 > 0.00001f );
							pkinematic->acceleration = (Kinemap[i+1]->velocity - pkinematic->velocity)/t1;

						}
						else
						{
							pkinematic->acceleration = Kinemap[i-1]->acceleration;
						}

					}

				}//end for "hip"
				if (bone->getName() == "lfoot")
				{
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++ )
					{
						pkinematic = Kinemap[i];
						CurrentKf = track->getNodeKeyFrame(i);
						boneNode->translate(CurrentKf->getTranslate());
						pkinematic->LeftFootTranslation = boneNode->getPosition();
					}
					//calculate velocity first


					CalcAnimationTrackKinematic(track,Kinemap);


					//add annotation for leftfoot contact

				}//end for lfoot
				if ( bone->getName() == "rfoot")
				{
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++ )
					{
						pkinematic = Kinemap[i];
						CurrentKf = track->getNodeKeyFrame(i);
						boneNode->translate(CurrentKf->getTranslate());
						pkinematic->RightFootTranslation = boneNode->getPosition();
					}
					//calculate velocity first
					CalcAnimationTrackKinematic(track,Kinemap);


					//add rightfoot contact annotation

				}//end for rightfoot
				if ( bone->getName() == "lfoot")
				{
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++  )
					{
						if ( 0 == i)
						{
							kinefile<<anim->getName()<<std::endl;
							kinefile<<track->getNumKeyFrames()<<std::endl;
						}


						kinefile<<Kinemap[i]->LeftFootTranslation.y<<std::endl;

						if ( i == track->getNumKeyFrames() - 1)
						{
							kinefile<<"\n"<<std::endl;
						}

					}
				}
				mKinematicData.insert(std::make_pair(animstate->getAnimationName(),Kinemap));
			}
		}

	}

	void MotionGraph::CalcAnimationTrackKinematic(const NodeAnimationTrack* track,std::map<float, KinematicElem*>& Kinemap)
	{
		//calculate velocity first
		KinematicElem* pkinematic = 0;
		TransformKeyFrame* CurrentKf = 0;
		TransformKeyFrame* NextKf = 0;
		TransformKeyFrame* ThirdKf = 0;
		for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++ )
		{
			pkinematic = Kinemap[i];
			CurrentKf = track->getNodeKeyFrame(i);

			if ( i < track->getNumKeyFrames() - 1 )
			{
				NextKf = track->getNodeKeyFrame(i+1);
				Ogre::Real t1 = NextKf->getTime() - CurrentKf->getTime();
				assert( t1 > 0.00001f );
				pkinematic->velocity = (NextKf->getTranslate() - CurrentKf->getTranslate())/t1;

			}
			else
			{
				pkinematic->velocity = Kinemap[i-1]->velocity;
			}
		}
		for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++  )
		{

			pkinematic = Kinemap[i];
			if ( i < track->getNumKeyFrames() - 1 )
			{

				Ogre::Real t1 = track->getNodeKeyFrame(i+1)->getTime() - track->getNodeKeyFrame(i)->getTime();
				assert( t1 > 0.00001f );
				pkinematic->acceleration = (Kinemap[i+1]->velocity - pkinematic->velocity)/t1;

			}
			else
			{
				pkinematic->acceleration = Kinemap[i-1]->acceleration;
			}

		}

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
						mCurrentState->RemoveTopTrigger();
						Transit(pEntity);
						ActionName = mCurrentState->GetCurrentActionName();
						pAnimState = pEntity->getAnimationState(ActionName);
						pAnimState->setEnabled(true);
						pAnimState->setLoop(false);
						break;
					}
				case DIRECTION_CONTROL:
					{	//DIRECTION_CONTROL will cause characters transit from
						// current state to the next state based on current speed and sth else like height
						String ActionName = mCurrentState->GetCurrentActionName();
						AnimationState* pAnimState = pEntity->getAnimationState(ActionName);
						pAnimState->setTimePosition(0.0);//restart from the beginning,although cyclic motion
						// is not needed to do so
						pAnimState->setEnabled(false);
						mCurrentState->RemoveTopTrigger();

						Transit(pEntity);
						ActionName = mCurrentState->GetCurrentActionName();
						pAnimState = pEntity->getAnimationState(ActionName);
						pAnimState->setEnabled(true);
						pAnimState->setLoop(false);

						// Do motion interpolation
						break;
					}
				case SPEED_CHANGE:
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

	void MotionGraph::Transit(const Entity* entity)
	{
		Transition* pTran =	mCurrentState->GetBestTransition();
		if ( pTran )
		{
			mCurrentState = pTran->GetToState();
		}
		// insert the blending animation clip
		/*
		Animation* anim = skel->getAnimation(animationName);
		Animation::NodeTrackIterator trackIter = anim->getNodeTrackIterator();
		while (trackIter.hasMoreElements())
		{
		NodeAnimationTrack* track = trackIter.getNext();

		TransformKeyFrame oldKf(0, 0);
		track->getInterpolatedKeyFrame(mAnimChop, &oldKf);

		// Drop all keyframes after the chop
		while (track->getKeyFrame(track->getNumKeyFrames()-1)->getTime() >= mAnimChop - mAnimChopBlend)
		track->removeKeyFrame(track->getNumKeyFrames()-1);

		TransformKeyFrame* newKf = track->createNodeKeyFrame(mAnimChop);
		TransformKeyFrame* startKf = track->getNodeKeyFrame(0);

		Bone* bone = skel->getBone(track->getHandle());
		if (bone->getName() == "Spineroot")
		{
		mSneakStartOffset = startKf->getTranslate() + bone->getInitialPosition();
		mSneakEndOffset = oldKf.getTranslate() + bone->getInitialPosition();
		mSneakStartOffset.y = mSneakEndOffset.y;
		// Adjust spine root relative to new location
		newKf->setRotation(oldKf.getRotation());
		newKf->setTranslate(oldKf.getTranslate());
		newKf->setScale(oldKf.getScale());


		}
		else
		{
		newKf->setRotation(startKf->getRotation());
		newKf->setTranslate(startKf->getTranslate());
		newKf->setScale(startKf->getScale());
		}
		}
		*/
	}

	bool MotionGraphScript::IsLoaded() const
	{
		return (strcmp(mScriptName.c_str(),""))?true:false;
	}

}