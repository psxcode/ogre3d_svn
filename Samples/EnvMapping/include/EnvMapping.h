#ifndef __EnvMapping_H__
#define __EnvMapping_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

/* NOTE: This sample simply displays an object with an env-mapped material. The really relevant stuff
is all in the material script itself. You won't find anything even vaguely related to env-mapping in
this source code. Check out the Examples/EnvMappedRustySteel material in Examples.material. */

class EnvMappingSample : public SdkSample
{
public:

	EnvMappingSample()
	{
		mInfo["Title"] = "Environment Mapping";
		mInfo["Description"] = "Shows the environment mapping feature of materials.";
		mInfo["Thumbnail"] = "thumb_envmap.png";
		mInfo["Category"] = "Unsorted";
	}

protected:

	void setupContent()
	{     
		mViewport->setBackgroundColour(ColourValue::White);

		// setup some basic lighting for our scene
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
        mSceneMgr->createLight()->setPosition(20, 80, 50);

		// set our camera to orbit around the origin and show cursor
		mCameraMan->setStyle(CS_ORBIT);
		mTrayMgr->showCursor();

		// create our model, give it the environment mapped material, and place it at the origin
        Entity *ent = mSceneMgr->createEntity("Head", "ogrehead.mesh");
		ent->setMaterialName("Examples/EnvMappedRustySteel");
		mSceneMgr->getRootSceneNode()->attachObject(ent);
	}
};

#endif
