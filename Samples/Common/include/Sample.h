#ifndef __Sample_H__
#define __Sample_H__

#include "Ogre.h"

#define OIS_DYNAMIC_LIB
#include "OIS/OIS.h"

namespace OgreBites
{
	/*=============================================================================
	| Base class responsible for everything specific to one sample.
	| Designed to be subclassed for each sample.
	=============================================================================*/
	class Sample
    {
    public:

		/*=============================================================================
		| Utility comparison structure for sorting samples.
		=============================================================================*/
		struct Comparer
		{
			bool operator() (Sample* a, Sample* b)
			{
				Ogre::NameValuePairList::iterator aTitle = a->getInfo().find("Title");
				Ogre::NameValuePairList::iterator bTitle = b->getInfo().find("Title");
				
				if (aTitle != a->getInfo().end() && bTitle != b->getInfo().end()) return aTitle->second.compare(bTitle->second) < 0;
				else return false;
			}
		};

		Sample()
        {
			mWindow = 0;
			mSceneMgr = 0;
			mDone = true;       // sample has not started
        }

		/*-----------------------------------------------------------------------------
		| Retrieves custom sample info.
		-----------------------------------------------------------------------------*/
		Ogre::NameValuePairList& getInfo()
		{
			return mInfo;
		}

		/*-----------------------------------------------------------------------------
		| Tests to see if target machine meets any special requirements of
		| this sample. Signal a failure by throwing an exception.
		-----------------------------------------------------------------------------*/
		virtual void testCapabilities(const Ogre::RenderSystemCapabilities* caps) {}

		/*-----------------------------------------------------------------------------
		| If this sample requires a specific render system to run, this method
		| will be used to return its name.
		-----------------------------------------------------------------------------*/
		virtual Ogre::String getRequiredRenderSystem()
		{
			return "";
		}

		/*-----------------------------------------------------------------------------
		| If this sample requires specific plugins to run, this method will be
		| used to return their names.
		-----------------------------------------------------------------------------*/
		virtual Ogre::StringVector getRequiredPlugins()
		{
			return Ogre::StringVector();
		}

		Ogre::SceneManager* getSceneManager()
		{
			return mSceneMgr;
		}

		bool isDone()
		{
			return mDone;
		}

		/*-----------------------------------------------------------------------------
		| Sets up a sample. Used by the SampleContext class. Do not call directly.
		-----------------------------------------------------------------------------*/
		virtual void _setup(Ogre::RenderWindow* window, OIS::Keyboard* keyboard, OIS::Mouse* mouse)
		{
			mWindow = window;
			mKeyboard = keyboard;
			mMouse = mouse;

			locateResources();
			loadResources();
			preSceneSetup();
			createSceneManager();
			setupView();
			setupScene();
			postSceneSetup();

			mDone = false;  // sample now started
		}

		/*-----------------------------------------------------------------------------
		| Resets a sample's state (not resources or listeners, etc.). I only reset
		| the scene graph here, but you can override to reset any custom variables.
		-----------------------------------------------------------------------------*/
		virtual void reset()
		{
			mSceneMgr->clearScene();
			setupScene();
		}

		/*-----------------------------------------------------------------------------
		| Shuts down a sample. Used by the SampleContext class. Do not call directly.
		-----------------------------------------------------------------------------*/
		virtual void _shutdown()
		{
			Ogre::Root::getSingleton().destroySceneManager(mSceneMgr);
			unloadResources();
			finalCleanup();

			mDone = true;          // sample now ended
		}

		/*-----------------------------------------------------------------------------
		| Saves the sample state to a string map.
		| Optional. Used by SampleContext::reset.
		-----------------------------------------------------------------------------*/
		virtual void saveState(Ogre::NameValuePairList& state) {}

		/*-----------------------------------------------------------------------------
		| Restores the sample state from a string map.
		| Optional. Used by SampleContext::reset.
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
		| Finds sample-specific resources. No such effort is made for most samples,
		| but this is useful for special samples with large, exclusive resources.
		-----------------------------------------------------------------------------*/
		virtual void locateResources() {}

		/*-----------------------------------------------------------------------------
		| Loads sample-specific resources. No such effort is made for most samples,
		| but this is useful for special samples with large, exclusive resources.
		-----------------------------------------------------------------------------*/
		virtual void loadResources() {}

		/*-----------------------------------------------------------------------------
		| Handles any setup that must happen before setup of scene. Optional.
		-----------------------------------------------------------------------------*/
		virtual void preSceneSetup() {}

		/*-----------------------------------------------------------------------------
		| Creates a scene manager for the sample. A generic one is the default,
		| but many samples require a special kind of scene manager.
		-----------------------------------------------------------------------------*/
		virtual void createSceneManager()
		{
			mSceneMgr = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC);
		}

		/*-----------------------------------------------------------------------------
		| Sets up viewport layout and camera.
		-----------------------------------------------------------------------------*/
		virtual void setupView() {}

		/*-----------------------------------------------------------------------------
		| Sets up the scene.
		-----------------------------------------------------------------------------*/
		virtual void setupScene() {}

		/*-----------------------------------------------------------------------------
		| Handles any setup that must happen after setup of scene. Optional.
		-----------------------------------------------------------------------------*/
		virtual void postSceneSetup() {}

		/*-----------------------------------------------------------------------------
		| Unloads sample-specific resources. My method here is simple and good
		| enough for most small samples, but your needs may vary.
		-----------------------------------------------------------------------------*/
		virtual void unloadResources()
		{
			Ogre::ResourceGroupManager::ResourceManagerIterator& resMgrs =
			Ogre::ResourceGroupManager::getSingleton().getResourceManagerIterator();

			while (resMgrs.hasMoreElements())
			{
				resMgrs.getNext()->unloadUnreferencedResources();
			}
		}

		/*-----------------------------------------------------------------------------
		| Performs any other necessary cleanup (like reverting special settings).
		-----------------------------------------------------------------------------*/
		virtual void finalCleanup() {}

		Ogre::RenderWindow* mWindow;      // context render window
		OIS::Keyboard* mKeyboard;         // context keyboard device
		OIS::Mouse* mMouse;               // context mouse device

		Ogre::SceneManager* mSceneMgr;    // scene manager for this sample
		Ogre::NameValuePairList mInfo;    // custom sample info

		bool mDone;                       // flag to mark the end of the sample
    };

	typedef std::set<Sample*, Sample::Comparer> SampleSet;
}

#endif
