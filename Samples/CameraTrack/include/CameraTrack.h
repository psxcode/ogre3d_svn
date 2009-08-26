#ifndef __CameraTrack_H__
#define __CameraTrack_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class CameraTrackSample : public SdkSample
{
public:

	CameraTrackSample()
	{
		mInfo["Title"] = "Camera Tracking";
		mInfo["Description"] = "An example of using AnimationTracks to make a node smoothly follow "
			"a predefined path with spline interpolation. Also uses the auto-tracking feature of the camera.";
		mInfo["Thumbnail"] = "thumb_camtrack.png";
		mInfo["Category"] = "Animation";
	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        mAnimState->addTime(evt.timeSinceLastFrame);   // increment animation time
		return SdkSample::frameRenderingQueued(evt);
    }

protected:

	void setupScene()
	{
		// setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
        mSceneMgr->createLight()->setPosition(20, 80, 50);
        
		mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);  // set a nice cloudy skydome

		// create a floor mesh resource
		MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Plane(Vector3::UNIT_Y, -30), 1000, 1000, 10, 10, true, 1, 8, 8, Vector3::UNIT_Z);

		// create a floor entity, give it a material, and place it at the origin
        Entity* floor = mSceneMgr->createEntity("Floor", "floor");
        floor->setMaterialName("Examples/BumpyMetal");
        mSceneMgr->getRootSceneNode()->attachObject(floor);

		// create an ogre head entity and attach it to a node
		Entity* head = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		headNode->attachObject(head);

		mCameraMan->setStyle(CS_MANUAL);   // we will be controlling the camera ourselves, so disable the camera man
        mCamera->setAutoTracking(true, headNode);   // make the camera face the head

        // create a camera node and attach camera to it
        SceneNode* camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        camNode->attachObject(mCamera);

		// set up a 10 second animation for our camera, using spline interpolation for nice curves
        Animation* anim = mSceneMgr->createAnimation("CameraTrack", 10);
        anim->setInterpolationMode(Animation::IM_SPLINE);

		// create a track to animate the camera's node
        NodeAnimationTrack* track = anim->createNodeTrack(0, camNode);

        // create keyframes for our track
        track->createNodeKeyFrame(0)->setTranslate(Vector3(200, 100, 0));
        track->createNodeKeyFrame(2.5)->setTranslate(Vector3(0, 50, 100));
        track->createNodeKeyFrame(5)->setTranslate(Vector3(-500, 200, 0));
        track->createNodeKeyFrame(7.5)->setTranslate(Vector3(0, 300, -300));
        track->createNodeKeyFrame(10)->setTranslate(Vector3(200, 100, 0));

        // create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("CameraTrack");
        mAnimState->setEnabled(true);
	}

	void cleanupScene()
	{
		MeshManager::getSingleton().remove("floor");
	}

	AnimationState* mAnimState;
};

#endif
