#ifndef __Test_H__
#define __Test_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

#define CUSTOM_SHININESS 1
#define CUSTOM_DIFFUSE 2
#define CUSTOM_SPECULAR 3

class Test1 : public SdkSample
{
public:

	Test1()
	{
		mInfo["Title"] = "Cel-shading";
		mInfo["Description"] = "A demo of cel-shaded graphics using vertex & fragment programs.";
	}

protected:

	void setupScene()
	{
        mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");

		Light* lt = mSceneMgr->createLight("BasicLight");
		lt->setPosition(20, 40, 50);

		Entity* ent = mSceneMgr->createEntity("OgreHead", "ogrehead.mesh");
		
		SubEntity* sub;

        sub = ent->getSubEntity(0);
        sub->setMaterialName("Examples/CelShading");
        sub->setCustomParameter(CUSTOM_SHININESS, Vector4(35.0f, 0.0f, 0.0f, 0.0f));
        sub->setCustomParameter(CUSTOM_DIFFUSE, Vector4(1.0f, 0.3f, 0.3f, 1.0f));
        sub->setCustomParameter(CUSTOM_SPECULAR, Vector4(1.0f, 0.6f, 0.6f, 1.0f));

        sub = ent->getSubEntity(1);
        sub->setMaterialName("Examples/CelShading");
        sub->setCustomParameter(CUSTOM_SHININESS, Vector4(10.0f, 0.0f, 0.0f, 0.0f));
        sub->setCustomParameter(CUSTOM_DIFFUSE, Vector4(0.0f, 0.5f, 0.0f, 1.0f));
        sub->setCustomParameter(CUSTOM_SPECULAR, Vector4(0.3f, 0.5f, 0.3f, 1.0f));

        sub = ent->getSubEntity(2);
        sub->setMaterialName("Examples/CelShading");
        sub->setCustomParameter(CUSTOM_SHININESS, Vector4(25.0f, 0.0f, 0.0f, 0.0f));
        sub->setCustomParameter(CUSTOM_DIFFUSE, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
        sub->setCustomParameter(CUSTOM_SPECULAR, Vector4(1.0f, 1.0f, 0.7f, 1.0f));

        sub = ent->getSubEntity(3);
        sub->setMaterialName("Examples/CelShading");
        sub->setCustomParameter(CUSTOM_SHININESS, Vector4(20.0f, 0.0f, 0.0f, 0.0f));
        sub->setCustomParameter(CUSTOM_DIFFUSE, Vector4(1.0f, 1.0f, 0.7f, 1.0f));
        sub->setCustomParameter(CUSTOM_SPECULAR, Vector4(1.0f, 1.0f, 1.0f, 1.0f));

		mHeadNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mHeadNode->attachObject(ent);       
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		mHeadNode->yaw(Radian(evt.timeSinceLastFrame));
		return SdkSample::frameRenderingQueued(evt);
	}

	SceneNode* mHeadNode;
};

class Test2 : public SdkSample
{
public:

	Test2()
	{
		mInfo["Title"] = "Fireworks";
		mInfo["Description"] = "A demo showing a basic fireworks effect created using a particle system.";
	}

protected:

	virtual void setupScene()
	{
		ParticleSystem* fw = mSceneMgr->createParticleSystem("Fireworks", "Examples/Fireworks");
		mSceneMgr->getRootSceneNode()->attachObject(fw);
	}

	virtual void postSceneSetup()
	{
		mTrayMgr->showLogo(TL_BOTTOMLEFT);
	}
};

class Test3 : public SdkSample
{
public:

	Test3()
	{
		std::ostringstream desc;
		desc << "An ogre (feminine: ogress) is a large, cruel and hideous "
			 << "humanoid monster, featured in mythology, folklore and fiction. Ogres are often "
			 << "depicted in fairy tales and folklore as feeding on human beings, and have appeared "
			 << "in many classic works of literature. In art, ogres are often depicted with a large head, "
			 << "abundant hair and beard, a voracious appetite, and a strong body. The term is often applied "
			 << "in a metaphorical sense to disgusting persons who exploit, brutalize or devour their victims."
			 << "\nBLAH:\tBLAH\nBLAH2:\tLULZHA";

		mInfo["Title"] = "Big Bad Ogre";
		mInfo["Description"] = desc.str();
	}

protected:

	virtual void setupScene()
	{
		mViewport->setBackgroundColour(ColourValue::White);
		mSceneMgr->setAmbientLight(ColourValue(0.1, 0.1, 0.1));

		mCamera->setPosition(0, 60, 190);
		mCamera->lookAt(0, 0, -150);

		Light* lt = mSceneMgr->createLight("BasicLight");
		lt->setPowerScale(10);
		lt->setPosition(400, 100, 400);

		Entity* ent = mSceneMgr->createEntity("Sinbad", "sinbad.mesh");
		ent->setMaterialName("Examples/Sinbad");

		sn = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		sn->setScale(20, 20, 20);
		sn->attachObject(ent);
	}

	virtual void postSceneSetup()
	{
		mTrayMgr->createButton(TL_BOTTOM, "Wire", "Toggle Wireframe");
	}

	void buttonHit(Button* b)
	{
		if (b->getName() == "Wire")
		{
			if (mCamera->getPolygonMode() == PM_WIREFRAME) mCamera->setPolygonMode(PM_SOLID);
			else mCamera->setPolygonMode(PM_WIREFRAME);
		}
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		sn->yaw(Radian(evt.timeSinceLastFrame / 2));
		return SdkSample::frameRenderingQueued(evt);
	}

	SceneNode* sn;
};

class Test4 : public SdkSample
{
public:

	Test4()
	{
		mInfo["Title"] = "Environment Mapping";
		mInfo["Description"] = "Shows OGRE's environment mapping feature as well as the blending modes available when using multiple texture layers.";
	}

protected:

	void setupScene()
	{
		mViewport->setBackgroundColour(ColourValue::White);

		Entity* ent = mSceneMgr->createEntity("OgreHead", "ogrehead.mesh");
		ent->setMaterialName("Examples/EnvMappedRustySteel");

		mHeadNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mHeadNode->attachObject(ent);       
	}

	virtual void postSceneSetup()
	{
		mTrayMgr->showStats(TL_TOPLEFT);
		mTrayMgr->showLogo(TL_TOPRIGHT);
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		mHeadNode->yaw(Radian(evt.timeSinceLastFrame));
		return SdkSample::frameRenderingQueued(evt);
	}

	SceneNode* mHeadNode;
};


















class Test5 : public SdkSample
{
public:

	Test5()
	{
		mInfo["Title"] = "Trays";
		mInfo["Description"] = "More trays!";
	}

protected:

	void postSceneSetup()
	{
		mTrayMgr->createCheckBox(TL_CENTER, "1", "Show Logo", true);
	}

	void boxChecked(CheckBox* cb)
	{
		mTrayMgr->showLogo(TL_BOTTOMRIGHT);
	}

	void boxUnchecked(CheckBox* cb)
	{
		mTrayMgr->hideLogo();
	}
};




#endif
