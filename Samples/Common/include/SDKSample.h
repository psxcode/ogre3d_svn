#ifndef __SDKSample_H__
#define __SDKSample_H__

#include "Sample.h"

namespace OgreBites
{
	/*=============================================================================
	| Base class for all SDK samples. Includes a default player camera and GUI.
	=============================================================================*/
	class SDKSample : public Sample
    {
    public:

		/*-----------------------------------------------------------------------------
		| Saves off camera state.
		-----------------------------------------------------------------------------*/
		virtual void saveState(Ogre::NameValuePairList& state) {}

		/*-----------------------------------------------------------------------------
		| Restores camera state.
		-----------------------------------------------------------------------------*/
		virtual void restoreState(const Ogre::NameValuePairList state) {}

		// callback interface copied from various listeners to be used by SampleContext
		virtual bool frameStarted(const Ogre::FrameEvent& evt) { return true; }
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) { return true; }
		virtual bool frameEnded(const Ogre::FrameEvent& evt) { return true; }
		virtual void windowMoved(Ogre::RenderWindow* rw) {}
		virtual void windowResized(Ogre::RenderWindow* rw) {}
		virtual bool windowClosing(Ogre::RenderWindow* rw) { return true; }
		virtual void windowClosed(Ogre::RenderWindow* rw) {}
		virtual void windowFocusChange(Ogre::RenderWindow* rw) {}
		virtual bool keyPressed(const OIS::KeyEvent& evt) { return true; }
		virtual bool keyReleased(const OIS::KeyEvent& evt) { return true; }
		virtual bool mouseMoved(const OIS::MouseEvent& evt) { return true; }
		virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id) { return true; }
		virtual bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id) { return true; }

    protected:

		/*-----------------------------------------------------------------------------
		| Sets up a default viewport layout and camera.
		-----------------------------------------------------------------------------*/
		virtual void setupView()
		{
			mMainCam = mSceneMgr->createCamera("Main Camera");
			Ogre::Viewport* vp = mWindow->addViewport(mMainCam);
			mMainCam->setAspectRatio((Ogre::Real)vp->getActualWidth() / (Ogre::Real)vp->getActualHeight());
			mMainCam->setAutoAspectRatio(true);
			mMainCam->setNearClipDistance(5);
			mMainCam->setPosition(0, 0, 250);
		}

		Ogre::Camera* mMainCam;     // main camera for this sample
    };
}

#endif
