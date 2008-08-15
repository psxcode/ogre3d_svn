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
#include <math.h>

namespace Ogre {


	const int MotionGraph::FOUND_NON = -1;
	const Real MotionGraph::State::UnrealTimePos = -1.0;
	const int MotionGraph::State::UnrealFrameIndex = -1;

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

	MotionGraph::Trigger* MotionGraph::State::GetTrigger(void)
	{
		if ( !mTriggers.empty() )
			return mTriggers.front();
		else
			return 0;
	}

	void MotionGraph::State::RemoveTopTrigger(void)
	{
		if ( !mTriggers.empty())
		{
			Trigger* trigger = mTriggers.front();
			mTriggers.pop();
			delete trigger; //truly remove the trigger
		}
	}

	void MotionGraph::State::RemoveAllTriggers(void)
	{
	    while ( !mTriggers.empty())
	    {
			Trigger* trigger = mTriggers.front();
			mTriggers.pop();
			delete trigger;
	    }
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

	MotionGraph::State::State(int stateid,MotionGraph* pMg,const String& statename)
		:mStateID(stateid),mStateName(statename),mOwnerMotionGraph(pMg),
		mStartTimePos(UnrealTimePos),mEndTimePos(UnrealTimePos),
		mStartFrameIndex(UnrealFrameIndex),mEndFrameIndex(UnrealFrameIndex),
		mIsActive(false)
	{

	}

	MotionGraph::State::State(int stateid,const String& statename )
		:mStateID(stateid),mStateName(statename),mOwnerMotionGraph(0),
		mStartTimePos(UnrealTimePos),mEndTimePos(UnrealTimePos),
		mStartFrameIndex(UnrealFrameIndex),mEndFrameIndex(UnrealFrameIndex),
		mIsActive(false)
	{

	}

	void MotionGraph::State::AddTrigger(Trigger* pTrigger)
	{
		mTriggers.push(pTrigger);
	}

	bool MotionGraph::State::HasLocomtionTrigger() const
	{
		Trigger* trigger = mTriggers.front();
		if ( trigger )
		{
			if ( trigger->CtrlInfo.speed != LOCOSPEED_IDLE 
				&& trigger->CtrlInfo.direct != LOCODIRECTION_CENTER )
			{
				return true;
			}
			else
				return false;
		}
		else
		{
			return false;
		}
	}

	void MotionGraph::State::AddTransition( Transition* pTran )
	{

		if ( pTran )
			mTransitions.insert(std::make_pair(pTran->GetProbability(),pTran));
	}

	void MotionGraph::State::ProcessAnimationEnded(const Entity* pEntity)
	{

		String ActionName = this->GetCurrentActionName();
		AnimationState* pAnimState = pEntity->getAnimationState(ActionName);
		assert(pAnimState);
		pAnimState->setTimePosition(0.0);//restart from the beginning,although cyclic motion
		// is not needed to do so
		pAnimState->setEnabled(false);
		// trigger is only been handled in ProcessTrigger()
		//this->RemoveTopTrigger();
		mIsActive = false;
	}

	void MotionGraph::State::EnableAnimation(const Ogre::Entity *pEntity)
	{
		String ActionName = this->GetCurrentActionName();
		AnimationState* pAnimState = pEntity->getAnimationState(ActionName);
		assert(pAnimState);
		//mIsActive = true;
		pAnimState->setEnabled(true);
		pAnimState->setLoop(false);
	}

	void MotionGraph::State::SetOneFootStep(const Entity* pEntity,unsigned short footindex,
		const FootStepDirection& footdirect)
	{

		
		String ActionName = this->GetCurrentActionName();
		AnimationState* pAnimState = pEntity->getAnimationState(ActionName);
		assert(pAnimState);

		//convert footindex from unsigned short frame index to Ogre::Real time pos
		Animation* anim = pEntity->getSkeleton()->getAnimation("wonder");
		NodeAnimationTrack* foottrack = anim->getNodeTrack(pEntity->getSkeleton()->getBone("lfoot")->getHandle());
		Ogre::Real timepos = (Real)footindex/foottrack->getNumKeyFrames();
		assert( timepos <= pAnimState->getLength() && timepos >= 0);

		pAnimState->setTimePosition(timepos);
		pAnimState->setEnabled(true);
		pAnimState->setLoop(false);

		this->SetStartTimePos(timepos,footindex);


		//now look for the next same foot step start time pos 
		// in fact, left and right foot steps by turns
		// by observing the statistic of gait, human beings seldom use the same foot
		// to step twice one after another, so it's surely safe to set the foot stand position
		// of the other foot to be the OneFootStep EndFrame
		Ogre::Real endtimepos;
		unsigned short endtimeindex;
		FootStandList::iterator it;
		switch (footdirect.foottype)
		{
		case MotionGraph::FOOTTYPE_LF_STAND:
			{
				it = upper_bound(mOwnerMotionGraph->mRFStandPoints.begin(),
					mOwnerMotionGraph->mRFStandPoints.end(),footindex);
				if ( it != mOwnerMotionGraph->mRFStandPoints.end() )
				{
					endtimepos = (Real)(*it)/foottrack->getNumKeyFrames();
					
				}
				break;
			}
		case MotionGraph::FOOTTYPE_RF_STAND:
			{
				it = upper_bound(mOwnerMotionGraph->mLFStandPoints.begin(),
					mOwnerMotionGraph->mLFStandPoints.end(),footindex);
				if ( it != mOwnerMotionGraph->mLFStandPoints.end() )
				{
					endtimepos = (Real)(*it)/foottrack->getNumKeyFrames();
					
					
				}
				break;
			}
		default:
			break;
		}
		assert( endtimepos <= pAnimState->getLength() && endtimepos >= 0);
		endtimeindex = *it;
		this->SetEndTimePos(endtimepos,endtimeindex);
		


		
	}


	bool MotionGraph::ConstructFromScript(const MotionGraphScript& mgScript)
	{

		if ( !mgScript.IsLoaded() )
			return false;

		for ( MotionGraphScript::StateMap::const_iterator it = mgScript.GetStates().begin();
			it != mgScript.GetStates().end(); it++)
		{
			State* pState = new State( it->first, this );
			pState->SetCurrentAction(it->second->GetCurrentActionName());
			mStates.insert(std::make_pair(pState->GetStateID(),pState));



		}
		mCurrentState = mStates.begin()->second;
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

	bool MotionGraph::ConstructDirectionalSubGraph(const Entity& entity)
	{
		AnimationState* animState = entity.getAnimationState("wonder");
		Animation* anim = 0;
		if ( animState )
		{
			anim = entity.getSkeleton()->getAnimation("wonder");
			SkeletonInstance* skelInstance = entity.getSkeleton();
			NodeAnimationTrack* lfoottrack = anim->getNodeTrack(skelInstance->getBone("lfoot")->getHandle());
			NodeAnimationTrack* rfoottrack = anim->getNodeTrack(skelInstance->getBone("rfoot")->getHandle());
			NodeAnimationTrack* hiptrack = anim->getNodeTrack(skelInstance->getBone("hip")->getHandle());

			for ( FootStandList::iterator it = mLFStandPoints.begin(); it != mLFStandPoints.end(); it++)
			{
				//current moment hip 
				TransformKeyFrame* hipkf = static_cast<TransformKeyFrame*>(hiptrack->getKeyFrame(*it));
				//next left foot stand point
				FootStandList::iterator nextit = it;
				nextit++;
				if ( nextit == mLFStandPoints.end())
				{
					break;
				}
				TransformKeyFrame* nextlfky = static_cast<TransformKeyFrame*>(lfoottrack->getKeyFrame(*(nextit)));
				Quaternion faceorientation = hipkf->getRotation();
				Ogre::Vector3 facedirection = faceorientation.zAxis();
				Ogre::Vector3 footdirection = nextlfky->getTranslate() - hipkf->getTranslate();
				footdirection.y = 0.;
				Ogre::Radian StepDirectionAngle = facedirection.angleBetween(footdirection); 
				mLFDirectTable.insert(std::make_pair(StepDirectionAngle.valueRadians(),*it));


			}
			for ( FootStandList::iterator it = mRFStandPoints.begin(); it != mRFStandPoints.end(); it++)
			{
				//current moment hip 
				TransformKeyFrame* hipkf = static_cast<TransformKeyFrame*>(hiptrack->getKeyFrame(*it));
				//next right foot stand point
				FootStandList::iterator nextit = it;
				nextit++;
				if ( nextit == mRFStandPoints.end())
				{
					break;
				}
				TransformKeyFrame* nextlfky = static_cast<TransformKeyFrame*>(rfoottrack->getKeyFrame(*(nextit)));
				Quaternion faceorientation = hipkf->getRotation();
				Ogre::Vector3 facedirection = faceorientation.zAxis();
				Ogre::Vector3 footdirection = nextlfky->getTranslate() - hipkf->getTranslate();
				footdirection.y = 0.;
				Ogre::Radian StepDirectionAngle = facedirection.angleBetween(footdirection); 
				mRFDirectTable.insert(std::make_pair(StepDirectionAngle.valueRadians(),*it));


			}


			return true;

		}
		else
		{
			return false;
		}
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

	void MotionGraph::AlignAnimations(Entity* pEntity)
	{
		AnimationStateSet* animations = pEntity->getAllAnimationStates();
		AnimationStateIterator it = animations->getAnimationStateIterator();
		while (it.hasMoreElements())
		{

			AnimationState* animstate= it.getNext();
			Animation* anim = pEntity->getSkeleton()->getAnimation(animstate->getAnimationName());
			Animation::NodeTrackIterator trackIter = anim->getNodeTrackIterator();
			TransformKeyFrame* CurrentKf = 0;
			TransformKeyFrame* PreKf = 0;

			while (trackIter.hasMoreElements())
			{
				NodeAnimationTrack* track = trackIter.getNext();
				Node* boneNode = track->getAssociatedNode();
				if (boneNode->getName() == "hip")
				{


					for ( int i = track->getNumKeyFrames() - 1; i >= 0; i-- )
					{

						CurrentKf = track->getNodeKeyFrame(i);
						//set the keyframe to be UsedRelative Coordinate
						// thus Interpolated Keyframe will take the "hip" boneNode's current
						// translation into account, so the global position of the Entity
						// will be in relative coordinate system
						//CurrentKf->SetRelativeCoordinate(true);

						Ogre::Vector3 TranslationOffset;
						if ( 0 == i)
						{

							TranslationOffset.x = 0;
							TranslationOffset.y = CurrentKf->getTranslate().y;
							TranslationOffset.z = 0;

						}
						else
						{
							PreKf = track->getNodeKeyFrame(i-1);
							TranslationOffset.x = CurrentKf->getTranslate().x - PreKf->getTranslate().x;
							TranslationOffset.y = CurrentKf->getTranslate().y;
							TranslationOffset.z = CurrentKf->getTranslate().z - PreKf->getTranslate().z;

						}
						//CurrentKf->setTranslate(TranslationOffset);
					}
				}

			}

		}

	}

	void MotionGraph::CalcKinematics(Entity* pEntity)
	{
		std::ofstream PositionFile;
		std::ofstream VelocityFile;
		std::ofstream AccelerationFile;
		PositionFile.open("position",std::ios::out);
		if ( !PositionFile.is_open())
		{
			exit(1);
		}
		else
			;
		VelocityFile.open("velocity",std::ios::out);
		if ( !VelocityFile.is_open())
		{
			exit(1);
		}
		else
			;
		AccelerationFile.open("acceleration",std::ios::out);
		if ( !AccelerationFile.is_open())
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
			std::vector<MotionAnnotation*> AnnotationList;
			KinematicElem* pkinematic = 0;
			MotionAnnotation* pAnnotation = 0;
			bool bMapReserve = false;
			Bone* bone = 0;
			int trackNum = 0;
			

			while (trackIter.hasMoreElements())
			{
				NodeAnimationTrack* track = trackIter.getNext();
				Node* boneNode = track->getAssociatedNode();
				trackNum = track->getNumKeyFrames();

				//preserve Kinemap data
				if( false == bMapReserve )
				{
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++ )
					{
						pkinematic = new KinematicElem;
						Kinemap.insert(std::make_pair(i,pkinematic));
						pAnnotation = new MotionAnnotation;
						pAnnotation->bLeftFootContact = false;
						pAnnotation->bRightFootContact = false;
						AnnotationList.push_back(pAnnotation);

					}
					bMapReserve = true;

					//calculate bone node's global position
					animstate->setEnabled(true);
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++ )
					{
						Matrix4 m;
						animstate->setTimePosition(track->getNodeKeyFrame(i)->getTime());
						pEntity->CalcBoneNodePositions();
						pkinematic = Kinemap[i];
						boneNode = anim->getNodeTrack(pEntity->getSkeleton()->getBone("lfoot")->getHandle())->getAssociatedNode();
						bone = pEntity->getSkeleton()->getBone("lfoot");
						
						
						pkinematic->LeftFootTranslation = bone->_getFullTransform().getTrans();

						boneNode = anim->getNodeTrack(pEntity->getSkeleton()->getBone("rfoot")->getHandle())->getAssociatedNode();
						bone = pEntity->getSkeleton()->getBone("rfoot");
						
						pkinematic->RightFootTranslation = bone->_getFullTransform().getTrans();
					}
					animstate->setEnabled(false);
					
				}


				bone = pEntity->getSkeleton()->getBone(track->getHandle());
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
				

				}//end for "hip"
				if (bone->getName() == "lfoot")
				{
					
					//calculate velocity first

					CalcBoneNodeKinematic(track,Kinemap,bone);
					
					//annotate the motion clip
					Ogre::Real MinVertical = 100;
					unsigned short MinIndex = -1;
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++)
					{
						if( MinVertical > Kinemap[i]->LeftFootTranslation.y)
						{
							MinVertical = Kinemap[i]->LeftFootTranslation.y;
							MinIndex = i;
						}
					}
					Ogre::Real VerticalThreshold = 0.05;

					
					if ( animstate->getAnimationName() == "wonder" )//this animation needs to be treated specially because of its
						// lowest vertical changed so drastically
					{
						MinVertical = 100;
						bool bInTrough = false;
						bool bStartInTrough = false;
						Ogre::Real ThresholdLine = -10.; //this is gotten by observation of the graph
						if (Kinemap[0]->LeftFootTranslation.y > ThresholdLine)
						{
							bInTrough = false;
						}
						else
						{
							bStartInTrough = true;
							bInTrough = true;
						}
						unsigned short leftbound = -1;
						unsigned short rightbound = -1;
						for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++)
						{
							
							if ( Kinemap[i]->LeftFootTranslation.y > ThresholdLine)
							{
								if ( bInTrough == false)
								{
									AnnotationList[i]->bLeftFootContact = false;
									continue;
								}
								else
								{
									bInTrough = false;
									if ( bStartInTrough == true )
									{
										bStartInTrough = false;
										continue;
									}
									else
									rightbound = i;
								}
							}
							else
							{
								if ( bInTrough == false )
								{
									bInTrough = true;
									leftbound = i;
									if( MinVertical > Kinemap[i]->LeftFootTranslation.y)
									{
										MinVertical = Kinemap[i]->LeftFootTranslation.y;
										MinIndex = i;
									}
								}
								else
								{
									if ( bStartInTrough == true )
									{
										//If the trough is not wholly evaluated, discard the part which maybe contain
										// the ture min point or not
										AnnotationList[i]->bLeftFootContact = false;
									}
									else
									{
										if( MinVertical > Kinemap[i]->LeftFootTranslation.y)
										{
											MinVertical = Kinemap[i]->LeftFootTranslation.y;
											MinIndex = i;
										}
									}
								}
								continue;

							}
							mLFStandPoints.insert(i);//now we set the lowest point to be the stand point
							for ( unsigned short j = leftbound; j <= rightbound; j++)
							{
								if ( abs(Kinemap[j]->LeftFootTranslation.y - MinVertical) < VerticalThreshold )
								{
									AnnotationList[j]->bLeftFootContact = true;
								}
								else
								{
									AnnotationList[j]->bLeftFootContact = false;
								}
							}
							MinVertical = 100;
						}
					}
					else
					{
						for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++)
						{
							if ( abs(Kinemap[i]->LeftFootTranslation.y - MinVertical) < VerticalThreshold )
							{
								AnnotationList[i]->bLeftFootContact = true;
							}
							else
							{
								AnnotationList[i]->bLeftFootContact = false;
							}
						}
					}
					
					//add annotation for leftfoot contact

				}//end for lfoot
				if ( bone->getName() == "rfoot")
				{
					
					//calculate velocity first
					CalcBoneNodeKinematic(track,Kinemap,bone);
					//annotate the motion clip
					Ogre::Real MinVertical = 100;
					unsigned short MinIndex = -1;
					for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++)
					{
						if( MinVertical > Kinemap[i]->RightFootTranslation.y)
						{
							MinVertical = Kinemap[i]->RightFootTranslation.y;
							MinIndex = i;
						}
					}
					Ogre::Real VerticalThreshold = 0.05;

					

					if ( animstate->getAnimationName() == "wonder" )//this animation needs to be treated specially because of its
						// lowest vertical changed so drastically
					{
						MinVertical = 100;
						bool bInTrough = false;
						bool bStartInTrough = false;
						Ogre::Real ThresholdLine = -11.; //this is gotten by observation of the graph
						if (Kinemap[0]->RightFootTranslation.y > ThresholdLine)
						{
							bInTrough = false;
						}
						else
						{
							bStartInTrough = true;
							bInTrough = true;
						}
						unsigned short leftbound = -1;
						unsigned short rightbound = -1;
						for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++)
						{

							if ( Kinemap[i]->RightFootTranslation.y > ThresholdLine)
							{
								if ( bInTrough == false)
								{
									AnnotationList[i]->bRightFootContact = false;
									continue;
								}
								else
								{
									bInTrough = false;
									if ( bStartInTrough == true )
									{
										bStartInTrough = false;
										continue;
									}
									else
										rightbound = i;
								}
							}
							else
							{
								if ( bInTrough == false )
								{
									bInTrough = true;
									leftbound = i;
									if( MinVertical > Kinemap[i]->RightFootTranslation.y)
									{
										MinVertical = Kinemap[i]->RightFootTranslation.y;
										MinIndex = i;
									}
								}
								else
								{
									if ( bStartInTrough == true )
									{
										//If the trough is not wholly evaluated, discard the part which maybe contain
										// the ture min point or not
										AnnotationList[i]->bRightFootContact = false;
									}
									else
									{
										if( MinVertical > Kinemap[i]->RightFootTranslation.y)
										{
											MinVertical = Kinemap[i]->RightFootTranslation.y;
											MinIndex = i;
										}
									}
								}
								continue;

							}
							mRFStandPoints.insert(i);//now we set the lowest point to be the stand point
							for ( unsigned short j = leftbound; j <= rightbound; j++)
							{
								if ( abs(Kinemap[j]->RightFootTranslation.y - MinVertical) < VerticalThreshold )
								{
									AnnotationList[j]->bRightFootContact = true;
								}
								else
								{
									AnnotationList[j]->bRightFootContact = false;
								}
							}
							MinVertical = 100;
						}
					}
					else
					{
						for ( unsigned short i = 0; i < track->getNumKeyFrames(); i++)
						{
							if ( abs(Kinemap[i]->RightFootTranslation.y - MinVertical) < VerticalThreshold )
							{
								AnnotationList[i]->bRightFootContact = true;
							}
							else
							{
								AnnotationList[i]->bRightFootContact = false;
							}
						}
					}

					//add rightfoot contact annotation

				}//end for rightfoot


				
				
				mKinematicData.insert(std::make_pair(animstate->getAnimationName(),Kinemap));
				mAnnotationData.insert(std::make_pair(animstate->getAnimationName(),AnnotationList));
			}//end "for" animation track

			for ( unsigned short i = 0; i < trackNum; i++  )
			{
				if ( 0 == i)
				{
					PositionFile<<anim->getName()<<std::endl;
					PositionFile<<trackNum<<std::endl;
					VelocityFile<<anim->getName()<<std::endl;
					VelocityFile<<trackNum<<std::endl;
					AccelerationFile<<anim->getName()<<std::endl;
					AccelerationFile<<trackNum<<std::endl;

				}

				PositionFile<<Kinemap[i]->RightFootTranslation.y<<std::endl;
				VelocityFile<<AnnotationList[i]->bLeftFootContact<<std::endl;
				AccelerationFile<<AnnotationList[i]->bRightFootContact<<std::endl;

				if ( i == trackNum - 1)
				{
					PositionFile<<"\n"<<std::endl;
					VelocityFile<<"\n"<<std::endl;
					AccelerationFile<<"\n"<<std::endl;
				}

			}
		}//end "while" animations

	}

	void MotionGraph::CalcBoneNodeKinematic(const NodeAnimationTrack* track,std::map<float, KinematicElem*>& Kinemap, const Bone* bone)
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

			if ( i < track->getNumKeyFrames() - 2 )
			{
				NextKf = track->getNodeKeyFrame(i+1);
				Ogre::Real t1 = NextKf->getTime() - CurrentKf->getTime();
				assert( t1 > 0.00001f );
				ThirdKf = track->getNodeKeyFrame(i+2);
				Ogre::Real t2 = ThirdKf->getTime() - NextKf->getTime();
				assert( t2 > 0.00001f );
				if ( bone->getName() == "lfoot")
				{
					pkinematic->velocity = (Kinemap[i+1]->LeftFootTranslation - Kinemap[i]->LeftFootTranslation)/t1;
				}else if( bone->getName() == "rfoot")
				{
					pkinematic->velocity = (Kinemap[i+1]->RightFootTranslation - Kinemap[i]->RightFootTranslation)/t1;
				}

			}
			else
			{
				pkinematic->velocity = Kinemap[i-1]->velocity;
			}
		}
		//calculate the acceleration of bone node, the acceleration only care about the trend of velocities' magnitude
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

	Ogre::Real MotionGraph::State::GetEndTimePosition(void) const
	{ 
		return mEndTimePos; 
	}


	void MotionGraph::ProcessTrigger(const Entity* pEntity)
	{
		Trigger* trigger = 0;
		if ( mCurrentState )
		{
			if (mCurrentState->GetTrigger())
			{
				trigger = new Trigger(*mCurrentState->GetTrigger());
			}
			if ( trigger)
			{
				//once the trigger has been processed, it must be removed in each "case"
				//mCurrentState->RemoveTopTrigger();
				switch ( trigger->mType )
				{
				case NON_TRIGGER:
					break;
				case ANIMATION_END:
					{
						
						mCurrentState->ProcessAnimationEnded(pEntity);
						//if the current state is to be transited to another one
						// its all triggers must be removed, for state doesn't has memory
						// of interactive control commands
						mCurrentState->RemoveAllTriggers();
						Transit(pEntity);
						mCurrentState->EnableAnimation(pEntity);
						break;
					}
				case DIRECTION_CONTROL:
					{	//DIRECTION_CONTROL will cause characters transit from
						// current state to the next state based on current speed and sth else like height

						//first check if current state is "wonder",
						//if it is "wonder", check further if current action's direction
						// is the same as the trigger's, if so skip this trigger this time,
						if ( mCurrentState->GetCurrentActionName() == "wonder" )
						{
							if( mCurrentState->GetCurrentActionDirection() == trigger->CtrlInfo.direct )
							{
								mCurrentState->RemoveTopTrigger();
							}
							else
								//keep this different direction trigger then when the current step
								//has been completed, new direction foot step can be taken immediately 
							{
								;
							}
						}
						else
						//transit current state to "wonder" state, because LOCOMOTION
						//motion is in "wonder"
						{
							mCurrentState->ProcessAnimationEnded(pEntity);
							mCurrentState->RemoveAllTriggers();
							TransitToState("wonder");

							//then the most important part of interactive locomotion control
							//select matching directional motion clips in "wonder" 
							FootStepDirection footdirect;
							mCurrentState->SetCurrentActionDirection(trigger->CtrlInfo.direct);
							switch (trigger->CtrlInfo.direct)
							{
							case LOCODIRECTION_FORWARD:
								{
									footdirect.foottype = FOOTTYPE_LF_STAND;
									footdirect.rad = 0;
									break;
								}
							case LOCODIRECTION_BACKWARD:
								{
									footdirect.foottype = FOOTTYPE_RF_STAND;
									footdirect.rad = Math::PI;
									break;
								}
							case LOCODIRECTION_LEFT:
								{
									footdirect.foottype = FOOTTYPE_LF_STAND;
									footdirect.rad = Math::PI/2;
									break;
								}
							case LOCODIRECTION_RIGHT:
								{
									footdirect.foottype = FOOTTYPE_LF_STAND;
									footdirect.rad = Math::PI*1.5;
									break;
								}
							}
							//prepare the next foot step
							int footindex = LookForFootStep(footdirect);
							if ( FOUND_NON != footindex )
							{
								mCurrentState->SetOneFootStep(pEntity,footindex,footdirect);
							}
							mCurrentState->StitchMotion(pEntity);
							mCurrentState->EnableAnimation(pEntity);
						}

						//ActionName = mCurrentState->GetCurrentActionName();
						//pAnimState = pEntity->getAnimationState(ActionName);
						//pAnimState->setEnabled(true);
						//pAnimState->setLoop(false);

						// Do motion interpolation
						break;
					}
				case SPEED_CHANGE:
					break;
				case ACTION_IDLE://if current state is idle, do nothing,
					// otherwise change current state to idle regardless which action type current
					//state is
					{

						if ( mCurrentState->GetCurrentActionName() != "idle")
						{
							mCurrentState->ProcessAnimationEnded(pEntity);
							mCurrentState->RemoveAllTriggers();
							TransitToState("idle");
							mCurrentState->EnableAnimation(pEntity);
						}
						else
							;
						break;
					}
				}


				delete trigger;
			}// trigger
			
		}//mCurrentState
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

	void MotionGraph::State::StitchMotion(const Entity* pEntity)
	{

		//only when within the current state there is subgraph
		// this state is set to be active for relative coordinate system
		// processing
		mIsActive = true;
		//calculate how angle in radian the start frame needs to rotate
		String ActionName = this->GetCurrentActionName();
		AnimationState* pAnimState = pEntity->getAnimationState(ActionName);
		Animation* anim = pEntity->getSkeleton()->getAnimation(pAnimState->getAnimationName());
		Bone* bonenode = pEntity->getSkeleton()->getBone("hip");

		NodeAnimationTrack* track = anim->getNodeTrack(pEntity->getSkeleton()->getBone("hip")->getHandle());
		TransformKeyFrame* CurrentKf = 0;
		TransformKeyFrame* PreKf = 0;

		//set the "hip" bonenode to be UsedRelative Coordinate system
		// thus Interpolated Keyframe will take the "hip" boneNode's current
		// translation and orientation into account, so the global position of the Entity
		// will be in relative coordinate system
		
		

		if( this->GetEndTimePosition() > this->GetStartTimePos() )
		{
			bonenode->SetRelativeCoordinate(true);

			Ogre::Vector3 LatestGlobalTranslation = pEntity->getSkeleton()->getRootBone()->getPosition();
			bonenode->setLatestGlobalPosition(LatestGlobalTranslation);

			CurrentKf = track->getNodeKeyFrame(this->GetStartFrameIndex());
			bonenode->setRelativeStartPosition(CurrentKf->getTranslate());

			//calculate the align rotation of the first frame of this state's current foot step
			Ogre::Quaternion faceorientation = pEntity->getSkeleton()->getRootBone()->getInitialOrientation();


			Ogre::Quaternion StartFrameOrientation = CurrentKf->getRotation();
			Ogre::Vector3 facedirection = faceorientation.zAxis();
			Ogre::Vector3 StartFrameDirection = StartFrameOrientation.zAxis();
			
			Ogre::Quaternion alignrotation = StartFrameDirection.getRotationTo(facedirection); 
			bonenode->setAlignOrientation(alignrotation);
		}
		

		
		//for ( int i = track->getNumKeyFrames() - 1; i >= 0; i-- )
		//{

		//	CurrentKf = track->getNodeKeyFrame(i);
		//	
		//	

		//	Ogre::Vector3 TranslationOffset;
		//	if ( 0 == i)
		//	{

		//		TranslationOffset.x = 0;
		//		TranslationOffset.y = CurrentKf->getTranslate().y;
		//		TranslationOffset.z = 0;

		//	}
		//	else
		//	{
		//		PreKf = track->getNodeKeyFrame(i-1);
		//		TranslationOffset.x = CurrentKf->getTranslate().x - PreKf->getTranslate().x;
		//		TranslationOffset.y = CurrentKf->getTranslate().y;
		//		TranslationOffset.z = CurrentKf->getTranslate().z - PreKf->getTranslate().z;

		//	}
		//	//CurrentKf->setTranslate(TranslationOffset);
		//}

		pAnimState->setLoop(false);
        
		
	}

	void MotionGraph::Transit(const Entity* entity)
	{
		
		

		Transition* pTran =	mCurrentState->GetBestTransition();
		if ( pTran )
		{
			

			mCurrentState = pTran->GetToState();//mCurrentState will have the StartTimeIndex and EndTimeIndex of
			// an animation
		}
		//mCurrentState->StitchMotion(entity,translation,orientation);
		
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


	bool MotionGraph::TransitToState(const Ogre::String &stateName)
	{
		for( StateMap::iterator it = mStates.begin(); it != mStates.end(); it++)
		{
			if( it->second )
			{
				if ( stateName == it->second->GetCurrentActionName() )
				{
					mCurrentState = it->second;
					return true;
				}

			}
			else
				continue;
		}
		return false;
	}

	bool MotionGraphScript::IsLoaded() const
	{
		return (strcmp(mScriptName.c_str(),""))?true:false;
	}

	int MotionGraph::LookForFootStep(const FootStepDirection& footdirect) const
	{
		DirectionLookupTable::const_iterator it;
		switch (footdirect.foottype)
		{
		case FOOTTYPE_LF_STAND:
			{

				it = mLFDirectTable.lower_bound(footdirect.rad);
				if ( it != mLFDirectTable.end())
				{
				return it->second;
				}
				else
				{
					return mLFDirectTable.rbegin()->second;
				}
				break;
			}
		case FOOTTYPE_RF_STAND:
			{
				it = mRFDirectTable.lower_bound(footdirect.rad);
				if ( it != mRFDirectTable.end())
				{
					return it->second;
				}
				else
				{
					return mRFDirectTable.rbegin()->second;
				}
				break;
			}
		case FOOTTYPE_BOTHF_STAND:
			{
				break;
			}
		default:
			;
		}
		return FOUND_NON;
	}


}