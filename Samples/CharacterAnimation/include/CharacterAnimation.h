/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
To fix inconsistent newline ending error.
Filename:    CharacterAnimation.h
Description: Specialisation of OGRE's framework application to show autonomous character animations.

An autonomous character will include the following features:

Motion graphs and variants
Motion interpolation and blending
Behavior-based graphs and search trees (similar to move trees)
Motion controllers learnt from existing data or based on physics
Finite State Machine based AI

-----------------------------------------------------------------------------
*/

#include "ExampleApplication.h"
#include "OgreMotionGraph.h"


AnimationState* mAnimState;
SceneNode* mSceneNode;
Entity *ent;
const std::string modelname = "character";
const std::string animationName = "pushup.bvh";

// Event handler to animate
class CharacterAnimationFrameListener : public ExampleFrameListener
{
protected:
	//If interactive control is active, character's x-z plane moving direction is repesented by this variable
	MotionGraph::LocomotionDirection mLocomotionDirection;
	//When a character is locomoting, its speed is set by interactive input, and kept by this variable
	MotionGraph::LocomotionSpeed mLocomotionSpeed;

public:
	CharacterAnimationFrameListener(RenderWindow* win, Camera* cam, const std::string &debugText)
		: ExampleFrameListener(win, cam),mLocomotionSpeed(MotionGraph::LOCOSPEED_IDLE),
		mLocomotionDirection(MotionGraph::LOCODIRECTION_CENTER)
	{
		mDebugText = debugText;
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		if( ExampleFrameListener::frameRenderingQueued(evt) == false )
			return false;

		
		if ( mLocomotionDirection != MotionGraph::LOCODIRECTION_CENTER )
		{
			if ( mLocomotionSpeed == MotionGraph::LOCOSPEED_IDLE )
			{
				mLocomotionSpeed = MotionGraph::LOCOSPEED_WALK;
			}
		}

		MotionGraph::InteractiveControlInfo ControlInfo;
		ControlInfo.direct = mLocomotionDirection;
		ControlInfo.speed = mLocomotionSpeed;
		if ( IsAnimated() )
			ent->AdvanceMotionGraphTime(evt.timeSinceLastFrame*2,ControlInfo);
		else
			;
		//mAnimState->addTime(evt.timeSinceLastFrame*2);
		return true;
	}

	bool processUnbufferedKeyInput(const FrameEvent& evt)
	{
		

		using namespace OIS;
		if ( mKeyboard->isKeyDown(KC_1) && mTimeUntilNextToggle <= 0)
			mLocomotionSpeed = MotionGraph::LOCOSPEED_WALK;
		else if ( mKeyboard->isKeyDown(KC_2) && mTimeUntilNextToggle <= 0)
			mLocomotionSpeed = MotionGraph::LOCOSPEED_RUN;
		else if ( mKeyboard->isKeyDown(KC_Y) && mTimeUntilNextToggle <= 0)
			mLocomotionDirection = MotionGraph::LOCODIRECTION_FORWARD;
		else if ( mKeyboard->isKeyDown(KC_B) && mTimeUntilNextToggle <= 0)
			mLocomotionDirection = MotionGraph::LOCODIRECTION_BACKWARD;
		else if ( mKeyboard->isKeyDown(KC_G) && mTimeUntilNextToggle <= 0)
			mLocomotionDirection = MotionGraph::LOCODIRECTION_LEFT;
		else if ( mKeyboard->isKeyDown(KC_J) && mTimeUntilNextToggle <= 0)
			mLocomotionDirection = MotionGraph::LOCODIRECTION_RIGHT;
		else
			mLocomotionDirection = MotionGraph::LOCODIRECTION_CENTER;

		if ( mKeyboard->isKeyDown(KC_T) && mTimeUntilNextToggle <= 0)
		{
			//relocate character to initializing position
		//	Vector3 currentPosition = mSceneNode->getPosition();

			Vector3 currentPosition = ent->getSkeleton()->getRootBone()->getPosition();
			mSceneNode->setPosition(-currentPosition.x,currentPosition.y,-currentPosition.z);
		}

		return ExampleFrameListener::processUnbufferedKeyInput(evt);

	}
};



class CharacterApplication : public ExampleApplication
{
public:
	CharacterApplication() {}

protected:
	std::string mDebugText;

	// Just override the mandatory create scene method
	void createScene(void)
	{
		mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
		mSceneMgr->setShadowTextureSize(512);
		mSceneMgr->setShadowColour(ColourValue(0.6, 0.6, 0.6));

		// Setup animation default
		Animation::setDefaultInterpolationMode(Animation::IM_LINEAR);
		Animation::setDefaultRotationInterpolationMode(Animation::RIM_LINEAR);

		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

		// The character will go through its motion graph
		// We want to copy the initial keyframes of all bones, but alter the Spineroot
		// to give it an offset of where the animation ends
		//	SkeletonPtr skel = SkeletonManager::getSingleton().load(modelname+".skeleton",
		//		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);



		ent = mSceneMgr->createEntity(modelname, modelname+".mesh");
		// Add entity to the scene node
		mSceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mSceneNode->attachObject(ent);
		mSceneNode->translate(0,12,0);

		ent->ExecuteMotionGraph("mg");

		//	mAnimState = ent->getAnimationState(animationName);
		//	mAnimState->setEnabled(true);
		//	mAnimState->setLoop(true); // manual loop since translation involved




		// Give it a little ambience with lights
		Light* l;
		l = mSceneMgr->createLight("BlueLight");
		l->setType(Light::LT_SPOTLIGHT);
		l->setPosition(-200,150,-100);
		Vector3 dir(-l->getPosition());
		dir.normalise();
		l->setDirection(dir);
		l->setDiffuseColour(0.5, 0.5, 1.0);

		l = mSceneMgr->createLight("GreenLight");
		l->setType(Light::LT_SPOTLIGHT);
		l->setPosition(0,150,-100);
		dir = -l->getPosition();
		dir.normalise();
		l->setDirection(dir);
		l->setDiffuseColour(0.5, 1.0, 0.5);

		// Position the camera
		mCamera->setPosition(100,20,0);
		mCamera->setNearClipDistance(.01);
		mCamera->lookAt(0,10,0);

		// Report whether hardware skinning is enabled or not
		/*Technique* t = ent->getSubEntity(0)->getMaterial()->getBestTechnique();
		Pass* p = t->getPass(0);
		if (p->hasVertexProgram() && p->getVertexProgram()->isSkeletalAnimationIncluded())
		mDebugText = "Hardware skinning is enabled";
		else
		mDebugText = "Software skinning is enabled";*/

		Plane plane;
		plane.normal = Vector3::UNIT_Y;
		plane.d = 100;
		MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
			1500,1500,20,20,true,1,60,60,Vector3::UNIT_Z);
		Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
		pPlaneEnt->setMaterialName("Examples/Rockwall");
		pPlaneEnt->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,99,0))->attachObject(pPlaneEnt);
	}

	// Create new frame listener
	void createFrameListener(void)
	{
		mFrameListener= new CharacterAnimationFrameListener(mWindow, mCamera, mDebugText);
		mRoot->addFrameListener(mFrameListener);
	}
};
