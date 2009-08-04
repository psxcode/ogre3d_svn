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
			mInfo["Help"] = "";

			mTrayMgr = 0;
			mCameraMan = 0;
		}

		virtual ~SdkSample() {}

		/*-----------------------------------------------------------------------------
		| Manually update the cursor position after being unpaused.
		-----------------------------------------------------------------------------*/
		virtual void unpaused()
		{
			mTrayMgr->refreshCursor();
		}

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

			if (!mTrayMgr->isDialogVisible()) mCameraMan->frameRenderingQueued(evt);  // don't move camera if dialog is up

			return true;
		}

		virtual void okDialogClosed(const Ogre::DisplayString& message)
		{
			if (!mCursorWasVisible) mTrayMgr->hideCursor();  // re-hide the cursor when dialog is closed
		}

		virtual bool keyPressed(const OIS::KeyEvent& evt)
		{
			if (evt.key == OIS::KC_H && mInfo["Help"] != "")   // toggle visibility of help dialog
			{
				if (mTrayMgr->isDialogVisible()) mTrayMgr->closeDialog();
				else
				{
					mCursorWasVisible = mTrayMgr->isCursorVisible();
					mTrayMgr->showCursor();
					mTrayMgr->showOkDialog("Help", mInfo["Help"]);
				}
			}

			if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

			if (evt.key == OIS::KC_F)   // toggle visibility of advanced stats
			{
				mTrayMgr->toggleAdvancedStats();
			}

			mCameraMan->injectKeyDown(evt);

			return true;
		}

		virtual bool keyReleased(const OIS::KeyEvent& evt)
		{
			mCameraMan->injectKeyUp(evt);

			return true;
		}

		/* IMPORTANT: When overriding these following handlers, remember to allow the tray manager
		to filter out any interface-related mouse events before processing them in your scene.
		If the tray manager handler returns true, the event was meant for the trays, not you. */

		virtual bool mouseMoved(const OIS::MouseEvent& evt)
		{
			if (mTrayMgr->injectMouseMove(evt)) return true;

			mCameraMan->injectMouseMove(evt);

			return true;
		}

		virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (mTrayMgr->injectMouseDown(evt, id)) return true;

			mCameraMan->injectMouseDown(evt, id);

			return true;
		}

		virtual bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (mTrayMgr->injectMouseUp(evt, id)) return true;

			mCameraMan->injectMouseUp(evt, id);

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

	private:

		bool mCursorWasVisible;      // was cursor visible before dialog appeared (for private use)
    };
}

#endif
