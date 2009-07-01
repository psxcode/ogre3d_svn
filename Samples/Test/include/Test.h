#ifndef __Test_H__
#define __Test_H__

#include "SDKSample.h"

using namespace Ogre;
using namespace OIS;
using namespace OgreBites;

#define CUSTOM_SHININESS 1
#define CUSTOM_DIFFUSE 2
#define CUSTOM_SPECULAR 3

class Test1 : public SDKSample
{
public:

	Test1()
	{
		mInfo["Title"] = "Test Sample 1";
		mHeadSpeed = 0;
	}

	String getRequiredRenderSystem()
	{
		return "PENIS BOT";
	}

protected:

	void setupScene()
	{
		mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0.3, 0.4, 0.3));
        mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox");

		Light* lt = mSceneMgr->createLight("Simple Light");
		lt->setPosition(20, 40, 50);

		Entity* ent = mSceneMgr->createEntity("Ogre Head", "ogrehead.mesh");
		
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
		mHeadNode->yaw(Radian(evt.timeSinceLastFrame) * mHeadSpeed);
		return true;
	}

	bool keyPressed(const KeyEvent& evt)
	{
		if (evt.key == KC_SPACE) mHeadSpeed = 1;
		else if (evt.key == KC_ESCAPE) mDone = true;
		return true;
	}

	bool keyReleased(const KeyEvent& evt)
	{
		if (evt.key == KC_SPACE) mHeadSpeed = 0;
		return true;
	}

	SceneNode* mHeadNode;
	Real mHeadSpeed;
};

class Test2 : public SDKSample
{
public:

	Test2()
	{
		mInfo["Title"] = "Test Sample 2";
	}

protected:

	virtual void setupScene()
	{
		ParticleSystem* fw = mSceneMgr->createParticleSystem("Fireworks", "Examples/Fireworks");
		mSceneMgr->getRootSceneNode()->attachObject(fw);
	}

	bool keyPressed(const KeyEvent& evt)
	{
		if (evt.key == KC_ESCAPE) mDone = true;
		return true;
	}
};

#endif
