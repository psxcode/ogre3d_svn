#ifndef __BSP_H__
#define __BSP_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class BspSample : public SdkSample
{
public:

	BspSample()
	{
		mInfo["Title"] = "BSP";
		mInfo["Description"] = "A demo of the indoor, or BSP (Binary Space Partition) scene manager. "
			"Also demonstrates how to load BSP maps from Quake 3.";
		mInfo["Thumbnail"] = "thumb_bsp.png";
		mInfo["Category"] = "Unsorted";
	}

	StringVector getRequiredPlugins()
	{
		StringVector names;
		names.push_back("BSP Scene Manager");
		return names;
	}

protected:

	void locateResources()
	{
		// load the Quake archive location and map name from a config file
		ConfigFile cf;
		cf.load("quakemap.cfg");
		mArchive = cf.getSetting("Archive");
		mMap = cf.getSetting("Map");

		// add the Quake archive to the world resource group
		ResourceGroupManager::getSingleton().addResourceLocation(mArchive, "Zip",
			ResourceGroupManager::getSingleton().getWorldResourceGroupName(), true);
	}

	void createSceneManager()
	{
		mSceneMgr = mRoot->createSceneManager("BspSceneManager");   // the BSP scene manager is required for this sample
	}

	void loadResources()
	{
		ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();
/*
		mLoadingBar.start(mWindow, 1, 1, 0.75);

		// Turn off rendering of everything except overlays
		mSceneMgr->clearSpecialCaseRenderQueues();
		mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
		mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);
*/
		// associate the world geometry with the world resource group, and then load the group
		rgm.linkWorldGeometryToResourceGroup(rgm.getWorldResourceGroupName(), mMap, mSceneMgr);
		rgm.initialiseResourceGroup(rgm.getWorldResourceGroupName());
		rgm.loadResourceGroup(rgm.getWorldResourceGroupName(), false);
/*
		// Back to full rendering
		mSceneMgr->clearSpecialCaseRenderQueues();
		mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);

		mLoadingBar.finish();*/
	}

	void unloadResources()
	{
		// unload the map so we don't interfere with subsequent samples
		ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();
		rgm.unloadResourceGroup(rgm.getWorldResourceGroupName());
		rgm.removeResourceLocation(mArchive, ResourceGroupManager::getSingleton().getWorldResourceGroupName());
	}

	void setupView()
	{
		SdkSample::setupView();

		// modify camera for close work
		mCamera->setNearClipDistance(4);
		mCamera->setFarClipDistance(4000);

		// set a random player starting point
		ViewPoint vp = mSceneMgr->getSuggestedViewpoint(true);

		// Quake uses the Z axis as the up axis, so make necessary adjustments
		mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);
		mCamera->pitch(Degree(90));

		mCamera->setPosition(vp.position);
		mCamera->rotate(vp.orientation);

		mCameraMan->setTopSpeed(350);   // make the camera move a bit faster
	}

	String mArchive;
	String mMap;
};

#endif
