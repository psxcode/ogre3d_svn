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
Filename:    SkeletalAnimation.h
Description: Specialisation of OGRE's framework application to show the
             skeletal animation feature, including spline animation.
-----------------------------------------------------------------------------
*/


#include "ExampleApplication.h"
#include "OgreMotionGraph.h"

#define NUM_JAIQUAS 1
AnimationState* mAnimState[NUM_JAIQUAS];
Real mAnimationSpeed[NUM_JAIQUAS];
Vector3 mSneakStartOffset;
Vector3 mSneakEndOffset;

Quaternion mOrientations[NUM_JAIQUAS];
Vector3 mBasePositions[NUM_JAIQUAS];
SceneNode* mSceneNode[NUM_JAIQUAS];
Degree mAnimationRotation(-60);
Real mAnimChop = 2.26662f;
Real mAnimChopBlend = 0.3f;
Entity *ent;

const std::string animationName = "rush";
const std::string modelname = "character";

// Event handler to animate
class SkeletalAnimationFrameListener : public ExampleFrameListener
{
protected:
	//If interactive control is active, character's x-z plane moving direction is repesented by this variable
	MotionGraph::LocomotionDirection mLocomotionDirection;
	//When a character is locomoting, its speed is set by interactive input, and kept by this variable
	MotionGraph::LocomotionSpeed mLocomotionSpeed;
public:
	SkeletalAnimationFrameListener(RenderWindow* win, Camera* cam, const std::string &debugText)
		: ExampleFrameListener(win, cam),mLocomotionSpeed(MotionGraph::LOCOSPEED_IDLE),
		mLocomotionDirection(MotionGraph::LOCODIRECTION_CENTER)
    {
		mDebugText = debugText;
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
	if( ExampleFrameListener::frameRenderingQueued(evt) == false )
		return false;

   //     for (int i = 0; i < NUM_JAIQUAS; ++i)
   //     {
			//Real inc;
			//if ( IsAnimated() )
			//inc = evt.timeSinceLastFrame * mAnimationSpeed[i]; 
			//else
			//	inc = 0;
			//if ((mAnimState[i]->getTimePosition() + inc) >= mAnimChop)
			//{
			//	// Loop
			//	// Need to reposition the scene node origin since animation includes translation
			//	// Calculate as an offset to the end position, rotated by the
			//	// amount the animation turns the character
			//	Quaternion rot(mAnimationRotation, Vector3::UNIT_Y);
			//	Vector3 startoffset = mSceneNode[i]->getOrientation() * -mSneakStartOffset;
			//	Vector3 endoffset = mSneakEndOffset;
			//	Vector3 offset = rot * startoffset;
			//	Vector3 currEnd = mSceneNode[i]->getOrientation() * endoffset + mSceneNode[i]->getPosition();
			//	mSceneNode[i]->setPosition(currEnd + offset);
			//	mSceneNode[i]->rotate(rot);

			//	//mAnimState[i]->setTimePosition((mAnimState[i]->getTimePosition() + inc) - mAnimChop);
			//}
			//else
			//{
			//	//mAnimState[i]->addTime(inc);
			//}
   //     }

	MotionGraph::InteractiveControlInfo ControlInfo;
	ControlInfo.direct = mLocomotionDirection;
	ControlInfo.speed = mLocomotionSpeed;
	if ( IsAnimated() )
		ent->AdvanceMotionGraphTime(evt.timeSinceLastFrame,ControlInfo);
	else
		ent->AdvanceMotionGraphTime(0.,ControlInfo);
        return true;
    }

	bool processUnbufferedKeyInput(const FrameEvent& evt)
	{


		using namespace OIS;
	
		if ( mKeyboard->isKeyDown(KC_T) && mTimeUntilNextToggle <= 0)
		{
			//relocate character to initializing position
			//Vector3 currentPosition = mSceneNode[0]->getPosition();
			Vector3 currentPosition = ent->getSkeleton()->getRootBone()->getPosition();
//ent->getSkeleton()->getRootBone()->setPosition()
			mSceneNode[0]->setPosition(-currentPosition.x,currentPosition.y,-currentPosition.z);
		}

		return ExampleFrameListener::processUnbufferedKeyInput(evt);

	}
};



class SkeletalApplication : public ExampleApplication
{
public:
    SkeletalApplication() {}

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

		// The jaiqua sneak animation doesn't loop properly, so lets hack it so it does
		// We want to copy the initial keyframes of all bones, but alter the Spineroot
		// to give it an offset of where the animation ends
		SkeletonPtr skel = SkeletonManager::getSingleton().load(modelname+".skeleton", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		//Animation* anim = skel->getAnimation(animationName);
		//Animation::NodeTrackIterator trackIter = anim->getNodeTrackIterator();
		//while (trackIter.hasMoreElements())
		//{
		//	NodeAnimationTrack* track = trackIter.getNext();

		//	TransformKeyFrame oldKf(0, 0);
		//	track->getInterpolatedKeyFrame(mAnimChop, &oldKf);

		//	// Drop all keyframes after the chop
		//	while (track->getKeyFrame(track->getNumKeyFrames()-1)->getTime() >= mAnimChop - mAnimChopBlend)
		//		track->removeKeyFrame(track->getNumKeyFrames()-1);

		//	TransformKeyFrame* newKf = track->createNodeKeyFrame(mAnimChop);
		//	TransformKeyFrame* startKf = track->getNodeKeyFrame(0);

		//	Bone* bone = skel->getBone(track->getHandle());
		//	if (bone->getName() == "hip")//root bone name is "Spineroot" before
		//	{
		//		mSneakStartOffset = startKf->getTranslate() + bone->getInitialPosition();
		//		mSneakEndOffset = oldKf.getTranslate() + bone->getInitialPosition();
		//		mSneakStartOffset.y = mSneakEndOffset.y;
		//		// Adjust spine root relative to new location
		//		newKf->setRotation(oldKf.getRotation());
		//		newKf->setTranslate(oldKf.getTranslate());
		//		newKf->setScale(oldKf.getScale());


		//	}
		//	else
		//	{
		//		newKf->setRotation(startKf->getRotation());
		//		newKf->setTranslate(startKf->getTranslate());
		//		newKf->setScale(startKf->getScale());
		//	}
		//}




        
		Real rotInc = Math::TWO_PI / (float)NUM_JAIQUAS;
		Real rot = 0.0f;
        for (int i = 0; i < NUM_JAIQUAS; ++i)
        {
			Quaternion q;
			q.FromAngleAxis(Radian(rot), Vector3::UNIT_Y);

			mOrientations[i] = q;
			mBasePositions[i] = q * Vector3(0,0,-20);

            ent = mSceneMgr->createEntity("man" + StringConverter::toString(i), modelname+".mesh");
            // Add entity to the scene node
			mSceneNode[i] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			mSceneNode[i]->attachObject(ent);
			mSceneNode[i]->rotate(q);
			mSceneNode[i]->translate(mBasePositions[i]);
			mSceneNode[i]->translate(0,12,0);
			
           // mAnimState[i] = ent->getAnimationState(animationName);
          //  mAnimState[i]->setEnabled(true);
			//mAnimState[i]->setLoop(false); // manual loop since translation involved
            mAnimationSpeed[i] = Math::RangeRandom(0.5, 1.5);

			rot += rotInc;
        }
		

		ent->ExecuteMotionGraph("mg");




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
        mCamera->lookAt(0,10,0);

        // Report whether hardware skinning is enabled or not
        Technique* t = ent->getSubEntity(0)->getMaterial()->getBestTechnique();
        Pass* p = t->getPass(0);
        if (p->hasVertexProgram() && p->getVertexProgram()->isSkeletalAnimationIncluded())
            mDebugText = "Hardware skinning is enabled";
        else
            mDebugText = "Software skinning is enabled";

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
        mFrameListener= new SkeletalAnimationFrameListener(mWindow, mCamera, mDebugText);
        mRoot->addFrameListener(mFrameListener);
    }


};

