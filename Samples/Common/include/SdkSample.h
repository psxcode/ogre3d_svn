#ifndef __SdkSample_H__
#define __SdkSample_H__

#include "Sample.h"
#include "SdkTrays.h"
#include "SdkCameraMan.h"

namespace OgreBites
{
	/*=============================================================================
	// Base SDK sample class. Includes default player camera and SDK trays.
	=============================================================================*/
	class SdkSample : public Sample, public SdkTrayListener
    {
    public:

		SdkSample()
		{
			// so we don't have to worry about checking if these keys exist later
			mInfo["Title"] = "Untitled";
			mInfo["Description"] = "";
			mInfo["Category"] = "Unsorted";
			mInfo["Thumbnail"] = "";

			mTrayMgr = 0;
			mCameraMan = 0;
		}

		virtual ~SdkSample() {}

		/*-----------------------------------------------------------------------------
		| Automatically saves position and orientation for free-look cameras.
		-----------------------------------------------------------------------------*/
		virtual void saveState(Ogre::NameValuePairList& state)
		{
			if (mCameraMan->getStyle() == CS_FREELOOK)
			{
				state["CameraPosition"] = Ogre::StringConverter::toString(mCameraMan->getCameraNode()->getPosition());
				state["CameraOrientation"] = Ogre::StringConverter::toString(mCameraMan->getCameraNode()->getOrientation());
			}
		}

		/*-----------------------------------------------------------------------------
		| Automatically restores position and orientation for free-look cameras.
		-----------------------------------------------------------------------------*/
		virtual void restoreState(Ogre::NameValuePairList& state)
		{
			if (state.find("CameraPosition") != state.end() && state.find("CameraOrientation") != state.end())
			{
				mCameraMan->setStyle(CS_FREELOOK);
				mCameraMan->getCameraNode()->setPosition(Ogre::StringConverter::parseVector3(state["CameraPosition"]));
				mCameraMan->getCameraNode()->setOrientation(Ogre::StringConverter::parseQuaternion(state["CameraOrientation"]));
			}
		}

		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
		{
			mTrayMgr->frameRenderingQueued(evt);
			mCameraMan->frameRenderingQueued(evt);
			return true;
		}

		virtual bool keyPressed(const OIS::KeyEvent& evt)
		{
			if (evt.key == OIS::KC_F) mTrayMgr->toggleAdvancedStats();   // toggle visibility of advanced stats

			mCameraMan->keyPressed(evt);

			return true;
		}

		virtual bool keyReleased(const OIS::KeyEvent& evt)
		{
			mCameraMan->keyReleased(evt);
			return true;
		}

		/* IMPORTANT: When overriding these following handlers, remember to allow the tray manager
		to filter out any interface-related mouse events before processing them in your scene.
		If the tray manager handler returns false, the event was meant for the trays, not you. */

		virtual bool mouseMoved(const OIS::MouseEvent& evt)
		{
			if (!mTrayMgr->mouseMoved(evt)) return false;
			mCameraMan->mouseMoved(evt);
			return true;
		}

		virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (!mTrayMgr->mousePressed(evt, id)) return false;
			mCameraMan->mousePressed(evt, id);
			return true;
		}

		virtual bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (!mTrayMgr->mouseReleased(evt, id)) return false;
			mCameraMan->mouseReleased(evt, id);
			return true;
		}

		/*-----------------------------------------------------------------------------
		| Extendeded to setup a default tray interface and camera controller.
		-----------------------------------------------------------------------------*/
		virtual void _setup(Ogre::RenderWindow* window, OIS::Keyboard* keyboard, OIS::Mouse* mouse)
		{
			mWindow = window;
			mKeyboard = keyboard;
			mMouse = mouse;

			locateResources();
			loadResources();
			createSceneManager();
			setupView();

			// create a tray interface with stats panel and logo
			mTrayMgr = new SdkTrayManager("SampleControls", window, mouse, this);
			mTrayMgr->showStats(TL_BOTTOMLEFT);
			mTrayMgr->showLogo(TL_BOTTOMRIGHT);
			mTrayMgr->hideCursor();

			mCameraMan = new SdkCameraMan(mCamera);   // create a default camera

			setupScene();

			mDone = false;
		}

		virtual void _shutdown()
		{
			if (mTrayMgr) delete mTrayMgr;
			if (mCameraMan) delete mCameraMan;

			Sample::_shutdown();
		}

    protected:

		virtual void setupView()
		{
			// setup default viewport layout and camera
			mCamera = mSceneMgr->createCamera("MainCamera");
			mViewport = mWindow->addViewport(mCamera);
			mCamera->setAspectRatio((Ogre::Real)mViewport->getActualWidth() / (Ogre::Real)mViewport->getActualHeight());
			mCamera->setAutoAspectRatio(true);
			mCamera->setNearClipDistance(10);
			mCamera->setFarClipDistance(10000);
		}

		SdkTrayManager* mTrayMgr;    // tray interface manager
		Ogre::Viewport* mViewport;   // main viewport
		Ogre::Camera* mCamera;       // main camera
		SdkCameraMan* mCameraMan;    // basic camera controller
    };
}

#endif
