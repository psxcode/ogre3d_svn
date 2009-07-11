#ifndef __SdkSample_H__
#define __SdkSample_H__

#include "Sample.h"
#include "SdkTrays.h"
#include "SdkCameraMan.h"

namespace OgreBites
{
	/*=============================================================================
	| Base SDK sample class. Includes default player camera and SDK trays.
	=============================================================================*/
	class SdkSample : public Sample, public SdkTrayListener
    {
    public:

		virtual void paused()
		{
			if (mTrayMgr->isCursorVisible())  // hide the cursor on pause if necessary
			{
				mCursorHiddenByPause = true;
				mTrayMgr->hideCursor();
			}
			else mCursorHiddenByPause = false;
		}

		virtual void unpaused()
		{
			// unhide the cursor after unpausing if necessary
			if (mCursorHiddenByPause) mTrayMgr->showCursor();
		}

		virtual void saveState(Ogre::NameValuePairList& state) {}

		virtual void restoreState(const Ogre::NameValuePairList state) {}

		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
		{
			//mCameraMan->frameRenderingQueued(evt);
			return true;
		}

		virtual void windowMoved(Ogre::RenderWindow* rw) {}

		virtual void windowResized(Ogre::RenderWindow* rw) {}

		virtual bool windowClosing(Ogre::RenderWindow* rw) { return true; }

		virtual void windowClosed(Ogre::RenderWindow* rw) {}

		virtual void windowFocusChange(Ogre::RenderWindow* rw) {}

		virtual bool keyPressed(const OIS::KeyEvent& evt)
		{
			if (evt.key == OIS::KC_TAB)    // toggle tray interface
			{
				if (mTrayMgr->isCursorVisible()) mTrayMgr->hideCursor();
				else mTrayMgr->showCursor();
			}
			return true;
		}

		virtual bool keyReleased(const OIS::KeyEvent& evt) { return true; }

		/* IMPORTANT: When overriding these following handlers, remember to allow the tray manager
		to filter out any interface-related mouse events before processing them in your scene.
		You can do this by calling these parent versions or calling the tray manager's handler
		yourself at the beginning of your handler. */

		virtual bool mouseMoved(const OIS::MouseEvent& evt)
		{
			return mTrayMgr->mouseMoved(evt);
		}

		virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			return mTrayMgr->mousePressed(evt, id);
		}

		virtual bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			return mTrayMgr->mouseReleased(evt, id);
		}

    protected:

		virtual void _setup(Ogre::RenderWindow* window, OIS::Keyboard* keyboard, OIS::Mouse* mouse)
		{
			mTrayMgr = new SdkTrayManager("SampleControls", mouse, this);
			Sample::_setup(window, keyboard, mouse);
		}

		virtual void _shutdown()
		{
			Sample::_shutdown();
			if (mTrayMgr) delete mTrayMgr;
		}

		virtual void setupView()
		{
			// setup default viewport layout and camera
			mCamera = mSceneMgr->createCamera("MainCamera");
			mViewport = mWindow->addViewport(mCamera);
			mCamera->setAspectRatio((Ogre::Real)mViewport->getActualWidth() / (Ogre::Real)mViewport->getActualHeight());
			mCamera->setAutoAspectRatio(true);
			mCamera->setPosition(0, 0, 200);
		}

		SdkTrayManager* mTrayMgr;    // tray interface manager
		Ogre::Viewport* mViewport;   // main viewport
		Ogre::Camera* mCamera;       // main camera
		SdkCameraMan* mCameraMan;    // basic camera controller

	private:

		bool mCursorHiddenByPause;    // for internal use only
    };
}

#endif
