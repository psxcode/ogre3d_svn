#ifndef __Transparency_H__
#define __Transparency_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

/* NOTE: This sample simply displays an object with a transparent material. The really relevant stuff
is all in the material script itself. You won't find anything even vaguely related to transparency in
this source code. Check out the Examples/WaterStream material in Examples.material. */

class TransparencySample : public SdkSample
{
public:

	TransparencySample()
	{
		mInfo["Title"] = "Transparency";
		mInfo["Description"] = "Demonstrates the use of transparent materials (or scene blending).";
		mInfo["Thumbnail"] = "thumb_trans.png";
		mInfo["Category"] = "Materials";
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		mTheta += evt.timeSinceLastFrame;

		// this is the equation for a PQ torus knot
		Ogre::Real r = 28 * (2 + Math::Sin(mTheta * 3 / 2 + 0.2));
		Ogre::Real x = r * Math::Cos(mTheta);
		Ogre::Real y = r * Math::Sin(mTheta);
		Ogre::Real z = 60 * Math::Cos(mTheta * 3 / 2 + 0.2);

		mLastFishPos = mFishNode->getPosition();   // save fishy's last position
		mFishNode->setPosition(x, y, z);           // set fishy's new position

		// set fishy's direction based on the change in position
		mFishNode->setDirection(mFishNode->getPosition() - mLastFishPos, Node::TS_PARENT, Vector3::NEGATIVE_UNIT_X);

		mFishSwim->addTime(evt.timeSinceLastFrame * 5);   // update fishy's swimming animation

		return SdkSample::frameRenderingQueued(evt);   // don't forget the parent class updates!
	}

protected:

	void setupScene()
	{     
		mSceneMgr->setSkyBox(true, "Examples/TrippySkyBox");

		mCamera->setPosition(0, 0, 300);   // set camera's starting position

        mSceneMgr->createLight()->setPosition(20, 80, 50);   // add basic point light

		// create a torus knot model, give it the translucent texture, and attach it to the origin
		Entity* ent = mSceneMgr->createEntity("Knot", "knot.mesh");
        ent->setMaterialName("Examples/WaterStream");
		mSceneMgr->getRootSceneNode()->attachObject(ent);

		// create a fishy and enable its swimming animation
		ent = mSceneMgr->createEntity("Fish", "fish.mesh");
		mFishSwim = ent->getAnimationState("swim");
		mFishSwim->setEnabled(true);
		
		// create a scene node, attach fishy to it, and scale it by a factor of 2
		mFishNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mFishNode->attachObject(ent);
		mFishNode->setScale(2, 2, 2);

		mTheta = 0;
	}

	SceneNode* mFishNode;
	AnimationState* mFishSwim;
	Real mTheta;
	Vector3 mLastFishPos;
};

#endif
