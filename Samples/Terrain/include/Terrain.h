#ifndef __Terrain_H__
#define __Terrain_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class TerrainSample : public SdkSample
{
public:

	TerrainSample()
	{
		mInfo["Title"] = "Terrain";
		mInfo["Description"] = "Demonstrates use of the terrain rendering plugin.";
		mInfo["Thumbnail"] = "thumb_terrain.png";
		mInfo["Category"] = "Unsorted";
	}

	StringVector getRequiredPlugins()
	{
		StringVector names;
		names.push_back("Octree & Terrain Scene Manager");
		return names;
	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {
		// update the scene query's ray using new camera position
        mGroundRay.setOrigin(mCamera->getPosition());
        mGroundRay.setDirection(Vector3::NEGATIVE_UNIT_Y);
        mGroundQuery->setRay(mGroundRay);

		// execute the query and fetch the results
        RaySceneQueryResult& result = mGroundQuery->execute();
        RaySceneQueryResult::iterator i = result.begin();

		// if we are above ground, then snap the camera to it at a fixed distance
        if (i != result.end() && i->worldFragment)
        {
            mCamera->setPosition(mCamera->getPosition().x,
				i->worldFragment->singleIntersection.y + 10,
				mCamera->getPosition().z);
        }

		return SdkSample::frameRenderingQueued(evt);  // don't forget the parent updates!
    }

protected:

    void createSceneManager()
    {
		// we're going to need a special terrain scene manager for this sample
        mSceneMgr = mRoot->createSceneManager("TerrainSceneManager");
    }

	/*-----------------------------------------------------------------------------
	| Extends setupView to change some initial camera settings for this sample.
	-----------------------------------------------------------------------------*/
	void setupView()
	{
		SdkSample::setupView();

        mCamera->setPosition(707, 2500, 528);
        mCamera->setOrientation(Quaternion(-0.3486, 0.0122, 0.9365, 0.0329));
        mCamera->setNearClipDistance(1);

		if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_INFINITE_FAR_PLANE))
        {
            mCamera->setFarClipDistance(0);   // enable infinite far clip distance if we can
        }
		else mCamera->setFarClipDistance(1000);
	}

	void setupContent()
	{
		mCameraMan->setTopSpeed(50);

		// create some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
		mSceneMgr->createLight()->setPosition(20, 80, 50);

        // IMPORTANT: Set fog before calling setWorldGeometry, because the vertex program picked will be different.
        ColourValue fadeColour(0.93, 0.86, 0.76);
        mSceneMgr->setFog(FOG_LINEAR, fadeColour, 0, 200, 1000);
        mViewport->setBackgroundColour(fadeColour);

        mSceneMgr->setWorldGeometry("terrain.cfg");  // load our terrain from a config file

		// create our scene query, which will be used to cast a ray to the ground from the camera
        mGroundQuery = mSceneMgr->createRayQuery(mGroundRay);
	}

	RaySceneQuery* mGroundQuery;
	Ray mGroundRay;
};

#endif
